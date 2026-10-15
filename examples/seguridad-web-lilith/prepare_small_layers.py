#!/usr/bin/env python3
"""Pre-resize visual layers and merge risk + defense into a single motion group.

Small inputs reduce FFmpeg RAM on constrained 4 GiB CPU sandboxes. The final
H.264 stream remains a 1920×1080 30fps documentary with 3 live motion groups.
"""
import subprocess,json
from pathlib import Path
R=Path(__file__).resolve().parent
D=json.loads((R/'storyboard.json').read_text())
src=R/'layers';out=R/'layers-720';out.mkdir(exist_ok=True)
def do(inputs,fx,dest):
    cmd=['ffmpeg','-nostdin','-hide_banner','-loglevel','error','-y']
    for v in inputs:cmd+=['-i',str(v)]
    cmd+=['-filter_complex',fx,'-map','[v]','-frames:v','1',str(dest)]
    subprocess.run(cmd,check=True,timeout=28)
for sc in D['scenes']:
    i=sc['index']+1
    for name,path in [('bg',src/f'background-{i:02d}.png'),('title',src/f'component-1-{i:02d}.png'),('tags',src/f'component-4-{i:02d}.png')]:
        subprocess.run(['ffmpeg','-nostdin','-hide_banner','-loglevel','error','-y','-i',str(path),'-vf','scale=1280:720:flags=bicubic','-frames:v','1',str(out/f'{name}-{i:02d}.png')],check=True,timeout=35)
    do([src/f'component-2-{i:02d}.png',src/f'component-3-{i:02d}.png'],
       '[0:v]scale=1280:720:flags=bicubic,format=rgba[a];[1:v]scale=1280:720:flags=bicubic,format=rgba[b];[a][b]overlay=0:0:format=auto,format=rgba[v]',
       out/f'content-{i:02d}.png')
    print('OPTIMIZED_VISUALS',i,'/',len(D['scenes']),flush=True)
print('ALL_OPTIMIZED_LAYERS_READY')
