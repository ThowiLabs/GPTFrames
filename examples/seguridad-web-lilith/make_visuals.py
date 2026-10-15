#!/usr/bin/env python3
"""Original layered security motion-graphics backgrounds, no external images."""
from __future__ import annotations
import html,json,subprocess
from pathlib import Path
R=Path(__file__).resolve().parent
doc=json.loads((R/'storyboard.json').read_text())
layer=R/'layers';layer.mkdir(exist_ok=True)
def esc(v):return html.escape(str(v),quote=True)
CSS=r"""
:root{--accent:#5FE7E3}*{box-sizing:border-box}
html,body{margin:0;padding:0;width:1920px;height:1080px;overflow:hidden}
body{font-family:Inter,"DejaVu Sans",Arial,sans-serif;color:#f2fbfe}
.stage{position:relative;width:1920px;height:1080px;overflow:hidden;
background:radial-gradient(ellipse at 80% 0%,color-mix(in srgb,var(--accent) 14%,transparent),transparent 48%),
radial-gradient(ellipse at 0% 100%,#163351b0,transparent 64%),linear-gradient(145deg,#050b16,#0a1728 62%,#061221)}
.stage::before{content:"";position:absolute;inset:0;opacity:.25;
background-image:linear-gradient(#98c7e311 1px,transparent 1px),linear-gradient(90deg,#98c7e311 1px,transparent 1px);
background-size:64px 64px;mask-image:linear-gradient(to bottom,transparent,#000 16%,#000 84%,transparent)}
.stage::after{content:"";position:absolute;width:800px;height:800px;left:1210px;top:330px;
border:1px solid color-mix(in srgb,var(--accent) 29%,transparent);border-radius:50%;
box-shadow:0 0 140px color-mix(in srgb,var(--accent) 16%,transparent);transform:rotate(-17deg)}
.orbit{position:absolute;border:1px dashed color-mix(in srgb,var(--accent) 14%,transparent);border-radius:50%;}
.orbit.o1{width:600px;height:600px;left:1330px;top:430px}
.orbit.o2{width:950px;height:950px;left:-720px;top:350px}
.line{position:absolute;left:93px;right:93px;height:1px;background:linear-gradient(90deg,var(--accent),#fff1,#0000);top:101px;opacity:.65}
.topbar{position:absolute;left:100px;right:100px;top:46px;display:flex;justify-content:space-between;align-items:center}
.topbar span{color:var(--accent);font-weight:800;letter-spacing:3px;font-size:18px}
.topbar aside{font-weight:700;color:#8fabbb;font-size:17px;letter-spacing:1.5px}
.foot{position:absolute;bottom:35px;left:102px;right:102px;color:#84a1b4;font-size:14px;letter-spacing:2px;
display:flex;justify-content:space-between;text-transform:uppercase}
.topic{position:absolute;top:139px;left:101px;right:105px;min-height:210px}
.kicker{color:var(--accent);font-size:19px;letter-spacing:3.6px;font-weight:900}
h1{font-family:"DejaVu Sans",Arial,sans-serif;font-size:75px;line-height:1.09;letter-spacing:-3.7px;
font-weight:900;margin:16px 0 17px;white-space:pre-line;text-shadow:0 12px 40px #0007}
.subtitle{color:#a8d3de;font-size:26px;letter-spacing:.1px;font-weight:600}
.pill{display:inline-block;margin-left:18px;border:1px solid var(--accent);border-radius:30px;
color:var(--accent);padding:7px 17px;font-size:16px;vertical-align:middle}
.diagram{position:absolute;top:438px;left:95px;width:1048px;height:335px;padding:26px 35px;
border:1px solid color-mix(in srgb,var(--accent) 45%,transparent);border-radius:22px;
background:linear-gradient(135deg,#122941ef,#0e1c31e9);
box-shadow:0 25px 85px #0007}
.section-label{display:flex;align-items:center;gap:10px;color:var(--accent);font-size:16px;
font-weight:900;letter-spacing:2.6px}
.dot{width:11px;height:11px;border-radius:50%;background:var(--accent);box-shadow:0 0 24px var(--accent)}
.nodes{display:flex;gap:7px;align-items:center;justify-content:space-between;margin:27px 2px 19px}
.node{height:105px;min-width:220px;flex:1;display:flex;justify-content:center;align-items:center;
padding:8px 13px;background:#0b1b2c;border:1px solid #b8d6e43b;border-radius:13px;
text-align:center;font-size:20px;line-height:1.3;font-weight:900;color:#e7f4fc}
.node:nth-of-type(2){background:color-mix(in srgb,var(--accent) 20%,#0a1828);border-color:var(--accent)}
.connector{font-size:26px;color:var(--accent);font-weight:900;filter:drop-shadow(0 0 12px var(--accent))}
.mechanism{font-size:19px;line-height:1.5;color:#c2d4e2;margin-top:9px}
.policy{position:absolute;left:1170px;top:438px;width:650px;height:335px;padding:29px 37px;
border:1px solid #70ecbd66;border-radius:22px;
background:linear-gradient(143deg,#09382de8,#0c2530f4 72%,#112e30ea);
box-shadow:0 25px 85px #0007}
.policy .section-label{color:#85ffd5}
.policy h3{font-size:29px;line-height:1.32;letter-spacing:-.3px;color:#fff;margin:28px 0 19px}
.policy p{font-size:23px;line-height:1.54;color:#b7e3d1;max-width:550px;margin:0}
.check{position:absolute;bottom:24px;right:29px;color:#92ffe1;font-size:50px;opacity:.78}
.takeaways{position:absolute;top:808px;left:98px;right:98px;display:flex;gap:20px}
.minitile{height:155px;flex:1;display:flex;align-items:center;gap:24px;
border:1px solid #668ca161;border-radius:17px;
background:linear-gradient(110deg,#0f233aef,#112039e0);
padding:20px 34px}
.minitile .num{font-size:44px;color:var(--accent);font-weight:900;letter-spacing:-1px}
.minitile span{font-size:23px;color:#d4eff4;font-weight:800;letter-spacing:1px}
"""
def render_html(sc, visible, out):
    accent=sc['accent']; i=sc['index'];parts=[]
    nodes=''.join(f'<div class="node">{esc(v)}</div>'+('<div class="connector">→</div>' if j<2 else '') for j,v in enumerate(sc['nodes']))
    parts.append(f'<section class="topic"><div class="kicker">{esc(sc["eyebrow"])}<span class="pill">OWASP · 2025</span></div><h1>{esc(sc["title"])}</h1><div class="subtitle">{esc(sc["subtitle"])}</div></section>')
    parts.append(f'<section class="diagram"><div class="section-label"><div class="dot"></div>{esc(sc["risk"])}</div><div class="nodes">{nodes}</div><p class="mechanism">{esc(sc["mechanism"])}</p></section>')
    parts.append(f'<section class="policy"><div class="section-label"><div class="dot"></div>{esc(sc["defense"])}</div><h3>QUÉ DEBES HACER</h3><p>{esc(sc["protect"])}</p><span class="check">✓</span></section>')
    parts.append(f'<section class="takeaways">'+''.join(f'<div class="minitile"><div class="num">{str(j+1).zfill(2)}</div><span>{esc(v)}</span></div>' for j,v in enumerate(sc['tags']))+'</section>')
    selector=".topic,.diagram,.policy,.takeaways{visibility:hidden!important}" if visible==-1 else """
    html,body{background:transparent!important}
    .stage{background:transparent!important}
    .stage::before,.stage::after,.orbit,.line,.topbar,.foot{display:none!important}
    .topic,.diagram,.policy,.takeaways{visibility:hidden!important}
    """+[".topic",".diagram",".policy",".takeaways"][visible]+"{visibility:visible!important}"
    background=f'<div class="orbit o1"></div><div class="orbit o2"></div><div class="line"></div><div class="topbar"><span>SEGURIDAD WEB / EXPLICACIÓN VISUAL</span><aside>GUÍA DEFENSIVA · {i+1:02d}/{len(doc["scenes"]):02d}</aside></div>'
    footer='<div class="foot"><span>REFERENCIA · OWASP TOP 10:2025</span><span>CASOS ILUSTRATIVOS · SIN PRUEBAS DE INTRUSIÓN</span></div>'
    content=f'<!doctype html><html lang="es"><head><meta charset="utf-8"/><style>:root{{--accent:{accent}}}{CSS}\n{selector}</style></head><body><div class="stage">{background}{footer}{"".join(parts)}</div></body></html>'
    temp=R/'layers'/'temp.html';temp.write_text(content,encoding="utf8")
    subprocess.run(['chromium','--headless','--no-sandbox','--disable-dev-shm-usage','--disable-gpu','--hide-scrollbars',
      '--force-device-scale-factor=1','--window-size=1920,1080','--default-background-color=00000000',
      '--screenshot='+str(out),'file://'+str(temp.resolve())],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True,timeout=40)
    if out.stat().st_size<2000:raise ValueError('Invalid render '+str(out))
for sc in doc['scenes']:
    i=sc['index']+1
    for j in range(-1,4):
        path=layer/f'{"background" if j==-1 else "component-"+str(j+1)}-{i:02d}.png'
        render_html(sc,j,path)
    print('SECURITY_SCENE_LAYERS_READY',i,'/',len(doc['scenes']),flush=True)
(layer/'temp.html').unlink(missing_ok=True)
print('ALL_SECURITY_GRAPHICS_READY',flush=True)
