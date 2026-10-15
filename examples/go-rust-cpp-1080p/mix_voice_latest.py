#!/usr/bin/env python3
"""Clean Capybara (epoch268/step110398) narration soundtrack.
No synthesizer music. An optional officially-downloaded YouTube Studio
Audio Library track may be supplied by the owner for licensed background.
"""
from __future__ import annotations
import argparse,array,json,math,subprocess,wave
from pathlib import Path
ROOT=Path(__file__).resolve().parent
DOC=json.loads((ROOT/"storyboard.json").read_text())
RATE=24000
TOTAL=DOC["duration"]
OUT=ROOT/"output"
OUT.mkdir(exist_ok=True)
ap=argparse.ArgumentParser()
ap.add_argument("--voice-dir",default=str(ROOT/"voice/latest/wav"))
ap.add_argument("--youtube-audio-library",type=Path,default=None,help="Locally downloaded MP3/WAV from official YouTube Studio Audio Library, with verified rights")
args=ap.parse_args()
samples=array.array("h",[0])*(TOTAL*RATE)
timing=[]
for scene in DOC["scenes"]:
 n=scene["index"]+1
 infile=Path(args.voice_dir)/f"capybara-{n:02d}.wav"
 if not infile.is_file():raise FileNotFoundError(infile)
 with wave.open(str(infile)) as w:
  original=w.getnframes()/w.getframerate()
 window=scene["d"]-1.05
 tempo=max(1.,original/window)
 if tempo>1.7:raise ValueError(f"Clip {n} too long ({tempo:.3f}x)")
 tmp=OUT/f"latest-voice-frag-{n:02d}.wav"
 subprocess.run(["ffmpeg","-nostdin","-hide_banner","-loglevel","error","-y","-i",str(infile),
   "-af",f"atempo={tempo:.6f},highpass=f=70,volume=1.08",
   "-ar",str(RATE),"-ac","1","-c:a","pcm_s16le",str(tmp)],check=True)
 with wave.open(str(tmp)) as w:
  a=array.array("h");a.frombytes(w.readframes(w.getnframes()))
 start=round((scene["start"]+.48)*RATE)
 count=min(len(a),int((scene["d"]-.68)*RATE),len(samples)-start)
 for j in range(count):
  fade=min(1.,j/800.,(count-j)/2200.)
  samples[start+j]=max(-32768,min(32767,int(a[j]*.95*fade)))
 timing.append({"scene":n,"original_s":round(original,3),"tempo":round(tempo,3),"mixed_samples":count})
 print(f"LATEST_CAPYBARA_AUDIO {n:02d}/{len(DOC['scenes'])} speed={tempo:.3f}",flush=True)
out=OUT/"latest-capybara-voiceonly.wav"
with wave.open(str(out),"wb") as w:
 w.setnchannels(1);w.setsampwidth(2);w.setframerate(RATE);w.writeframes(samples.tobytes())
(OUT/"latest-voice-timing.json").write_text(json.dumps(timing,indent=2)+"\n")
if args.youtube_audio_library is not None:
 music=args.youtube_audio_library.resolve()
 if not music.is_file() or music.suffix.lower() not in (".mp3",".wav",".m4a",".flac"):
  raise ValueError("Expected a locally downloaded YouTube Studio Audio Library MP3/WAV/M4A/FLAC")
 # The owner is responsible for confirming Studio's track-specific terms.
 mixed=OUT/"latest-capybara-youtube-library.wav"
 subprocess.run(["ffmpeg","-nostdin","-hide_banner","-loglevel","error","-y",
   "-i",str(out),"-stream_loop","-1","-i",str(music),
   "-filter_complex",
   f"[1:a]aresample={RATE},volume=0.07,afade=t=in:st=0:d=2,afade=t=out:st={TOTAL-3}:d=3[bg];[0:a][bg]amix=inputs=2:duration=first:normalize=0[out]",
   "-map","[out]","-t",str(TOTAL),"-ar",str(RATE),"-ac","1",str(mixed)],check=True)
 out=mixed
print("LATEST_CAPYBARA_TRACK",out,TOTAL,flush=True)
