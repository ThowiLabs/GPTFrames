#!/usr/bin/env python3
"""Storyboard for Master Motion Suite: nine editable tagged presets, Chromium seekable."""
from pathlib import Path
import json,re,html
R=Path(__file__).resolve().parent
rows=[x for x in json.loads((R/"catalogo.json").read_text())["presets"] if x.get("batch")=="master-motion-suite-03"]
assert len(rows)==9
D=3.5;seconds=D*len(rows)
styles=[]
panels=[]
css="""html,body{height:100%;width:100%;margin:0;padding:0;overflow:hidden}
body{background:radial-gradient(ellipse at 25% 35%,#1c2940,#080b12 68%)!important;font-family:system-ui,"Segoe UI",Arial,sans-serif;color:#f6faff}
header{position:absolute;top:17px;left:32px;right:32px;display:flex;justify-content:space-between;align-items:center;font-size:12px;font-weight:760;color:#acc3d0;letter-spacing:1.2px}
header strong{color:#45e6ed}.rule{position:absolute;top:51px;left:32px;right:32px;height:2px;background:#344458}
.module{position:absolute;inset:0;visibility:hidden;display:flex;gap:34px;align-items:center;padding:68px 37px 37px}
.preview{width:385px;height:339px;flex-shrink:0;border-radius:24px;background:radial-gradient(ellipse at 48% 40%,#1b3141,#0e1726 78%);box-shadow:0 15px 50px #0007;border:1px solid #314a5d;display:flex;align-items:center;justify-content:center;overflow:hidden}
.preview .anim-engine{width:320px!important;height:280px!important;flex-shrink:0;transform-origin:center}
.preview.large .anim-engine{transform:scale(1.45)}.preview.medium .anim-engine{transform:scale(1.14)}
.preview.ticker .anim-engine{width:700px!important;transform:scale(.52);flex-shrink:0}
.copy{width:365px;min-width:0}
.index{font-size:13px;font-weight:800;letter-spacing:1.1px;color:#45e5ed;margin:0 0 15px}
.name{font-size:31px;line-height:1.15;letter-spacing:-.7px;margin:0 0 18px;font-weight:850;color:#f7faff}
.description{font-size:16px;line-height:1.42;color:#bbd0de;margin:0 0 18px}
.label{font-size:11px;color:#8da5b9;text-transform:uppercase;letter-spacing:1.4px;margin:0 0 7px}
.tags{font-size:12px;color:#f2c988;line-height:1.4;margin:0 0 20px}
.fields{font-size:11px;color:#9edee0;line-height:1.4}
footer{position:absolute;left:34px;right:34px;bottom:16px;color:#8ea5bb;display:flex;justify-content:space-between;font-size:11px}
"""
css+="""
.module:last-child{display:block;padding:0}
.module:last-child .copy{position:absolute;left:48px;top:72px;width:730px}
.module:last-child .copy .index{margin:0 0 8px}
.module:last-child .copy .name{font-size:30px;margin:0 0 8px}
.module:last-child .copy .description{font-size:16px;margin:0 0 8px}
.module:last-child .copy .label,.module:last-child .copy .fields{display:none}
.module:last-child .copy .tags{font-size:12px;margin:0}
.module:last-child .preview{position:absolute;left:36px;top:232px;width:776px;height:148px}
.module:last-child .preview .anim-engine{width:760px!important;height:144px!important;transform:none!important}
.module:last-child .preview .stage{width:750px!important;max-width:none!important;height:140px!important}
"""
for i,item in enumerate(rows):
 path=R/"presets"/(item["id"]+".html")
 page=path.read_text()
 sty=re.search(r"<style\b[^>]*>(.*?)</style>",page,re.S|re.I).group(1)
 body=re.search(r"<body>(.*?)</body>",page,re.S|re.I).group(1)
 styles.append(sty)
 pos=i*100/len(rows);end=(i+1)*100/len(rows)
 css+=(f"\n.module:nth-child({i+1}){{animation:show{i+1} {seconds:.1f}s linear infinite}}\n"
       f"@keyframes show{i+1}{{0%,{pos:.5f}%{{visibility:hidden}}{pos+.0002:.5f}%,{end-.0002:.5f}%{{visibility:visible}}"
       f"{end:.5f}%,100%{{visibility:hidden}}}}\n")
 group=("ticker" if item["id"]=="breaking-news-ticker" else
        "large" if item["id"] in ("rf-transmit","rf-receive","editable-cpu","laptop-boot") else "medium")
 tags=", ".join(item.get("tags",[])[:4])
 editable=", ".join(item.get("editable",{}).keys()) or "Solo parámetros visuales"
 panels.append(
   f'<section class="module" data-id="{html.escape(item["id"])}"><div class="preview {group}">{body}</div>'
   '<article class="copy">'
   f'<p class="index">{i+1:02d} / {len(rows):02d} · {html.escape(item["category"]).upper()}</p>'
   f'<h1 class="name">{html.escape(item["name"])}</h1>'
   f'<p class="description">{html.escape(item["description"])}</p>'
   f'<p class="label">Tags</p><p class="tags">{html.escape(tags)}</p>'
   f'<p class="label">Campos editables</p><p class="fields">{html.escape(editable)}</p>'
   '</article></section>')
# Wrap animation CSS first and gallery styling last for predictable presentation.
style="\n".join(styles)+"\n"+css
csp='<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; script-src \'none\'; connect-src \'none\'; style-src \'unsafe-inline\'; img-src data:; font-src data:">'
page='<!doctype html><html lang="es"><head><meta charset="utf-8">'+csp+'<style>'+style+'</style></head><body>'
page+='<header><span>GPTFRAMES / ANIMACIONES EDITABLES</span><strong>COLECCIÓN 03 · 9 PRESETS</strong></header><div class="rule"></div>'
page+='<main data-segment-seconds="3.5">'+''.join(panels)+'</main>'
page+='<footer><span>HTML/CSS + TAGS / MOTOR C++ + CHROMIUM</span><span>848×480 · 24 FPS</span></footer></body></html>'
(R/"catalogo-editable-03.html").write_text(page)
print("Nine preset gallery:",len(page),"bytes",seconds,"seconds")
