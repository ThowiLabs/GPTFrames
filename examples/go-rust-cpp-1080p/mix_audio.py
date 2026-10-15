#!/usr/bin/env python3
"""Compose Capybara narration per chapter with gentle synthetic background music."""
from __future__ import annotations
import array
import json
import math
import subprocess
import wave
from pathlib import Path

ROOT=Path(__file__).resolve().parent
doc=json.loads((ROOT/"storyboard.json").read_text(encoding="utf-8"))
sr=24000
total=doc["duration"]
pcm=array.array("h",[0])*(total*sr)
voice_dir=ROOT/"voice"/"wav"
output=ROOT/"output"
output.mkdir(exist_ok=True)
analysis=[]
for scene in doc["scenes"]:
    i=scene["index"]+1
    src=voice_dir/f"capybara-{i:02d}.wav"
    if not src.is_file():
        raise FileNotFoundError(src)
    with wave.open(str(src)) as wf:
        duration=wf.getnframes()/wf.getframerate()
    room=scene["d"]-1.1
    speed=max(1.,duration/room)
    # Each narrator clip fits in its scene; the tempo adjustment is explicit and measured.
    if speed>1.65: raise ValueError(f"Narration {i} requires too much speed-up: {speed}")
    short=output/f"voice-{i:02d}.wav"
    subprocess.run(["ffmpeg","-nostdin","-hide_banner","-loglevel","error","-y","-i",str(src),
      "-af",f"atempo={speed:.5f},highpass=f=75,volume=1.12",
      "-ar",str(sr),"-ac","1","-c:a","pcm_s16le",str(short)],check=True)
    with wave.open(str(short)) as wf:
        v=array.array("h");v.frombytes(wf.readframes(wf.getnframes()))
    start=round((scene["start"]+.43)*sr)
    count=min(len(v),int((scene["d"]-.6)*sr),len(pcm)-start)
    for ix in range(count):
        dest=start+ix
        # Fade gently to prevent clicks.
        envelope=min(1.,ix/800,(count-ix)/1700)
        pcm[dest]=max(-32768,min(32767,pcm[dest]+int(v[ix]*.93*envelope)))
    analysis.append({"scene":i,"source_duration":round(duration,2),"speed":round(speed,3),"samples_mixed":count})
print("VOICE_CHAPTERS",analysis,flush=True)
# Original low-volume procedurally generated score, not third-party music.
# Keep dynamic below spoken word; 3-note ambient motif and soft kick.
beat=60/108
tones=(82.4069,110.,130.81,164.81)
for off in range(len(pcm)):
    t=off/sr
    chord=tones[int(t//12)%len(tones)]
    env=min(1.,t/2.,(total-t)/3.)
    if env<=0:continue
    kickphase=(t/beat)%1.
    kick=math.sin(2*math.pi*(50.+20.*math.exp(-22*kickphase))*t)*math.exp(-30*kickphase)
    pad=(math.sin(2*math.pi*chord*t)*.55+math.sin(2*math.pi*chord*1.5*t)*.23+math.sin(2*math.pi*chord*2*t)*.1)
    val=int(1700*env*(pad+.21*kick))
    pcm[off]=max(-32768,min(32767,pcm[off]+val))
with wave.open(str(output/"capybara-narracion-musica.wav"),"wb") as f:
    f.setnchannels(1);f.setsampwidth(2);f.setframerate(sr);f.writeframes(pcm.tobytes())
(output/"voice-timing.json").write_text(json.dumps(analysis,ensure_ascii=False,indent=2)+"\n")
print("FINAL_AUDIO",total,"sec",len(pcm),"samples",flush=True)
