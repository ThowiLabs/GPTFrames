#!/usr/bin/env python3
"""Conform Neo Lite narration to seven 8s scene windows without cutting words."""
from pathlib import Path
import subprocess,json
R=Path(__file__).resolve().parent
data=json.loads((R/"narracion.json").read_text())
assert len(data)==7
def call(cmd):
 p=subprocess.run(cmd,capture_output=True,text=True)
 if p.returncode:raise RuntimeError(p.stderr[-1300:])
for i,scene in enumerate(data):
 wav=R/f"voice-{i:02d}.wav"
 data_duration=float(subprocess.check_output([
  "ffprobe","-v","error","-show_entries","format=duration","-of","csv=p=0",str(wav)
 ],text=True))
 # Leaves 0.36 s of quiet intro and 0.2 s of quiet ending.
 # Compress only enough to guarantee audio finishes before scene cut.
 rate=max(1.,data_duration/7.39)
 scene["voice_duration"]=round(data_duration,3)
 scene["audio_atempo"]=round(rate,4)
 filtered=R/f"clip-{i:02d}.wav"
 call(["ffmpeg","-nostdin","-v","error","-y","-i",str(wav),
       "-af",f"atempo={rate:.5f},aresample=24000,adelay=360,apad=whole_len=192000,atrim=end_sample=192000",
       "-ac","1","-ar","24000","-c:a","pcm_s16le",str(filtered)])
file_list=R/"concat.txt"
file_list.write_text("".join(f"file '{(R/f'clip-{i:02d}.wav').resolve()}'\n" for i in range(7)))
audio=R/"narracion-56s.wav"
call(["ffmpeg","-nostdin","-v","error","-y","-f","concat","-safe","0",
      "-i",str(file_list),"-c:a","pcm_s16le",str(audio)])
def ts(seconds):
 ms=round(seconds*1000)
 return f"{ms//3600000:02d}:{ms//60000%60:02d}:{ms//1000%60:02d},{ms%1000:03d}"
srt="\n".join(
  f"{i+1}\n{ts(i*8)} --> {ts((i+1)*8)}\n{v['text']}\n"
  for i,v in enumerate(data))
(R/"Monolitico_vs_Modular_subtitulos.srt").write_text(srt)
(R/"narracion.json").write_text(json.dumps(data,ensure_ascii=False,indent=2)+"\n")
print("VOICE_MIX_READY",audio.stat().st_size,"bytes",[(v["voice_duration"],v["audio_atempo"]) for v in data])
