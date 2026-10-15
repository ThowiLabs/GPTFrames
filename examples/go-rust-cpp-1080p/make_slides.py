#!/usr/bin/env python3
"""Generate editorial 1920x1080 static slide masters for documentary rendering."""
from __future__ import annotations
from pathlib import Path
import html
import json
import subprocess
import shutil
import textwrap

ROOT=Path(__file__).resolve().parent
scenes=json.loads((ROOT/"storyboard.json").read_text(encoding="utf-8"))["scenes"]
sources={s["key"]:s for s in json.loads((ROOT/"assets-sources.json").read_text())}
dest=ROOT/"slides"
dest.mkdir(exist_ok=True)

def esc(v):return html.escape(str(v),quote=True)
def image(path,cls="",style=""):
    return f'<img class="{cls}" style="{style}" src="../{esc(path)}">'
def photo(scene):
    key=scene.get("asset","")
    p=ROOT/"assets"/(key+".jpg")
    if p.is_file():return image("assets/"+key+".jpg","photo")
    return ""
def footer(scene):
    src=sources.get(scene.get("asset",""),{})
    credit=f'RECURSO VISUAL  ·  {src.get("author","Wikimedia Commons")}  ·  {src.get("license","")} ' if src else 'COMPARACIÓN EDITORIAL  ·  SIN DATOS DE BENCHMARK INVENTADOS'
    return f'<footer><div class="footer-logo">COMPARATIVA <span>· GO · RUST · C++</span></div><div class="foot-source">{esc(credit)}</div><div class="foot-line"></div></footer>'

def panel(title,note,accent):
    return f'<div class="feature-chip" style="--tone:{accent}"><strong>{esc(title)}</strong><span>{esc(note)}</span></div>'

def standard(scene):
    sty=scene["style"];key=scene["asset"];accent=scene["accent"]
    body=''
    if sty=="hero":
        body=f'''
        <div class="big-hero">
         <div class="eyebrow"><i></i> GO · RUST · C++ / PERFORMANCE</div>
         <h1 class="display">GO.<br><span class="rust">RUST.</span><br><span class="cpp">C++.</span></h1>
         <p class="headline-sub">TRES CAMINOS.<br>UNA SOLA PREGUNTA: ¿QUIÉN GANA?</p>
        </div>
        <div class="hero-icons">
          <div class="hero-icon">{image("assets/go-logo.png")}</div>
          <div class="hero-icon">{image("assets/rust-logo.png")}</div>
          <div class="hero-icon">{image("assets/cpp-logo.png")}</div>
        </div>
        '''
    elif sty=="metrics":
        body=f'''<div class="section-banner">{photo(scene)}<div class="bar-copy">
           <p class="hint">NO TODO ES UNA CIFRA</p>
           <h2>VELOCIDAD<br><span>≠ UNA SOLA</span><br>MÉTRICA</h2>
           <div class="metrics">{panel("LATENCIA","tiempo por petición",accent)}{panel("THROUGHPUT","tareas procesadas",accent)}
           {panel("MEMORIA","RAM y asignaciones",accent)}{panel("PRODUCTIVIDAD","tiempo de entrega",accent)}</div>
          </div></div>'''
    elif sty=="language":
        color={"go-logo":"#53DFFF","rust-logo":"#F3B27B","cpp-logo":"#A697FF"}.get(key,accent)
        tag={"go-logo":"CONCURRENCIA ACCESIBLE","rust-logo":"OWNERSHIP + BORROWING","cpp-logo":"CONTROL Y ECOSISTEMA"}.get(key,"")
        body=f'''<div class="language-text">
           <p class="hint">PERFIL DEL LENGUAJE</p>
           <h2>{esc(scene["title"])}</h2>
           <p class="section-sub">{esc(scene["subtitle"])}</p>
           <div class="lang-tags">{panel("RENDIMIENTO","compilación nativa",color)}
           {panel("DISEÑO","compromisos conscientes",color)}</div>
          </div>
          <div class="logo-orbit" style="--tone:{color}"><div class="orbit one"></div><div class="orbit two"></div><div class="logo-card">{photo(scene) if (ROOT/"assets"/(key+".jpg")).exists() else image("assets/"+key+".png")}</div><span class="orbit-label">{esc(tag)}</span></div>'''
    elif sty=="benefits":
        specific=({"go-logo":[("GOROUTINES","concurrencia fácil"),("TOOLING","buenas herramientas"),("DEPLOY","binarios sencillos")],
          "code":[("SEGURIDAD","sin GC por defecto"),("PERFORMANCE","abstracciones ligeras"),("FIABILIDAD","menos fallos de memoria")]})
        cards=specific.get(key,specific["code"])
        body=f'''<div class="cards-section"><h2>{esc(scene["title"])}</h2><p class="section-sub">{esc(scene["subtitle"])}</p>
            <div class="benefit-cards">{"".join(panel(t,n,accent) for t,n in cards)}</div></div><div class="photo-shape">{photo(scene)}</div>'''
    elif sty=="drawbacks":
        isgo="GO" in scene["title"] or scene["start"]<90
        cards=([("GC","puede usar más memoria"),("LATENCIA","pausas en ciertas cargas"),("CONTROL","menos ajuste fino")]
           if isgo else [("APRENDIZAJE","ownership y lifetimes"),("COMPILACIÓN","proyectos exigentes"),("INTEGRACIÓN","ecosistemas heredados")])
        body=f'''<div class="risk-layout"><div class="risk-image">{photo(scene)}</div>
          <div class="risk-copy"><p class="hint">LA OTRA CARA</p><h2>{esc(scene["title"])}</h2>
          <div class="risk-cards">{"".join(panel(t,n,accent) for t,n in cards)}</div></div></div>'''
    elif sty=="cpptrade":
        body=f'''<div class="trade-wrap"><div class="trade-copy"><h2>LIBERTAD<br><em>TOTAL.</em></h2><p class="section-sub">UN POTENCIAL ENORME.<br>UNA RESPONSABILIDAD ENORME.</p>
          <div class="trade-chips">{panel("VENTAJA","control de memoria, ABI y hardware",accent)}{panel("RIESGO","errores de memoria y complejidad",accent)}</div></div><div class="trade-image">{photo(scene)}</div></div>'''
    elif sty=="table":
        rows=[
          ("Rendimiento pico","ALTO","ALTO","ALTO"),
          ("Seguridad de memoria","GC / SAFE","OWNERSHIP","DEPENDE DEL CÓDIGO"),
          ("Recolección de basura","SÍ","NO","NO"),
          ("Curva de aprendizaje","MÁS SUAVE","EXIGENTE","EXIGENTE"),
          ("Concurrencia","GOROUTINES","TIPOS / ASYNC","HILOS / BIBLIOTECAS"),
          ("Control de hardware","MEDIO","MUY ALTO","MUY ALTO"),
          ("Mejor encaje","SERVICIOS","NUEVOS SISTEMAS","LEGADO / MOTORES")]
        body=f'''<div class="table-wrap"><div class="table-kicker">ANÁLISIS MULTICRITERIO / SIN BENCHMARKS FALSOS</div>
        <div class="comp-table"><div class="theads"><div class="blank">CRITERIO</div><div class="go-head">GO</div><div class="rust-head">RUST</div><div class="cpp-head">C++</div></div>
        {"".join('<div class="tr"><div class="metric-name">'+esc(row[0])+'</div><div>'+esc(row[1])+'</div><div>'+esc(row[2])+'</div><div>'+esc(row[3])+'</div></div>' for row in rows)}
        </div><div class="table-note">* Comparación cualitativa; los resultados reales dependen del algoritmo, compilador y carga.</div></div>'''
    elif sty=="scenarios":
        body=f'''<div class="case-layout"><div><p class="hint">GUÍA DE ELECCIÓN</p><h2>EL PROBLEMA<br><em>DEFINE AL REY.</em></h2>
        <div class="case-list">{panel("GO  →  BACKEND","microservicios y redes", "#53DFFF")}
        {panel("RUST  →  SISTEMAS","seguridad y performance", "#F3B27B")}
        {panel("C++  →  MOTORES","hardware y código heredado", "#A697FF")}</div></div>
        <div class="case-photo">{photo(scene)}</div></div>'''
    elif sty=="verdict":
        body=f'''<div class="winner"><div class="winner-glow"></div><p class="hint">VEREDICTO / OPINIÓN RAZONADA</p>
        <h2>NO HAY REY<br><span>UNIVERSAL.</span></h2>
        <p>RUST SE LLEVA LA CORONA DEL EQUILIBRIO.<br>C++ DEL CONTROL. GO DE LA PRODUCTIVIDAD.</p>
        <div class="win-columns">{panel("RUST","rendimiento + seguridad","#F3B27B")}
         {panel("C++","control + legado","#A697FF")}{panel("GO","simplicidad + servicios","#53DFFF")}</div>
        </div>
        <div class="subscribe-cta">
          <div class="subscribe-play">▶</div>
          <p class="subscribe-kicker">SI TE GUSTÓ ESTE VÍDEO</p>
          <div class="subscribe-heading">SUSCRÍBETE<br><span>A HIRCOIR</span></div>
          <p class="subscribe-desc">DALE ME GUSTA · COMPARTE · NOS VEMOS EN EL PRÓXIMO</p>
        </div>'''
    return body

css=r'''
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@500;600;700;800;900&display=swap');
*{box-sizing:border-box}html,body{width:1920px;height:1080px;margin:0;overflow:hidden}
body{font-family:Inter,Arial,sans-serif;color:#f1f8ff;background:#07101c}
.stage{position:relative;width:1920px;height:1080px;overflow:hidden;isolation:isolate;
 background:radial-gradient(circle at 63% 44%,#18374b 0,#0e1b2f 30%,#07111e 78%)}
.stage::before{content:"";z-index:-1;position:absolute;inset:0;background:
 linear-gradient(90deg,#060d1b 0%,transparent 80%),repeating-linear-gradient(90deg,#ffffff08 0,#ffffff08 1px,transparent 1px,transparent 90px)}
.stage::after{content:"";position:absolute;inset:0;pointer-events:none;
 background:linear-gradient(0deg,#020816 1%,transparent 22%),radial-gradient(circle at 18% 15%,transparent 0,#00000030 75%)}
.topbar{position:absolute;top:0;left:0;right:0;height:126px;display:flex;align-items:center;justify-content:space-between;
 padding:0 96px;border-bottom:1px solid #dceeff23;z-index:3;letter-spacing:3px;font-size:22px;color:#a8cfdf;font-weight:800}
.brand{font-size:29px;color:#fff;letter-spacing:-1px}.brand b{color:#5cf4db}
.tab{color:var(--accent);background:#d0faff11;border:1px solid #cfffff32;border-radius:99px;padding:11px 22px;font-size:16px;letter-spacing:2px}
.content{position:absolute;top:135px;left:0;right:0;bottom:103px;z-index:2;padding:54px 100px}
footer{position:absolute;bottom:0;left:0;right:0;height:92px;padding:0 96px;z-index:4;display:flex;align-items:center;gap:22px;
 background:#071020eb;border-top:1px solid #ffffff23}
.footer-logo{font-size:24px;font-weight:900}.footer-logo span{color:#68ffe7;font-weight:600;font-size:15px}
.foot-source{margin-left:auto;text-align:right;color:#91a8bd;font-size:14px;max-width:940px;white-space:nowrap}
.foot-line{width:36px;height:5px;border-radius:8px;background:var(--accent)}
h2{font-weight:900;font-size:90px;letter-spacing:-4px;line-height:1.06;margin:0 0 34px}
em{font-style:normal;color:var(--accent)}
p{margin:0}.hint{color:var(--accent);font-size:25px;letter-spacing:4px;font-weight:900;margin-bottom:28px}
.section-sub{font-size:27px;color:#b5cbdc;line-height:1.5;letter-spacing:1px}
.feature-chip{border:1px solid color-mix(in srgb,var(--tone) 48%,transparent);background:linear-gradient(130deg,#183449bd,#111c30e8);
 box-shadow:0 20px 55px #04081570;border-radius:25px;padding:32px;display:flex;flex-direction:column;gap:15px;min-width:0}
.feature-chip strong{color:var(--tone);font-size:27px;letter-spacing:1px}.feature-chip span{color:#e4edf7;font-size:20px;line-height:1.3}
.big-hero{position:absolute;top:25px;left:100px;width:950px}.eyebrow{font-size:25px;color:var(--accent);letter-spacing:4px;font-weight:900;display:flex;align-items:center;gap:16px}
.eyebrow i{width:14px;height:14px;border-radius:50%;background:var(--accent);box-shadow:0 0 35px var(--accent)}
h1.display{font-size:185px;line-height:.87;letter-spacing:-12px;margin:70px 0 34px;color:#fff}
h1.display .rust{color:#f5b87e}h1.display .cpp{color:#9d9dff}
.headline-sub{font-size:34px;line-height:1.4;color:#c7e1ea;font-weight:700}
.hero-icons{position:absolute;right:150px;top:160px;width:570px;height:600px;display:flex;flex-direction:column;justify-content:center;gap:22px;
 filter:drop-shadow(0 20px 50px #0009)}
.hero-icon{align-self:center;width:230px;height:160px;background:linear-gradient(130deg,#ffffffec,#cbe0edeb);border-radius:27px;
 display:grid;place-items:center;transform:rotate(8deg);border:1px solid #fff8}
.hero-icon:nth-child(2){transform:rotate(-9deg);margin-right:200px}.hero-icon:nth-child(3){transform:rotate(9deg);margin-left:80px}
.hero-icon img{width:170px;height:114px;object-fit:contain}
.section-banner{height:100%;display:flex;position:relative;border-radius:30px;overflow:hidden;background:#172333;border:1px solid #ffffff38}
.section-banner .photo{height:100%;width:48%;object-fit:cover;opacity:.68;filter:saturate(.7)}
.bar-copy{position:absolute;top:50px;left:700px;right:50px}.bar-copy h2{font-size:100px}.bar-copy h2 span{color:var(--accent)}
.metrics{display:grid;grid-template-columns:repeat(2,1fr);gap:18px}.metrics .feature-chip{padding:25px}.metrics strong{font-size:22px}.metrics span{font-size:17px}
.language-text{position:absolute;left:102px;top:125px;width:940px}.language-text h2{font-size:94px;max-width:920px}.lang-tags{display:flex;gap:20px;margin-top:70px}.lang-tags .feature-chip{min-width:240px}
.logo-orbit{position:absolute;right:142px;top:70px;width:640px;height:650px;border:2px solid #7fffe242;border-radius:50%;
 background:radial-gradient(circle,#c0ffe636,#ffffff05 48%,transparent 68%);box-shadow:0 0 155px #2ceac125;display:flex;align-items:center;justify-content:center}
.orbit{position:absolute;width:93%;height:63%;border:2px solid #9eedec64;border-radius:50%;transform:rotate(35deg)}
.orbit.two{transform:rotate(-39deg);height:78%}.logo-card{width:385px;height:385px;background:#edf3f7ed;border-radius:65px;display:grid;place-items:center;box-shadow:0 35px 100px #0007}
.logo-card img{max-width:290px;max-height:290px;object-fit:contain}
.orbit-label{position:absolute;bottom:28px;font-weight:900;color:var(--tone);letter-spacing:3px;font-size:19px}
.cards-section{position:absolute;left:100px;top:110px;width:1110px}.cards-section h2{font-size:100px;max-width:1100px}.cards-section .section-sub{margin-bottom:65px}
.benefit-cards{display:flex;gap:18px}.benefit-cards .feature-chip{width:330px;min-height:235px}
.photo-shape{position:absolute;right:78px;top:146px;width:580px;height:540px;transform:rotate(5deg);border:15px solid #ffffff18;border-radius:35px;overflow:hidden;box-shadow:0 0 140px #0007}
.photo-shape .photo{width:100%;height:100%;object-fit:cover}
.risk-layout{display:flex;gap:95px;width:100%;height:100%}.risk-image{width:640px;overflow:hidden;border-radius:35px;transform:rotate(-3deg);box-shadow:0 25px 100px #0009}
.risk-image .photo{width:100%;height:100%;object-fit:cover}.risk-copy{flex:1}.risk-copy h2{font-size:84px}.risk-cards{display:grid;grid-template-columns:repeat(2,1fr);gap:16px}.risk-cards .feature-chip{padding:23px}
.trade-wrap{display:flex;height:100%;gap:55px}.trade-copy{width:55%}.trade-copy h2{font-size:120px}.trade-chips{display:flex;gap:18px;margin-top:70px}.trade-chips .feature-chip{flex:1}
.trade-image{width:43%;border-radius:30px;overflow:hidden;transform:rotate(2deg);box-shadow:0 20px 100px #0008}.trade-image .photo{height:100%;width:100%;object-fit:cover}
.table-wrap{position:absolute;inset:8px 80px 12px}.table-kicker{font-size:23px;color:var(--accent);letter-spacing:3px;font-weight:800;margin-bottom:19px}
.comp-table{border-radius:22px;overflow:hidden;border:1px solid #ffffff4c;background:#0a1c2fbb;box-shadow:0 20px 65px #0006}
.theads,.tr{display:grid;grid-template-columns:1.6fr 1fr 1fr 1fr;align-items:center}
.theads{height:100px;background:#183950;font-size:32px;font-weight:900;text-align:center}
.theads .blank{text-align:left;color:#a8c3d7;font-size:24px;padding-left:28px}.go-head{color:#62dcfa}.rust-head{color:#efad76}.cpp-head{color:#b1a2ff}
.tr{height:72px;border-top:1px solid #ffffff20;font-size:23px;text-align:center;font-weight:600}.tr:nth-child(2n){background:#ffffff08}
.tr .metric-name{text-align:left;padding-left:28px;color:#ecf8ff;font-weight:800;font-size:24px}
.table-note{margin-top:14px;color:#a3b9c9;font-size:20px}
.case-layout{display:grid;grid-template-columns:1.1fr .9fr;height:100%;gap:60px}.case-layout h2{font-size:100px}.case-list{display:flex;flex-direction:column;gap:17px;margin-top:35px}.case-list .feature-chip{padding:21px}
.case-photo{border-radius:30px;overflow:hidden;transform:rotate(3deg)}.case-photo .photo{width:100%;height:100%;object-fit:cover}
.winner{position:relative;text-align:center;top:30px}.winner h2{font-size:155px;line-height:1}.winner h2 span{color:var(--accent)}
.winner>p:not(.hint){font-size:33px;color:#c5e1e6;font-weight:650;line-height:1.6}.win-columns{display:grid;grid-template-columns:repeat(3,1fr);gap:24px;margin-top:65px}
.winner-glow{position:absolute;left:30%;top:50px;width:700px;height:650px;background:radial-gradient(circle,#54eacd22,transparent 62%);pointer-events:none}

.subscribe-cta{position:absolute;inset:0;z-index:10;border-radius:21px;background:radial-gradient(circle at 68% 45%,#143f49 0%,#0a2434 43%,#07111e 88%);box-shadow:inset 0 0 150px #0007;border:1px solid #72faea55;display:flex;flex-direction:column;align-items:center;justify-content:center;text-align:center;gap:22px}
.subscribe-play{display:grid;place-items:center;width:140px;height:105px;border-radius:30px;background:#ef384e;box-shadow:0 15px 85px #f84a4e49;font-size:65px;color:#fff;padding-left:8px}
.subscribe-kicker{letter-spacing:6px;font-size:24px;color:#a3dfdf;font-weight:800}
.subscribe-heading{font-size:120px;line-height:1.05;letter-spacing:-4px;font-weight:900;color:#fff}
.subscribe-heading span{color:#64ffdb}
.subscribe-desc{font-size:23px;font-weight:800;letter-spacing:1px;color:#bfd4df}

'''
def make_html(s):
    title=esc(s["title"]);accent=esc(s["accent"])
    top=f'<div class="topbar"><span class="brand">GO <b>·</b> RUST <b>·</b> C++</span><span class="tab">CAPYBARA / VOZ ESPAÑOLA</span><span>{esc(s["k"])}</span></div>'
    return f'<!DOCTYPE html><html lang="es"><head><meta charset="UTF-8"><style>:root{{--accent:{accent}}}{css}</style></head><body><div class="stage">{top}<div class="content">{standard(s)}</div>{footer(s)}</div></body></html>'

for scene in scenes:
    path=dest/f'scene-{scene["index"]+1:02d}.html'
    path.write_text(make_html(scene),encoding="utf-8")
    print("SLIDE",path.name,scene["style"])
print("DONE")
