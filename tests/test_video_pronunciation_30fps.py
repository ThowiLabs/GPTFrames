#!/usr/bin/env python3
"""Regression checks for user-facing, no-brand documentary 30 FPS."""
from pathlib import Path
import subprocess,sys,json
BASE=Path(__file__).resolve().parents[1]
ROOT=BASE/"examples/go-rust-cpp-1080p"
subprocess.run([sys.executable,str(ROOT/"storyboard.py")],check=True,stdout=subprocess.DEVNULL)
story=json.loads((ROOT/"storyboard.json").read_text(encoding="utf-8"))
assert story["fps"]==30 and story["width"]==1920 and story["height"]==1080
assert len(story["scenes"])==13 and story["duration"]>=180
assert story["duration"]*story["fps"]==7290
for chapter in story["scenes"]:
    spoken=chapter["speech_text"]
    assert "GPTFrames" not in spoken and "GPTFRAMES" not in spoken
    assert "Go" not in spoken and "Rust" not in spoken and "C más más" not in spoken
    assert "gou" in spoken or "rast" in spoken or "ce más más" in spoken or chapter["index"]==1
assert "Jírcoir" in story["scenes"][-1]["speech_text"]
assert "suscríbete" in story["scenes"][-1]["speech_text"].lower()
render=(ROOT/"render_motion.py").read_text()
assert "FPS=30" in render and "fps=30" in render
assert "ass=filename" not in render, "No se admiten subtítulos ASS en el vídeo"
slides=(ROOT/"make_slides.py").read_text()
assert "GPTFRAMES" not in slides.upper()
assert "subscribe-cta" in slides and "HIRCOIR" in slides
for page in (ROOT/"slides").glob("scene-*.html"):
    assert "GPTFRAMES" not in page.read_text().upper()
print("PASS: 13 escenas, 1920x1080, 30 FPS, pronunciación, cierre HirCoir, sin marca ni subtítulos")
