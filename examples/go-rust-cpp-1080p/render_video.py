#!/usr/bin/env python3
"""Render 243s of 1920x1080 documentary animation from generated HTML masters."""
from __future__ import annotations
import json
import subprocess
import time
from pathlib import Path

ROOT=Path(__file__).resolve().parent
doc=json.loads((ROOT/"storyboard.json").read_text(encoding="utf-8"))
fps=doc["fps"]
out=ROOT/"output"
out.mkdir(exist_ok=True)
clips=[]
started=time.time()

for slide in doc["scenes"]:
    n=slide["index"]+1
    source=ROOT/"slides"/f"scene-{n:02d}.png"
    if not source.exists():
        raise FileNotFoundError(source)
    duration=slide["d"]
    target=out/f"clip-{n:02d}.mp4"
    in_fade="0.4"
    fade_out=max(0.,duration-.55)
    # Slow Ken Burns optical zoom with scene-local fade. No pagination overlays.
    filters=[
      "zoompan=z='min(zoom+0.000065,1.037)':x='(iw-iw/zoom)/2':y='(ih-ih/zoom)/2':d=1:fps=24:s=1920x1080",
      f"fade=t=in:st=0:d={in_fade}",
      f"fade=t=out:st={fade_out}:d=0.55",
      "format=yuv420p",
    ]
    command=[
      "ffmpeg","-nostdin","-hide_banner","-loglevel","error","-y",
      "-loop","1","-framerate",str(fps),"-i",str(source),
      "-vf",",".join(filters),"-frames:v",str(fps*duration),
      "-an","-c:v","libx264","-preset","ultrafast","-crf","24",
      "-threads","3",str(target)
    ]
    if not target.is_file() or target.stat().st_size<100_000:
        subprocess.run(command,check=True,timeout=330)
    clips.append(target)
    print(f"CLIP {n:02d}/{len(doc['scenes'])} {duration}s bytes={target.stat().st_size} elapsed={time.time()-started:.1f}s",flush=True)

playlist=out/"playlist.txt"
playlist.write_text("".join("file '"+str(f.resolve()).replace("'","'\\''")+"'\n" for f in clips))
video=out/"GPTFrames-Go-Rust-Cpp-1920x1080-Capybara.mp4"
mix=out/"capybara-narracion-musica.wav"
if not mix.is_file():
    raise FileNotFoundError(mix)
subprocess.run([
  "ffmpeg","-nostdin","-hide_banner","-loglevel","error","-y",
  "-f","concat","-safe","0","-i",str(playlist),
  "-i",str(mix),
  "-map","0:v:0","-map","1:a:0",
  "-c:v","copy","-c:a","aac","-b:a","160k","-ar","48000",
  "-movflags","+faststart","-t",str(doc["duration"]),str(video)
],check=True,timeout=180)
print("FINAL_VIDEO",video,video.stat().st_size,"TOTAL",doc["duration"],"s",flush=True)
