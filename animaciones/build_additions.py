"""Convert owner-supplied source HTML to 17 unique standalone motion presets.
Builder only; the rendered assets use zero Python or JavaScript dependencies.
Run: python3 animaciones/build_additions.py
"""
from __future__ import annotations
import re, json, html, importlib.util
from pathlib import Path
from hashlib import sha256

R=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location("owner_base",R/"build_collection.py")
base=importlib.util.module_from_spec(spec)
spec.loader.exec_module(base)
source=Path("/workspace/kit_usuario_02.html")
curated=R/"source_curated_02.json"
data=source.read_text(encoding="utf-8") if source.is_file() else ""

def cards(source):
    """Extract nested <div class=card> without a third-party HTML dependency."""
    result=[]
    token=re.compile(r'<div\b[^>]*>|</div\s*>',re.I)
    starts=[m for m in token.finditer(source) if re.match(r'<div\s+class="card"',m.group(),re.I)]
    for start in starts:
        depth=0
        for match in token.finditer(source,start.start()):
            if match.group().startswith('</'):depth-=1
            else:depth+=1
            if depth==0:
                result.append(source[start.end():match.start()])
                break
    return result

src_cards=cards(data) if data else []
if data: assert len(src_cards)==21,len(src_cards)
# 21 supplied cards, four repeat existing smartphone/check/padlock/equalizer.
identifiers=[
("avatar-neutral","Avatar neutral","Personajes"),
("avatar-happy","Avatar feliz","Personajes"),
("avatar-thinking","Avatar pensando","Personajes"),
("avatar-angry","Avatar enojado","Personajes"),
("avatar-screaming","Avatar gritando","Personajes"),
("avatar-sleepy","Avatar durmiendo","Personajes"),
("linux-server","Servidor Linux","Hardware"),
("moving-car","Automóvil animado","Hardware"),
("analog-clock","Reloj analógico","Hardware"),
("tachometer","Tacómetro","Hardware"),
None,None,None,
("notification-bell","Campana activa","Notificaciones"),
("curtain-wipe","Cortina wipe","Transiciones"),
("diagonal-split","División diagonal","Transiciones"),
("iris-reveal","Revelado iris","Transiciones"),
("data-chart","Gráfica dinámica","Datos"),
("radial-neon","Loader radial neón","Loaders"),
("shimmer-progress","Barra shimmer","Loaders"),
None,
]
assert len(identifiers)==len(src_cards)
new=[]
if data:
    for card,spec in zip(src_cards,identifiers):
        if spec is None:continue
        slug,title,category=spec
        css_blocks=re.findall(r'<style\b[^>]*>(.*?)</style\s*>',card,flags=re.I|re.S)
        assert css_blocks,slug
        css="\n".join(css_blocks)
        body=re.sub(r'<style\b[^>]*>.*?</style\s*>','',card,flags=re.I|re.S)
        body=re.sub(r'<span\b[^>]*class="card-label"[^>]*>.*?</span\s*>','',body,count=1,flags=re.I|re.S).strip()
        if '<script' in body.lower() or 'onload=' in body.lower() or 'onclick=' in body.lower():
            raise RuntimeError("Unsafe inline JS")
        new.append((slug,title,category,css,body))
    curated.write_text(json.dumps([{"id":slug,"name":title,"category":cat,"css":css,"html":markup}
                for slug,title,cat,css,markup in new],ensure_ascii=False,indent=2))
else:
    if not curated.is_file():raise RuntimeError("Missing curated animation source")
    new=[(x["id"],x["name"],x["category"],x["css"],x["html"])
         for x in json.loads(curated.read_text())]
assert len(new)==17,len(new)

all_styles="\n".join(x[3] for x in new)
shared_base=new[0][3] # Avatar .avatar-base/.eyes-pair reused by other faces

owner_vars="""
:root {--bg-main:#0a0d14;--card-bg:#131924;--border-color:#232d3f;
--text-main:#f8fafc;--text-dim:#94a3b8;--c-neon-blue:#00e5ff;--c-pink:#ff007f;
--c-green:#10b981;--c-yellow:#fbbf24;--c-red:#f43f5e;--c-purple:#8b5cf6;
--ease-elastic:cubic-bezier(.34,1.56,.64,1);
--ease-cinematic:cubic-bezier(.16,1,.3,1);}
"""
# Asset-specific proportional styling (only motion graphics, not controls).
def scale_for(slug):
    if slug.startswith("avatar-"):return 2.0
    if slug in ("radial-neon","analog-clock","tachometer"):return 1.9
    if slug in ("notification-bell",):return 2.3
    if slug in ("moving-car","data-chart","linux-server"):return 1.18
    if slug.endswith(("wipe","split","reveal")):return 1.15
    return 1.5

shell_css="""*{box-sizing:border-box}
html,body{margin:0;width:100%;height:100%;overflow:hidden}
body{background:transparent;color:#f8fafc;font-family:system-ui,"Segoe UI",Arial,sans-serif}
.anim-engine{position:relative;width:320px;height:300px;display:grid;place-items:center}
.asset-body{width:260px;height:220px;display:flex;flex-direction:column;align-items:center;justify-content:center;transform-origin:center}
"""
csp='<meta http-equiv="Content-Security-Policy" content="default-src &#39;none&#39;; script-src &#39;none&#39;; connect-src &#39;none&#39;; style-src &#39;unsafe-inline&#39;; img-src data:; font-src data:">'
new_records=[]
for i,(slug,title,category,css,body) in enumerate(new,7):
    scale=scale_for(slug)
    preset_css=shell_css+owner_vars+shared_base+"\n"+css
    preset_html=('<!doctype html><html lang="es"><head><meta charset="utf-8">'+csp+'<style>'+
          preset_css+'</style></head><body><div class="anim-engine"><div class="asset-body" style="transform:scale('+str(scale)+')">'+body+
          '</div></div></body></html>')
    (R/"presets"/(slug+".html")).write_text(preset_html,encoding="utf-8")
    new_records.append({
        "number":i,"id":slug,"name":title,"category":category,
        "description":{
            "avatar-neutral":"Personaje sereno con respiración ligera y parpadeo.",
            "avatar-happy":"Rostro alegre que salta y sonríe.",
            "avatar-thinking":"Personaje pensativo acompañado de una pregunta flotante.",
            "avatar-angry":"Expresión de enfado con temblor y cejas inclinadas.",
            "avatar-screaming":"Reacción enérgica con sacudida y globo de símbolos.",
            "avatar-sleepy":"Personaje somnoliento con letras Z ascendentes.",
            "linux-server":"Rack Linux con LEDs, actividad y terminal parpadeante.",
            "moving-car":"Vehículo estilizado con ruedas y carretera animadas.",
            "analog-clock":"Reloj de tres manecillas en movimiento constante.",
            "tachometer":"Indicador de velocidad con aguja y círculo progresivo.",
            "notification-bell":"Campana que oscila y muestra una insignia de alerta.",
            "curtain-wipe":"Cortina que cubre y descubre una escena.",
            "diagonal-split":"Transición que reúne dos fragmentos diagonales.",
            "iris-reveal":"Máscara circular que revela progresivamente el contenido.",
            "data-chart":"Barras ascendentes acompañadas por una línea de tendencia.",
            "radial-neon":"Indicador radial luminoso con giro y trazado progresivo.",
            "shimmer-progress":"Barra de carga con brillo desplazándose sobre su gradiente."
        }[slug],
        "style":category.lower()+" / neon / CSS",
        "backend":"browser","collection":"animaciones",
        "preset_file":"animaciones/presets/"+slug+".html",
        "status":"candidate","duration":2.5,
        "origin":"Adaptado del HTML facilitado por el usuario"
    })

old=json.loads((R/"catalogo.json").read_text())["presets"][:6]
for i,item in enumerate(old,1):
    item["number"]=i
    item["duration"]=2.5
records=old+new_records
assert len(records)==23 and len({x["id"] for x in records})==23
(R/"catalogo.json").write_text(json.dumps({
    "version":1,"name":"Animaciones","description":"Biblioteca HTML/CSS compilada para GPTFrames",
    "total":len(records),"duration":57.5,"presets":records,
    "deduplicated":["phone-vibrate","success-checkmark","padlock-closing","audio-equalizer"]
},indent=2,ensure_ascii=False)+"\n",encoding="utf-8")

# HTML catalog: progress through scenes while each child timeline restarts at its scene start.
CSS=base.RESET+owner_vars+"\n".join(base.CSS.values())+all_styles+"\n"+shell_css+"""
body{background:radial-gradient(ellipse at 25% 27%,#192537,#0a0d14 67%)}
.head{position:absolute;top:20px;left:38px;right:38px;display:flex;align-items:center;justify-content:space-between;color:#afc2d2;font-size:12px;letter-spacing:1px;font-weight:800}
.head strong{color:#00e5ff}.rule{position:absolute;top:52px;left:38px;right:38px;height:2px;background:#283a4d}
.module{position:absolute;inset:0;visibility:hidden;display:flex;align-items:center;gap:29px;padding:74px 37px 40px 37px}
.panel{width:385px;height:335px;border:1px solid #3a4e60;border-radius:23px;display:grid;place-items:center;background:radial-gradient(ellipse at 46% 40%,#1b3343,#121b28 75%);box-shadow:0 16px 42px #0005;flex:none;overflow:hidden}
.information{min-width:0;width:344px}
.index{font-size:12px;letter-spacing:1.4px;color:#55dbff;font-weight:780;margin:0 0 13px}
.title{font-size:31px;font-weight:850;line-height:1.12;letter-spacing:-.55px;margin:0 0 20px;color:#f4fbff}
.description{font-size:17px;line-height:1.4;color:#bfd0dc;margin:0 0 27px}
.lbl{font-size:12px;color:#9bb0c3;letter-spacing:1.4px}
.styled{font-size:15px;color:#f8c46b;font-weight:700;margin:7px 0 0}
.foot{position:absolute;bottom:17px;left:39px;right:39px;display:flex;justify-content:space-between;color:#8197aa;font-size:11px}
"""
components=[]
for i,record in enumerate(records):
    slug=record["id"]
    if i<6:
        num=f"{i+1:02d}"
        asset='<div class="motion-stage p'+num+'">'+base.MARKUP[num]+'</div>'
    else:
        _,_,_,_,body=new[i-6]
        scale=scale_for(slug)
        asset='<div class="anim-engine"><div class="asset-body" style="transform:scale('+str(scale)+')">'+body+'</div></div>'
    start=i/len(records)*100
    end=(i+1)/len(records)*100
    CSS+=(f'\n.module:nth-child({i+1}){{animation:show{i+1} 57.5s linear infinite}}\n'
         f'@keyframes show{i+1}{{0%,{start:.5f}%{{visibility:hidden}}'
         f'{start+.0002:.5f}%,{end-.0002:.5f}%{{visibility:visible}}'
         f'{end:.5f}%,100%{{visibility:hidden}}}}\n')
    components.append('<section class="module" data-id="'+slug+'"><div class="panel">'+asset+
      '</div><article class="information"><p class="index">'+str(i+1).zfill(2)+
      ' / 23 · '+html.escape(record["category"].upper())+'</p><h1 class="title">'+
      html.escape(record["name"])+'</h1><p class="description">'+
      html.escape(record["description"])+'</p><p class="lbl">ESTILO</p><p class="styled">'+
      html.escape(record["style"])+'</p></article></section>')
page=('<!doctype html><html lang="es"><head><meta charset="utf-8">'+csp+
      '<style>'+CSS+'</style></head><body><header class="head"><span>GPTFRAMES / ANIMACIONES</span>'
      '<strong>COLECCIÓN 01+02 · 23 MÓDULOS ÚNICOS</strong></header><div class="rule"></div>'
      '<main data-segment-seconds="2.5">'+''.join(components)+'</main>'
      '<footer class="foot"><span>HTML + CSS · SIN CDN · NAVEGADOR CDP</span>'
      '<span>480p · 24 FPS</span></footer></body></html>')
(R/"catalogo.html").write_text(page,encoding="utf-8")
print("Unique presets:",len(records),"new:",len(new),"HTML",len(page),
      "curated_sha256:",sha256(curated.read_bytes()).hexdigest())
