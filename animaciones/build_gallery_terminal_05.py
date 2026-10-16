#!/usr/bin/env python3
"""Gallery of four detailed Linux terminals: each plays for 8 seconds."""
from pathlib import Path
import html,re,json
R=Path(__file__).resolve().parent
records=[x for x in json.loads((R/"catalogo.json").read_text())["presets"] if x.get("batch")=="linux-terminal-05"]
assert len(records)==4
css=""
sections=[]
for i,item in enumerate(records):
    doc=(R/item["preset_file"].split("/")[-2]/(item["id"]+".html")).read_text()
    style=re.search(r'<style>(.*?)</style>',doc,re.S).group(1)
    body=re.search(r'<body>(.*?)</body>',doc,re.S).group(1)
    css+="\n"+style
    st=i*25;ed=(i+1)*25
    css+=(f"\n.module:nth-child({i+1}){{animation:visible{i+1} 32s linear infinite}}\n"
          f"@keyframes visible{i+1}{{0%,{st:.3f}%{{visibility:hidden}}"
          f"{st+.001:.3f}%,{ed-.001:.3f}%{{visibility:visible}}"
          f"{ed:.3f}%,100%{{visibility:hidden}}}}\n")
    sections.append('<section class="module">'
      f'<div class="card-caption"><span class="counter">{i+1:02d} / 04</span>'
      f'<span class="title-text">{html.escape(item["name"])}</span>'
      '<span class="pill">LINUX • ANIMACIÓN</span></div>'
      '<div class="big-terminal">'+body+'</div>'
      f'<div class="description">{html.escape(item["description"])}</div>'
      '</section>')
css+="""
html,body{width:100%;height:100%;margin:0;overflow:hidden}
body{background:radial-gradient(ellipse at 50% 10%,#1a2b40 0%,#07111e 65%,#05070c 100%);color:#eff7ff;font-family:system-ui,"Segoe UI",Arial,sans-serif}
body>header{position:absolute;top:16px;left:34px;right:34px;height:30px;display:flex;align-items:center;justify-content:space-between;color:#a2b7c9;letter-spacing:1px;font-size:11px;font-weight:800}
header strong{color:#70e1bf}
.divider{position:absolute;top:55px;left:34px;right:34px;height:2px;background:linear-gradient(90deg,#2ed9b2 0%,#35526e 55%,#182b40 100%)}
.module{position:absolute;inset:0;visibility:hidden;padding:71px 34px 34px;display:flex;flex-direction:column;gap:9px}
.card-caption{display:flex;gap:13px;align-items:center;height:35px;flex:none}
.counter{font-size:13px;font-weight:850;color:#64e3be;letter-spacing:.8px}
.title-text{font-size:23px;font-weight:820;color:#f0f8ff;letter-spacing:-.5px}
.pill{margin-left:auto;border:1px solid #295345;background:#0d2c27;color:#9be9cc;border-radius:999px;font-size:10px;font-weight:780;padding:6px 10px;letter-spacing:.3px}
.big-terminal{flex:1;min-height:0;border:1px solid #3c526a;border-radius:16px;background:#0b111c;box-shadow:0 15px 38px #0007;overflow:hidden}
.big-terminal .anim-engine{width:100%;height:100%}
.big-terminal .card{border:0;border-radius:14px;box-shadow:none}
.description{flex:none;height:26px;padding:3px 3px 0;color:#a4bdd0;font-size:12px;line-height:1.4}
footer{position:absolute;bottom:11px;right:36px;color:#678296;font-size:10px;letter-spacing:1px}
"""
CSP='<meta http-equiv="Content-Security-Policy" content="default-src &#39;none&#39;; script-src &#39;none&#39;; connect-src &#39;none&#39;; style-src &#39;unsafe-inline&#39;; img-src data:; font-src data:">'
document='<!doctype html><html lang="es"><head><meta charset="utf-8">'+CSP+'<style>'+css+'</style></head><body>'
document+='<header><span>GPTFRAMES / TERMINALES ANIMADAS</span><strong>COLECCIÓN 05 · 4 DEMOSTRACIONES</strong></header><div class="divider"></div>'
document+='<main data-segment-seconds="8">'+''.join(sections)+'</main>'
document+='<footer>HTML + CSS • RENDER DETERMINISTA • 480P 24 FPS</footer></body></html>'
(R/"catalogo-terminal-05.html").write_text(document)
print("TERMINAL_GALLERY",len(document),"32 SECONDS")
