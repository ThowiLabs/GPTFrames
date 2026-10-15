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


if __name__ == "__main__":
    unittest.main(verbosity=2)
