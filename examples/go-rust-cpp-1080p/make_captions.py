#!/usr/bin/env python3
"""Per-chapter animated Spanish closed captions. Text stays within safe bottom margin."""
import json,math,re
from pathlib import Path
ROOT=Path(__file__).resolve().parent
doc=json.loads((ROOT/"storyboard.json").read_text(encoding="utf8"))
out=ROOT/"motion-captions";out.mkdir(exist_ok=True)
def stamp(t):
    t=max(0.,t)
    cs=round(t*100);h,cs=divmod(cs,360000);m,cs=divmod(cs,6000);sec,cs=divmod(cs,100)
    return f"{h}:{m:02d}:{sec:02d}.{cs:02d}"
header='''[Script Info]
Title: GPTFrames subtítulos animados Capybara
ScriptType: v4.00+
PlayResX: 1920
PlayResY: 1080
ScaledBorderAndShadow: yes
WrapStyle: 2

[V4+ Styles]
Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding
Style: Narrator,DejaVu Sans,32,&H00E7FFFF,&H00E7FFFF,&H99051421,&HA8050B15,1,0,0,0,100,100,1,0,3,1,1,2,150,150,133,1

[Events]
Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
'''
for scene in doc["scenes"]:
    words=scene["narration"].split()
    # break in meaningful chunks, not individual (distracting) words
    group=11
    chunks=[" ".join(words[i:i+group]) for i in range(0,len(words),group)]
    length=scene["d"]-.85
    offset=.43
    piece=length/max(1,len(chunks))
    lines=[]
    for i,t in enumerate(chunks):
        start=offset+i*piece
        end=min(scene["d"]-.3,offset+(i+1)*piece)
        clean=t.replace("{","").replace("}","").replace("\\","")
        lines.append(f"Dialogue: 0,{stamp(start)},{stamp(end)},Narrator,,0,0,0,,{{\\fad(170,220)}}{clean}")
    file=out/f"scene-{scene['index']+1:02d}.ass"
    file.write_text(header+"\n".join(lines)+"\n",encoding="utf-8")
    print("CAPTIONS",scene["index"]+1,len(lines))
