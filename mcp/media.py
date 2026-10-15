"""Search licensed images; downloads only validated Wikimedia Commons raster assets."""
from __future__ import annotations

import html
import json
import re
import socket
from pathlib import Path
from urllib.parse import urlencode, urlparse
from urllib.request import Request, build_opener, HTTPRedirectHandler

USER_AGENT = "GPTFrames/0.1 (media research; attribution preserved)"
MAX_IMAGE_BYTES = 18 * 1024 * 1024
COMMONS_API = "https://commons.wikimedia.org/w/api.php"
OPENVERSE_API = "https://api.openverse.org/v1/images/"


def _get_json(url: str, limit: int = 3_000_000) -> dict:
    parsed = urlparse(url)
    if parsed.scheme != "https" or parsed.hostname not in {"commons.wikimedia.org", "api.openverse.org"}:
        raise ValueError("Fuente de búsqueda no permitida")
    with build_opener().open(Request(url, headers={"User-Agent": USER_AGENT, "Accept": "application/json"}), timeout=15) as response:
        raw = response.read(limit + 1)
    if len(raw) > limit:
        raise ValueError("Respuesta externa demasiado grande")
    return json.loads(raw)


def _text(value: str) -> str:
    return re.sub(r"\s+", " ", html.unescape(re.sub(r"<[^>]*>", "", str(value)))).strip()[:500]


def _metadata(info: dict, key: str) -> str:
    return _text(info.get("extmetadata", {}).get(key, {}).get("value", ""))


def search_images(query: str, provider: str = "wikimedia", limit: int = 6) -> list[dict]:
    if not isinstance(query, str) or not 2 <= len(query.strip()) <= 120:
        raise ValueError("Consulta entre 2 y 120 caracteres")
    if not isinstance(limit, int) or not 1 <= limit <= 15:
        raise ValueError("Límite 1–15")
    if provider == "wikimedia":
        params = {
            "action": "query", "generator": "search", "gsrsearch": query,
            "gsrnamespace": 6, "gsrlimit": limit, "prop": "imageinfo",
            "iiprop": "url|extmetadata", "iiurlwidth": 700, "format": "json",
            "formatversion": 2,
        }
        data = _get_json(COMMONS_API + "?" + urlencode(params))
        results = []
        for page in data.get("query", {}).get("pages", []):
            infos = page.get("imageinfo", [])
            if not infos:
                continue
            info = infos[0]
            url = info.get("thumburl") or info.get("url") or ""
            if urlparse(url).hostname not in {"upload.wikimedia.org", "thumb.wikimedia.org"}:
                continue
            license_name = _metadata(info, "LicenseShortName")
            results.append({
                "provider": "wikimedia", "title": page.get("title", ""),
                "preview_url": url,
                "source_url": info.get("descriptionurl", ""),
                "author": _metadata(info, "Artist"),
                "license": license_name,
                "license_url": _metadata(info, "LicenseUrl"),
                "credit": _metadata(info, "Credit"),
                "attribution_required": _metadata(info, "AttributionRequired"),
                "downloadable": True,
            })
        return results
    if provider == "openverse":
        data = _get_json(OPENVERSE_API + "?" + urlencode({"q": query, "page_size": limit}))
        return [{
            "provider": "openverse",
            "title": result.get("title") or "Untitled",
            "preview_url": result.get("thumbnail") or "",
            "source_url": result.get("foreign_landing_url") or "",
            "author": result.get("creator") or "",
            "license": result.get("license") or "",
            "license_url": result.get("license_url") or "",
            "credit": result.get("attribution") or "",
            "downloadable": False,
            "id": result.get("id") or "",
        } for result in data.get("results", [])]
    raise ValueError("Proveedor admitido: wikimedia, openverse")


class _SafeRedirects(HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        p = urlparse(newurl)
        if p.scheme != "https" or p.hostname not in {"upload.wikimedia.org", "thumb.wikimedia.org"}:
            raise ValueError("Redirección a servidor no autorizado")
        return super().redirect_request(req, fp, code, msg, headers, newurl)


def download_commons_image(file_title: str, asset_directory: Path, basename: str) -> dict:
    if not re.fullmatch(r"[\w -]{1,64}", basename, flags=re.UNICODE) or basename.startswith("."):
        raise ValueError("Nombre de archivo inválido")
    if not isinstance(file_title, str) or not file_title.startswith("File:") or len(file_title) > 240:
        raise ValueError("Se requiere un título de Commons que comience con File:")
    params = {
        "action": "query", "titles": file_title, "prop": "imageinfo",
        "iiprop": "url|extmetadata", "iiurlwidth": 1280, "format": "json",
        "formatversion": 2,
    }
    data = _get_json(COMMONS_API + "?" + urlencode(params))
    page = (data.get("query", {}).get("pages") or [{}])[0]
    if page.get("missing") or not page.get("imageinfo"):
        raise ValueError("Archivo Commons no encontrado")
    info = page["imageinfo"][0]
    url = info.get("thumburl") or info.get("url") or ""
    parsed = urlparse(url)
    if parsed.scheme != "https" or parsed.hostname not in {"upload.wikimedia.org", "thumb.wikimedia.org"}:
        raise ValueError("URL externa no permitida")
    opener = build_opener(_SafeRedirects())
    with opener.open(Request(url, headers={"User-Agent": USER_AGENT}), timeout=25) as response:
        size = int(response.headers.get("Content-Length", "0") or "0")
        if size > MAX_IMAGE_BYTES:
            raise ValueError("Imagen demasiado grande")
        content = response.read(MAX_IMAGE_BYTES + 1)
    if len(content) > MAX_IMAGE_BYTES:
        raise ValueError("Descarga demasiado grande")
    if content.startswith(b"\x89PNG\r\n\x1a\n"):
        ext = ".png"
    elif content.startswith(b"\xff\xd8\xff"):
        ext = ".jpg"
    else:
        raise ValueError("Solo PNG/JPEG; formato incompatible con el compositor")
    asset_directory.mkdir(parents=True, exist_ok=True)
    target = asset_directory / (basename + ext)
    if target.exists():
        raise FileExistsError("La imagen ya existe; elige otro nombre")
    target.write_bytes(content)
    attribution = {
        "filename": target.name,
        "source_title": page.get("title"),
        "source_url": info.get("descriptionurl", ""),
        "author": _metadata(info, "Artist"),
        "license": _metadata(info, "LicenseShortName"),
        "license_url": _metadata(info, "LicenseUrl"),
        "credit": _metadata(info, "Credit"),
        "attribution_required": _metadata(info, "AttributionRequired"),
    }
    sidecar = asset_directory / (basename + ".attribution.json")
    sidecar.write_text(json.dumps(attribution, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    return {"file": str(target), "attribution_file": str(sidecar), **attribution}
