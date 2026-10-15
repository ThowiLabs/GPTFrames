#!/usr/bin/env python3
"""29 Linux motion assets, storyboard 480p/24fps, 2s per asset."""
from pathlib import Path
import json,re,html
R=Path(__file__).resolve().parent
items=[v for v in json.loads((R/"catalogo.json").read_text())["presets"] if v.get("batch")=="linux-motion-04"]
assert len(items)==29
duration=2.0;total=duration*len(items)
css="""*{box-sizing:border-box}html,body{width:100%;height:100%;margin:0;overflow:hidden}
body{background:radial-gradient(ellipse at 20% 24%,#14253b,#06090e 72%);color:#eef5ff;font-family:ui-monospace,Consolas,monospace}
header{position:absolute;top:26px;left:35px;right:35px;font-size:12px;font-weight:800;letter-spacing:1px;color:#b8c5d2;display:flex;justify-content:space-between}
header em{font-style:normal;color:#7cebc2}
.rule{position:absolute;top:52px;height:2px;left:35px;right:35px;background:#324458}
.module{position:absolute;inset:0;display:flex;align-items:center;gap:32px;padding:73px 37px 32px;visibility:hidden}
.canvas{width:380px;height:330px;flex-shrink:0;display:flex;align-items:center;justify-content:center;overflow:hidden;background:radial-gradient(ellipse at 44% 42%,#1a3042,#0c1726 85%);border:1px solid #355368;border-radius:24px;box-shadow:0 13px 38px #0007}
.canvas .anim-engine{width:370px;height:315px;flex-shrink:0}
.canvas .linux-asset{transform:scale(1.30)}
.content{min-width:0;width:350px}
.number{font-size:12px;font-weight:800;color:#68f0ae;letter-spacing:1.2px;margin:0 0 15px}
.name{font-size:32px;line-height:1.1;font-weight:800;letter-spacing:-.6px;color:#f7fbff;margin:0 0 20px}
.description{font-family:system-ui,sans-serif;font-size:17px;line-height:1.42;color:#c3d2e1;margin:0 0 23px}
.label{font-size:11px;letter-spacing:1px;color:#98acc2;margin:0 0 8px}
.tags{font-size:13px;color:#eecb80;line-height:1.5}
footer{position:absolute;bottom:17px;left:35px;right:35px;display:flex;justify-content:space-between;color:#899cb1;font-size:11px}
"""
panels=[];styles=[]
for i,item in enumerate(items):
    src=(R/"presets"/(item["id"]+".html")).read_text()
    styles.append(re.search(r'<style>(.*?)</style>',src,re.S).group(1))
    body=re.search(r'<body>(.*?)</body>',src,re.S).group(1)
    pos=i/len(items)*100;end=(i+1)/len(items)*100
    css+=(f"\n.module:nth-child({i+1}){{animation:show{i+1} {total:.1f}s linear infinite}}\n"
          f"@keyframes show{i+1}{{0%,{pos:.4f}%{{visibility:hidden}}"
          f"{pos+.0002:.4f}%,{end-.0002:.4f}%{{visibility:visible}}"
          f"{end:.4f}%,100%{{visibility:hidden}}}}\n")
    tags=", ".join(item["tags"][:5])
    panels.append(f'<section class="module" data-id="{item["id"]}">'
      f'<div class="canvas">{body}</div><article class="content">'
      f'<p class="number">{i+1:02d} / 29 · {html.escape(item["category"]).upper()}</p>'
      f'<h1 class="name">{html.escape(item["name"])}</h1>'
      f'<p class="description">{html.escape(item["description"])}</p>'
      f'<p class="label">TAGS · MOVIMIENTO</p><p class="tags">{html.escape(tags)}'
      f'<br>{html.escape(", ".join(item["motion"][:3]))}</p></article></section>')
CSP='<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; script-src \'none\'; connect-src \'none\'; style-src \'unsafe-inline\'; img-src data:; font-src data:">'
body=f'<!doctype html><html lang="es"><head><meta charset="utf-8">{CSP}<style>{"".join(styles)}{css}</style></head><body>'
body+='<header><span>GPTFRAMES / ANIMACIONES LINUX</span><em>29 PRESETS · SIN DEBIAN</em></header><div class="rule"></div>'
body+='<main data-segment-seconds="2">'+''.join(panels)+'</main>'
body+='<footer><span>CSS ANIMADO EN CHROMIUM / MCP AUTO-INDEXADO</span><span>480p · 24 FPS</span></footer></body></html>'
(R/"catalogo-linux-04.html").write_text("\n".join(line.rstrip() for line in body.splitlines())+"\n")
print("LINUX_GALLERY",len(body),"bytes",total,"s")
