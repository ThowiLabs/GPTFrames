#!/usr/bin/env python3
"""GPTFrames MCP: stdio or minimal Streamable HTTP server, stdlib only."""
from __future__ import annotations

import argparse
import hmac
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

from media import download_commons_image, search_images

ROOT = Path(__file__).resolve().parent.parent
PROJECTS = Path(os.environ.get("GPTFRAMES_PROJECTS", ROOT / "projects")).resolve()
MODELS = Path(os.environ.get("GPTFRAMES_MODELS", ROOT / "models")).resolve()
ENGINE = Path(os.environ.get("GPTFRAMES_ENGINE", ROOT / "build" / "gptframes-linux" / "hyperframes-cpp-native")).resolve()
NEO_LITE = os.environ.get("GPTFRAMES_NEO_LITE", "neo-lite")
MAX_RENDER_SECONDS = 60

TOOLS = [
    {"name": "list_projects", "description": "List available GPTFrames projects.", "inputSchema": {"type": "object", "properties": {}, "additionalProperties": False}},
    {"name": "create_project", "description": "Create an editable GPTFrames JSON video project from the included showcase.", "inputSchema": {"type": "object", "properties": {"name": {"type": "string", "description": "Short project slug, e.g. promo_octubre"}}, "required": ["name"], "additionalProperties": False}},
    {"name": "validate_project", "description": "Validate a project's scenes, layers, paths and timing using the native C++ engine.", "inputSchema": {"type": "object", "properties": {"project": {"type": "string"}}, "required": ["project"], "additionalProperties": False}},
    {"name": "preview_frame", "description": "Render one PNG preview of a project at the requested second.", "inputSchema": {"type": "object", "properties": {"project": {"type": "string"}, "second": {"type": "number", "minimum": 0}}, "required": ["project", "second"], "additionalProperties": False}},
    {"name": "render_video", "description": "Render a project's MP4 with native H.264/AAC encoding.", "inputSchema": {"type": "object", "properties": {"project": {"type": "string"}, "seconds": {"type": "number", "minimum": 0.1, "maximum": 60}}, "required": ["project"], "additionalProperties": False}},
    {"name": "synthesize_voice", "description": "Synthesize Spanish or other single-speaker audio using installed Neo Lite and a configured local .onnx/.neo model, then attach 24kHz WAV audio to the project.", "inputSchema": {"type": "object", "properties": {"project": {"type": "string"}, "model": {"type": "string", "description": "Model basename installed in GPTFRAMES_MODELS"}, "text": {"type": "string", "maxLength": 4000}, "start": {"type": "number", "minimum": 0}, "track_name": {"type": "string"}}, "required": ["project", "model", "text"], "additionalProperties": False}},
    {"name": "search_images", "description": "Search licensed images in Wikimedia Commons or Openverse, returning author, source and license information; search results are NOT automatically licensed for all uses.", "inputSchema": {"type": "object", "properties": {"query": {"type": "string"}, "provider": {"type": "string", "enum": ["wikimedia", "openverse"]}, "limit": {"type": "integer", "minimum": 1, "maximum": 15}}, "required": ["query"], "additionalProperties": False}},
    {"name": "download_commons_image", "description": "Download a compatible PNG/JPEG from Wikimedia Commons into project assets, preserving license and attribution in a sidecar JSON.", "inputSchema": {"type": "object", "properties": {"project": {"type": "string"}, "file_title": {"type": "string", "description": "Commons page title, e.g. File:Example.jpg"}, "asset_name": {"type": "string"}}, "required": ["project", "file_title", "asset_name"], "additionalProperties": False}},
]


def _slug(raw: object) -> str:
    if not isinstance(raw, str) or not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9_-]{0,39}", raw):
        raise ValueError("El nombre admite 1–40 caracteres: letras, números, guion y guion bajo")
    return raw


def _project(name: object) -> tuple[Path, Path]:
    base = PROJECTS / _slug(name)
    if not base.is_dir() or base.is_symlink() or base.resolve().parent != PROJECTS:
        raise FileNotFoundError("Proyecto no encontrado")
    file = base / "project.json"
    if not file.is_file() or file.is_symlink():
        raise ValueError("Falta project.json")
    return base, file


def _call_binary(args: list[str], timeout: int = 75) -> str:
    if not ENGINE.is_file():
        raise RuntimeError("Ejecutable GPTFrames no encontrado: compile el motor")
    result = subprocess.run([str(ENGINE), *args], capture_output=True, text=True, timeout=timeout, check=False)
    if result.returncode:
        raise RuntimeError((result.stderr or result.stdout or "Motor falló")[-1500:])
    return result.stdout.strip()[-1500:]


def _model(raw: object) -> Path:
    if not isinstance(raw, str) or not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9_.-]{0,100}\.(onnx|neo)", raw):
        raise ValueError("Modelo admitido: nombre.onnx o nombre.neo")
    resolved = (MODELS / raw).resolve()
    if resolved.parent != MODELS or not resolved.is_file():
        raise FileNotFoundError("Modelo local no encontrado en GPTFRAMES_MODELS")
    return resolved


def _run_tool(name: str, args: dict) -> object:
    if not isinstance(args, dict):
        raise ValueError("arguments debe ser un objeto JSON")
    if name == "list_projects":
        return {"projects": sorted(p.name for p in PROJECTS.glob("*") if p.is_dir() and not p.is_symlink() and (p / "project.json").is_file())[:100]}
    if name == "create_project":
        PROJECTS.mkdir(parents=True, exist_ok=True)
        dest = PROJECTS / _slug(args.get("name"))
        if dest.exists():
            raise FileExistsError("El proyecto ya existe")
        fixture = ROOT / "examples" / "json-showcase-v2"
        shutil.copytree(fixture / "assets", dest / "assets")
        (dest / "project.json").write_bytes((fixture / "showcase-v2.json").read_bytes())
        return {"project": dest.name, "project_json": str(dest / "project.json"), "tip": "Personaliza project.json para animar textos, imágenes y keyframes."}
    if name == "search_images":
        return {"results": search_images(args.get("query", ""), args.get("provider", "wikimedia"), args.get("limit", 6))}
    if name == "download_commons_image":
        base, _ = _project(args.get("project"))
        result = download_commons_image(args.get("file_title", ""), base / "assets", _slug(args.get("asset_name")))
        result["src"] = "assets/" + Path(result["file"]).name
        return result
    if name == "validate_project":
        _, f = _project(args.get("project"))
        return {"message": _call_binary(["--project", "validate", str(f)])}
    if name == "preview_frame":
        base, f = _project(args.get("project"))
        second = args.get("second")
        if not isinstance(second, (int, float)) or not 0 <= second <= 240:
            raise ValueError("Segundo no válido")
        out = base / "preview.png"
        msg = _call_binary(["--project", "frame", str(f), str(out), str(second)])
        return {"message": msg, "preview_path": str(out)}
    if name == "render_video":
        base, f = _project(args.get("project"))
        seconds = args.get("seconds")
        if seconds is not None and (not isinstance(seconds, (float, int)) or not .1 <= seconds <= MAX_RENDER_SECONDS):
            raise ValueError("Render máximo 60s por llamada MCP")
        source = json.loads(f.read_text(encoding="utf-8"))
        scenes = source.get("scenes") or []
        total = (scenes[-1].get("start", 0) + scenes[-1].get("duration", 0)) if scenes else 0
        if seconds is None and total > MAX_RENDER_SECONDS:
            raise ValueError("Proyecto de más de 60s: indique una duración menor")
        out = base / "render.mp4"
        params = ["--project", "render", str(f), str(out)]
        if seconds is not None:
            params.append(str(seconds))
        msg = _call_binary(params, timeout=240)
        return {"message": msg, "video_path": str(out), "bytes": out.stat().st_size}
    if name == "synthesize_voice":
        base, project_file = _project(args.get("project"))
        text = args.get("text")
        start = args.get("start", 0)
        if not isinstance(text, str) or not 1 <= len(text.strip()) <= 4000:
            raise ValueError("Texto de voz 1–4000 caracteres")
        if not isinstance(start, (float, int)) or not 0 <= start <= 240:
            raise ValueError("Inicio de voz 0–240s")
        model = _model(args.get("model"))
        voice_id = _slug(args.get("track_name", "narracion"))
        audio_dir = base / "assets" / "voices"
        audio_dir.mkdir(parents=True, exist_ok=True)
        wav = audio_dir / (voice_id + ".wav")
        txt = audio_dir / (voice_id + ".txt")
        if wav.exists() or txt.exists():
            raise FileExistsError("Nombre de pista existente")
        txt.write_text(text, encoding="utf-8")
        try:
            result = _call_binary(["--voice", "synth", "--model", str(model), "--text-file", str(txt), "--out", str(wav), "--neo-lite", NEO_LITE], timeout=180)
            doc = json.loads(project_file.read_text(encoding="utf-8"))
            if not isinstance(doc.get("audio", []), list) or len(doc.get("audio", [])) >= 12:
                raise ValueError("El proyecto admite como máximo 12 pistas")
            doc.setdefault("audio", []).append({"src": "assets/voices/" + wav.name, "start": start, "gain": 1})
            # Commit atomically only after valid inference. The C++ engine validates
            # the WAV and the project on the next render.
            with tempfile.NamedTemporaryFile("w", encoding="utf-8", dir=base, prefix=".project-", suffix=".json", delete=False) as temp:
                temp.write(json.dumps(doc, ensure_ascii=False, indent=2) + "\n")
                temp_name = temp.name
            Path(temp_name).replace(project_file)
            return {"message": result, "wav_path": str(wav), "project": base.name}
        except Exception:
            txt.unlink(missing_ok=True)
            wav.unlink(missing_ok=True)
            raise
    raise ValueError("Herramienta no reconocida")


def _rpc(req: dict) -> dict | None:
    if not isinstance(req, dict) or req.get("jsonrpc") != "2.0":
        return {"jsonrpc": "2.0", "id": req.get("id") if isinstance(req, dict) else None, "error": {"code": -32600, "message": "Invalid Request"}}
    method = req.get("method")
    if method == "notifications/initialized":
        return None
    if "id" not in req:
        return None
    response = {"jsonrpc": "2.0", "id": req["id"]}
    if method == "initialize":
        response["result"] = {"protocolVersion": "2025-06-18", "capabilities": {"tools": {"listChanged": False}}, "serverInfo": {"name": "gptframes", "version": "0.1.0"}}
    elif method == "ping":
        response["result"] = {}
    elif method == "tools/list":
        response["result"] = {"tools": TOOLS}
    elif method == "tools/call":
        params = req.get("params") or {}
        try:
            result = _run_tool(params.get("name", ""), params.get("arguments", {}))
            response["result"] = {"content": [{"type": "text", "text": json.dumps(result, ensure_ascii=False)}], "isError": False}
        except Exception as exc:
            response["result"] = {"content": [{"type": "text", "text": f"{type(exc).__name__}: {exc}"}], "isError": True}
    elif method == "resources/list":
        response["result"] = {"resources": []}
    else:
        response["error"] = {"code": -32601, "message": "Method not found"}
    return response


def serve_stdio() -> None:
    for line in sys.stdin:
        if len(line) > 1_000_000:
            continue
        try:
            msg = _rpc(json.loads(line))
            if msg is not None:
                print(json.dumps(msg, ensure_ascii=False), flush=True)
        except Exception as exc:
            print(json.dumps({"jsonrpc": "2.0", "id": None, "error": {"code": -32700, "message": str(exc)[:300]}}), flush=True)


def serve_http(port: int) -> None:
    token = os.environ.get("GPTFRAMES_MCP_TOKEN", "")
    if len(token) < 24:
        raise SystemExit("HTTP MCP requiere GPTFRAMES_MCP_TOKEN de al menos 24 caracteres")
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *unused):
            return

        def _auth(self) -> bool:
            return hmac.compare_digest(self.headers.get("Authorization", ""), "Bearer " + token)

        def do_GET(self):
            if self.path == "/health":
                body = b'{"status":"ok"}'
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(body)))
                self.end_headers()
                self.wfile.write(body)
            else:
                self.send_error(405)

        def do_POST(self):
            if self.path != "/mcp":
                self.send_error(404)
                return
            if not self._auth():
                self.send_response(401)
                self.send_header("WWW-Authenticate", 'Bearer realm="GPTFrames"')
                self.send_header("Content-Length", "0")
                self.end_headers()
                return
            size = int(self.headers.get("Content-Length") or 0)
            if size <= 0 or size > 1_000_000:
                self.send_error(413)
                return
            try:
                result = _rpc(json.loads(self.rfile.read(size)))
                if result is None:
                    self.send_response(202)
                    self.send_header("Content-Length", "0")
                    self.end_headers()
                    return
                payload = json.dumps(result, ensure_ascii=False).encode("utf-8")
            except Exception as exc:
                payload = json.dumps({"jsonrpc": "2.0", "id": None, "error": {"code": -32700, "message": str(exc)[:200]}}).encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Cache-Control", "no-store")
            self.send_header("Content-Length", str(len(payload)))
            self.end_headers()
            self.wfile.write(payload)
    ThreadingHTTPServer(("127.0.0.1", port), Handler).serve_forever()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--transport", choices=["stdio", "http"], default="stdio")
    parser.add_argument("--port", type=int, default=8769)
    args = parser.parse_args()
    if args.transport == "stdio":
        serve_stdio()
    else:
        if not 1024 <= args.port <= 65535:
            raise SystemExit("Puerto fuera de rango")
        serve_http(args.port)


if __name__ == "__main__":
    main()
