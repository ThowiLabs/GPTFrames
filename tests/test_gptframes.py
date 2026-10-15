#!/usr/bin/env python3
"""Regression tests for GPTFrames MCP and its media safety boundaries."""
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "mcp"))
import media
import server


class ProtocolTests(unittest.TestCase):
    def test_initialize(self):
        response = server._rpc({"jsonrpc":"2.0","id":1,"method":"initialize","params":{}})
        self.assertEqual(response["result"]["serverInfo"]["name"], "gptframes")
        self.assertEqual(response["result"]["protocolVersion"], "2025-06-18")

    def test_tools_list(self):
        out = server._rpc({"jsonrpc":"2.0","id":2,"method":"tools/list"})
        names = {t["name"] for t in out["result"]["tools"]}
        self.assertTrue({"create_project", "search_images", "synthesize_voice", "download_commons_image", "render_video"} <= names)

    def test_reject_unknown_method(self):
        self.assertEqual(server._rpc({"jsonrpc":"2.0","id":"x","method":"shutdown"})["error"]["code"], -32601)

    def test_notification_no_response(self):
        self.assertIsNone(server._rpc({"jsonrpc":"2.0","method":"notifications/initialized"}))


class SecurityTests(unittest.TestCase):
    def test_reject_image_query_and_host(self):
        with self.assertRaises(ValueError):
            media.search_images("", "wikimedia")
        with self.assertRaises(ValueError):
            media._get_json("http://127.0.0.1/internal")
        with self.assertRaises(ValueError):
            media._get_json("https://evil.example.com/internal")

    def test_reject_redirect(self):
        redirect = media._SafeRedirects()
        with self.assertRaises(ValueError):
            redirect.redirect_request(None, None, 302, "", {}, "https://127.0.0.1/private")

    def test_reject_bad_slug(self):
        for slug in ("../private", "/etc/passwd", "name/other", "", "." ):
            with self.assertRaises(ValueError):
                server._slug(slug)


class ProjectTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.old = server.PROJECTS
        server.PROJECTS = self.root
    def tearDown(self):
        server.PROJECTS = self.old
        self.temp.cleanup()

    def test_project_template_and_validate(self):
        result = server._run_tool("create_project", {"name": "ejemplo"})
        self.assertEqual(result["project"], "ejemplo")
        self.assertTrue((self.root / "ejemplo" / "project.json").is_file())
        with self.assertRaises(FileExistsError):
            server._run_tool("create_project", {"name": "ejemplo"})
        if server.ENGINE.is_file():
            message = server._run_tool("validate_project", {"project": "ejemplo"})["message"]
            self.assertIn("4 escenas", message)

    def test_missing_model_is_not_silent(self):
        server._run_tool("create_project", {"name": "ejemplo"})
        with self.assertRaises(FileNotFoundError):
            server._run_tool("synthesize_voice", {
                "project": "ejemplo", "model": "no-existe.onnx", "text": "Hola"
            })

    def test_directory_traversal_not_accepted(self):
        with self.assertRaises(ValueError):
            server._run_tool("create_project", {"name": "../../escape"})

    def test_rpc_unknown_project_returns_tool_error(self):
        r = server._rpc({"jsonrpc":"2.0","id":1,"method":"tools/call",
                         "params":{"name":"render_video","arguments":{"project":"missing"}}})
        self.assertTrue(r["result"]["isError"])


class ProductionGuidelinesTests(unittest.TestCase):
    def test_skill_parity(self):
        locations = [
            ROOT / ".claude/skills/gptframes-video/SKILL.md",
            ROOT / ".claude/skills/hyperframes-cpp/SKILL.md",
        ]
        for path in locations:
            content = path.read_text(encoding="utf-8")
            self.assertIn("pronuncia", content.lower())
            self.assertIn("animaci", content.lower())
            self.assertIn("estátic", content.lower())
            self.assertIn("analyze_motion", content)
        self.assertIn("synthesize_voice", locations[0].read_text(encoding="utf-8"))

    def test_guide_exposed_in_mcp_tools_list(self):
        result = server._rpc({"jsonrpc": "2.0", "id": 1, "method": "tools/list"})
        names = {t["name"] for t in result["result"]["tools"]}
        self.assertTrue({"get_production_guidelines", "prepare_narration", "analyze_motion"} <= names)
        guidance = server._run_tool("get_production_guidelines", {})
        self.assertIn("PRONUNCIAN", guidance["tts"]["required"])
        self.assertIn("NO crear", guidance["animation"]["required"])
        self.assertIn("30 FPS", guidance["visual"]["format"])

    def test_pronunciation_preserves_visible_terms(self):
        original = "Go, Rust, C++ y HirCoir. HTTP."
        result = server._run_tool("prepare_narration", {"display_text": original})
        self.assertEqual(result["display_text"], original)
        self.assertEqual(result["suggested_spoken_text"], "gou, rast, ce más más y Jírcoir. hache te te pe.")
        self.assertTrue(result["requires_listening"])
        with self.assertRaises(ValueError):
            server._run_tool("prepare_narration", {"display_text": ""})

    def test_motion_detection_rejects_static_and_constant_keyframes(self):
        from production import analyze_motion
        doc = {"scenes": [
            {"duration": 4, "layers": [{"type": "text", "text": "fijo", "x": 10}]},
            {"duration": 4, "layers": [{"type": "text", "text": "falso", "x": 10,
                "keyframes": [{"time": 0, "x": 10}, {"time": 3, "x": 10}]}]},
            {"duration": 4, "layers": [{"type": "text", "text": "salto en cero", "x": 10,
                "keyframes": [{"time": 0, "x": 40}]}]},
            {"duration": 4, "layers": [{"type": "text", "text": "dinámico", "x": 10,
                "keyframes": [{"time": 0, "x": 10}, {"time": 0.5, "x": 40, "opacity": 0.4}]}]},
        ]}
        stats = analyze_motion(doc)
        self.assertEqual(stats["static_scenes"], [1, 2, 3])
        self.assertEqual(stats["scenes"][3]["animated_layers"], 1)
        self.assertFalse(stats["meets_motion_requirement"])

    def test_render_requires_motion_or_explicit_user_exception(self):
        with tempfile.TemporaryDirectory() as temp:
            old = server.PROJECTS
            server.PROJECTS = Path(temp)
            try:
                folder = Path(temp) / "fijo"
                folder.mkdir()
                (folder / "project.json").write_text(json.dumps({"scenes": [{
                    "start": 0, "duration": 2, "layers": [{"type": "text", "text": "estático"}]
                }]}))
                with self.assertRaisesRegex(ValueError, "Escenas estáticas"):
                    server._run_tool("render_video", {"project": "fijo", "seconds": 1})
                with self.assertRaisesRegex(ValueError, "booleano"):
                    server._run_tool("render_video", {"project": "fijo", "allow_static": "true", "seconds": 1})
                def fake_binary(args, timeout=75):
                    self.assertIn("--project", args)
                    self.assertEqual(args[1], "render")
                    Path(args[3]).write_bytes(b"test_mp4")
                    return "render simulado"
                with patch.object(server, "_call_binary", side_effect=fake_binary):
                    result = server._run_tool("render_video", {"project": "fijo", "seconds": 1, "allow_static": True})
                self.assertEqual(result["motion"]["static_scenes"], [1])
            finally:
                server.PROJECTS = old

    def test_synthesize_separates_spoken_and_visible_text(self):
        with tempfile.TemporaryDirectory() as temp:
            old = server.PROJECTS
            server.PROJECTS = Path(temp)
            try:
                server._run_tool("create_project", {"name": "voz"})
                with patch.object(server, "_model", return_value=Path("/tmp/capybara.neo")):
                    with patch.object(server, "_call_binary", return_value="ok"):
                        out = server._run_tool("synthesize_voice", {
                            "project": "voz", "model": "capybara.neo", "text": "gou, rast y Jírcoir",
                            "display_text": "Go, Rust y HirCoir", "track_name": "prueba",
                        })
                self.assertEqual(Path(out["spoken_text_path"]).read_text(), "gou, rast y Jírcoir")
                self.assertEqual(Path(out["display_text_path"]).read_text(), "Go, Rust y HirCoir")
            finally:
                server.PROJECTS = old


if __name__ == "__main__":
    unittest.main(verbosity=2)
