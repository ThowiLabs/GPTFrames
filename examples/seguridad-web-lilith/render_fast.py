#!/usr/bin/env python3
"""CPU-conscious 30 FPS Lilith security video: 3 independently animated groups."""
from pathlib import Path
import argparse,json,subprocess
R=Path(__file__).resolve().parent
D=json.loads((R/'storyboard.json').read_text())
L=R/'layers-720';O=R/'output/clips';O.mkdir(exist_ok=True)
FPS=30
def clip(sc):
    i=sc["index"]+1
    dur=sc["duration"]
    paths=[L/f"{kind}-{i:02d}.png" for kind in ("bg","title","content","tags")]
    for p in paths:
        if not p.is_file():raise FileNotFoundError(p)
    inputs=[]
    for p in paths:inputs+=["-loop","1","-framerate","30","-i",str(p)]
    fx=[
      "[0:v]format=yuv420p,scale=1300:732:flags=bilinear,crop=1280:720:x='10+8*sin(n/105)':y='6+5*cos(n/108)'[base]",
      "[1:v]format=rgba,fade=t=in:st=0.2:d=0.85:alpha=1[title]",
      "[base][title]overlay=x='0+3*sin(t/3)':y='if(lt(t,1.2),-105*(1-max(0,t-.2)),0)':eval=frame:format=auto[a]",
      "[2:v]format=rgba,fade=t=in:st=1.55:d=0.9:alpha=1[content]",
      "[a][content]overlay=x='if(lt(t,2.65),-290*(1-max(0,(t-1.55)/1.1)),0)+4*sin(t/4)':y='0':eval=frame:format=auto[b]",
      "[3:v]format=rgba,fade=t=in:st=4.4:d=0.9:alpha=1[tags]",
      "[b][tags]overlay=x='0':y='if(lt(t,5.55),140*(1-max(0,(t-4.4)/1.15)),0)+3*sin(t/3.1)':eval=frame:format=auto[c]",
      f"[c]fade=t=in:st=0:d=0.25,fade=t=out:st={dur-.5}:d=0.5,scale=1920:1080:flags=bilinear,format=yuv420p[v]"
    ]
    out=O/f"chapter-{i:02d}.mp4"
    subprocess.run(["ffmpeg","-nostdin","-hide_banner","-loglevel","error","-y",*inputs,
      "-filter_complex_threads","1","-filter_complex",";".join(fx),"-map","[v]",
      "-frames:v",str(dur*FPS),"-t",str(dur),"-an","-c:v","libx264","-preset","ultrafast",
      "-crf","23","-threads","2",str(out)],check=True,timeout=600)
    print("LILITH_SECURITY_ANIMATED",i,"/",len(D["scenes"]),"bytes",out.stat().st_size,flush=True)
    return out
parser=argparse.ArgumentParser()
parser.add_argument("--only",type=int)
args=parser.parse_args()
if args.only:
    clip(D["scenes"][args.only-1]);raise SystemExit(0)
files=[clip(sc) for sc in D["scenes"]]
playlist=O/"playlist.txt"
playlist.write_text("".join("file '"+str(p.resolve())+"'\n" for p in files))
audio=R/"output/lilith-voiceonly.wav"
if not audio.is_file():raise FileNotFoundError(audio)
final=R/"output/Vulnerabilidades-Web-Lilith-1080p-30fps.mp4"
subprocess.run(["ffmpeg","-nostdin","-hide_banner","-loglevel","error","-y",
  "-f","concat","-safe","0","-i",str(playlist),"-i",str(audio),
  "-map","0:v:0","-map","1:a:0","-c:v","copy","-c:a","aac","-b:a","160k",
  "-t",str(D["duration"]),"-movflags","+faststart",str(final)],check=True,timeout=180)
print("LILITH_DOCUMENTARY_FULL_MP4",final,final.stat().st_size,flush=True)
