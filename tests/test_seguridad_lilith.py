"""Unit checks for the Lilith defensive-web educational video."""
import json,unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
EX=ROOT/"examples/seguridad-web-lilith"

class SecurityLilithVideoTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((EX/"storyboard.json").read_text(encoding="utf8"))
    def test_chapters_timing_and_fps(self):
        d=self.data
        self.assertEqual((d["width"],d["height"],d["fps"]),(1920,1080,30))
        self.assertEqual(len(d["scenes"]),13)
        self.assertEqual(d["duration"],280)
        self.assertEqual(d["fps"]*d["duration"],8400)
        current=0
        for sc in d["scenes"]:
            self.assertEqual(sc["start"],current)
            self.assertGreater(sc["duration"],10)
            current+=sc["duration"]
        self.assertEqual(current,d["duration"])
    def test_introduction_and_defensive_recommendations(self):
        alltext=" ".join(sc["narration"] for sc in self.data["scenes"]).lower()
        self.assertIn("tema complejo",alltext)
        self.assertIn("breve introducción",alltext)
        self.assertIn("consultas parametrizadas",alltext)
        self.assertIn("autenticación multifactor",alltext)
        self.assertIn("copias de seguridad",alltext)
        self.assertIn("solo en sistemas propios o con autorización",alltext)
    def test_owasp_source_documented(self):
        self.assertTrue(any("top10.owasp.org/2025" in s for s in self.data["sources"]))
        self.assertTrue((EX/"README.md").exists())
        self.assertTrue((EX/"model-provenance.json").exists())
    def test_animated_layers_and_real_voice(self):
        code=(EX/"render_fast.py").read_text()
        self.assertIn("overlay=x=",code)
        self.assertIn("overlay=0:0", (EX/"prepare_small_layers.py").read_text())
        self.assertIn("scale=1920:1080",code)
        self.assertNotIn("ass=filename",code)
        tts=(EX/"synthesize.py").read_text()
        self.assertIn("neo-lite",tts)
        self.assertIn("lilith.neo",tts)
    def test_no_model_files_in_public_paths(self):
        pub=(EX/"publish.py").read_text()
        self.assertNotIn("copy2(R/\"voice/lilith.neo\"",pub)
        self.assertIn("fuentes-y-licencias.json",pub)

if __name__=="__main__":
    unittest.main()
