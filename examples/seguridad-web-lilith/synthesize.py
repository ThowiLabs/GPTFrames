#!/usr/bin/env python3
"""Lilith NEO direct CPU synthesis, deterministic chapter soundtrack, no music."""
import array,json,subprocess,wave
from pathlib import Path
R=Path(__file__).resolve().parent
doc=json.loads((R/'storyboard.json').read_text())
model=R/'voice/lilith.neo'
assert model.is_file()
RATE=24000
buf=array.array('h',[0])*(doc['duration']*RATE)
folder=R/'voice/wav';folder.mkdir(exist_ok=True)
measure=[]
for s in doc['scenes']:
    ix=s['index']+1
    wav=folder/f'lilith-{ix:02d}.wav'
    if not wav.exists():
        subprocess.run(['build/neo-lite/neo-lite','infer','--model',str(model),'--text-file',str(R/'text'/f'narracion-{ix:02d}.txt'),'--out',str(wav),'--device','cpu'],check=True,timeout=120)
    with wave.open(str(wav)) as wf:
        original=wf.getnframes()/wf.getframerate()
    slot=s['duration']-1.1
    tempo=max(1.,original/slot)
    if tempo>1.65:raise RuntimeError(f"Chapter {ix} too long: {original:.2f} seconds")
    piece=folder/f'prepared-{ix:02d}.wav'
    subprocess.run(['ffmpeg','-nostdin','-hide_banner','-loglevel','error','-y','-i',str(wav),
      '-af',f'atempo={tempo:.5f},highpass=f=75,volume=1.02',
      '-ar',str(RATE),'-ac','1','-c:a','pcm_s16le',str(piece)],check=True)
    with wave.open(str(piece)) as w:
        a=array.array('h');a.frombytes(w.readframes(w.getnframes()))
    start=round((s['start']+.46)*RATE)
    limit=min(int((s['duration']-.6)*RATE),len(a),len(buf)-start)
    for j in range(limit):
        fade=min(1,j/1000,(limit-j)/1800)
        buf[start+j]=max(-32768,min(32767,round(a[j]*fade*.92)))
    measure.append(dict(scene=ix,raw_seconds=round(original,2),tempo=round(tempo,3),sample_count=limit))
    print('LILITH_NEO_CHAPTER',ix,'/',len(doc['scenes']),'duration',round(original,2),'tempo',round(tempo,3),flush=True)
output=R/'output/lilith-voiceonly.wav'
output.parent.mkdir(exist_ok=True)
with wave.open(str(output),'wb') as w:
    w.setnchannels(1);w.setsampwidth(2);w.setframerate(RATE);w.writeframes(buf.tobytes())
(R/'output/voice-timing.json').write_text(json.dumps(measure,indent=2))
print('LILITH_FULL_VOICE_READY',output,doc['duration'],flush=True)
