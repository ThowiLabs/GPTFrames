#!/usr/bin/env python3
"""Animated Full HD render: moving individual cards/photos/text, not static slides."""
from __future__ import annotations
import argparse,json,subprocess,time
from pathlib import Path

ROOT=Path(__file__).resolve().parent
DOC=json.loads((ROOT/"storyboard.json").read_text())
LAYERS=ROOT/"motion-layers"
OUT=ROOT/"output"/"motion-30fps"
OUT.mkdir(parents=True,exist_ok=True)
FPS=30

def render(scene,preview_seconds:int|None=None):
    ix=scene["index"]+1
    manifest=json.loads((LAYERS/"manifest.json").read_text())
    count=len(manifest["groups"][scene["style"]])
    duration=preview_seconds if preview_seconds is not None else scene["d"]
    inputs=[]
    for p in [LAYERS/f"bg-{ix:02d}.png"]+[LAYERS/f"layer-{ix:02d}-{i}.png" for i in range(1,count+1)]:
        if not p.is_file():raise FileNotFoundError(p)
        inputs+=["-loop","1","-framerate",str(FPS),"-i",str(p)]
    filters=[
      "[0:v]format=rgba,zoompan=z='min(zoom+0.000075,1.045)':x='(iw-iw/zoom)/2':y='(ih-ih/zoom)/2':d=1:fps=30:s=1920x1080,format=yuva420p[b0]"
    ]
    current="b0"
    for layer in range(1,count+1):
        begin=(14.1 if scene["style"]=="verdict" and layer==count else
               ([0.15,1.55,3.25,5.3][min(layer-1,3)] if count<=5 else 0.25+1.65*(layer-1)))
        fade_d=0.8
        # Different real movements per component: horizontal, vertical, reverse.
        if layer%3==1:
            motion=f"if(lt(t,{begin+1.1:.2f}),-320*(1-max(0,t-{begin:.2f})/1.1),0)"
            y="0"
        elif layer%3==2:
            motion=f"if(lt(t,{begin+1.0:.2f}),320*(1-max(0,t-{begin:.2f})/1.0),0)"
            y="0"
        else:
            motion="0"
            y=f"if(lt(t,{begin+1.2:.2f}),180*(1-max(0,t-{begin:.2f})/1.2),0)"
        filters.append(f"[{layer}:v]format=rgba,fade=t=in:st={begin:.2f}:d={fade_d:.2f}:alpha=1[p{layer}]")
        filters.append(f"[{current}][p{layer}]overlay=x='{motion}':y='{y}':eval=frame:shortest=1:format=auto[b{layer}]")
        current=f"b{layer}"
    # Sin subtítulos ASS: solo motion graphics sin subtítulos flotantes.
    filters.append(f"[{current}]fade=t=in:st=0:d=0.3,fade=t=out:st={max(0,duration-.45):.2f}:d=0.45,format=yuv420p[v]")
    outfile=OUT/f"clip-{ix:02d}.mp4"
    cmd=["ffmpeg","-nostdin","-hide_banner","-loglevel","error","-y",*inputs,
         "-filter_complex",";".join(filters),"-map","[v]",
         "-t",str(duration),"-frames:v",str(FPS*duration),"-an",
         "-c:v","libx264","-preset","ultrafast","-crf","23","-threads","3",str(outfile)]
    subprocess.run(cmd,check=True,timeout=400)
    print(f"MOTION_CLIP {ix:02d}/{len(DOC['scenes'])} {duration}s bytes={outfile.stat().st_size}",flush=True)
    return outfile

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--only",type=int)
    parser.add_argument("--seconds",type=int)
    parser.add_argument("--audio",required=False,help="Narration WAV; background music is not generated")
    args=parser.parse_args()
    clips=[]
    for s in DOC["scenes"]:
        if args.only is not None and args.only!=s["index"]+1:continue
        clips.append(render(s,args.seconds))
    if args.only is not None:return
    playlist=OUT/"concat.txt"
    playlist.write_text("".join("file '"+str(c.resolve())+"'\n" for c in clips))
    audio=Path(args.audio) if args.audio else ROOT/"output"/"capybara-fonetica-30fps.wav"
    if not audio.is_file():raise FileNotFoundError(audio)
    final=OUT/"Go-Rust-Cpp-Capybara-30fps-SinSubtitulos.mp4"
    subprocess.run(["ffmpeg","-nostdin","-hide_banner","-loglevel","error","-y",
       "-f","concat","-safe","0","-i",str(playlist),"-i",str(audio),
       "-map","0:v:0","-map","1:a:0","-c:v","copy","-c:a","aac",
       "-b:a","160k","-ar","48000","-t",str(DOC["duration"]),"-movflags","+faststart",str(final)],
       check=True,timeout=210)
    print("LATEST_MOTION_VIDEO_DONE",str(final),final.stat().st_size,flush=True)
if __name__=="__main__":main()
