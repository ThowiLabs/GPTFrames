#!/usr/bin/env python3
"""Public preview of the looping ticker with visible movement at 480p/24fps."""
from pathlib import Path
import re
base=Path(__file__).resolve().parent
src=(base/"presets"/"breaking-news-ticker.html").read_text()
css=re.search(r'<style>(.*?)</style>',src,re.S).group(1)
markup=re.search(r'<body>(.*?)</body>',src,re.S).group(1)
csp='<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; script-src \'none\'; connect-src \'none\'; style-src \'unsafe-inline\'; img-src data:; font-src data:">'
skin="""
html,body{width:100%;height:100%;overflow:hidden;margin:0}
body{background:radial-gradient(ellipse at 15% 18%,#192d43,#090f1c 65%);color:#eafaff;font-family:system-ui,"Segoe UI",sans-serif}
#ticker-frame{position:absolute;top:220px;left:24px;width:800px;height:145px;display:flex;align-items:center;justify-content:center;background:#101c2c;border:1px solid #37506c;border-radius:16px;box-shadow:0 13px 40px #0005}
#ticker-frame .anim-engine{width:800px;height:139px}
#ticker-frame .stage{width:798px!important;max-width:none!important}
header{position:absolute;left:30px;right:30px;top:26px;display:flex;align-items:center;justify-content:space-between;color:#b6c9da;font-size:12px;font-weight:800;letter-spacing:1.2px}
header em{font-style:normal;color:#58e2e9}
h1{position:absolute;top:75px;left:34px;font-size:32px;line-height:1.1;letter-spacing:-.6px;margin:0;color:#f5f9ff}
.subtitle{position:absolute;left:36px;top:132px;color:#a7c1d7;font-size:17px}
.caption{position:absolute;left:34px;bottom:28px;color:#a2b8ca;font-size:12px}
.stamp{position:absolute;bottom:28px;right:34px;color:#73d8e5;font-size:12px}
"""
html='<!doctype html><html lang="es"><head><meta charset="utf-8">'+csp+'<style>'+css+skin+'</style></head><body>'
html+='<header><span>GPTFRAMES / ANIMACIONES</span><em>NEWS TICKER · CORREGIDO</em></header>'
html+='<h1>Cintillo informativo animado</h1><p class="subtitle">Noticias en desplazamiento continuo y totalmente editables</p>'
html+='<section id="ticker-frame">'+markup+'</section>'
html+='<div class="caption">TITULAR FIJO + MARQUESINA DINÁMICA · HTML/CSS / CHROMIUM</div>'
html+='<div class="stamp">480p · 24 FPS</div></body></html>'
(base/"demo-cintillo-corregido.html").write_text(html)
print("TICKER_DEMO_READY",len(html))
