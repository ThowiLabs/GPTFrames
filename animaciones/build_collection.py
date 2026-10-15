from pathlib import Path
import json, html
R=Path(__file__).resolve().parent
(R/"presets").mkdir(parents=True,exist_ok=True)
items=[
 ("01","smartphone-vibrate","Teléfono vibrando","Dispositivos","Llamada entrante: teléfono vibrando con ondas acústicas.","dark / haptic / cyan"),
 ("02","success-checkmark","Palomita de éxito","Estados","Un círculo se traza y después aparece la verificación.","success / SVG / verde"),
 ("03","padlock-closing","Candado cerrándose","Seguridad","El arco desciende y el cuerpo rebota en el cierre.","security / elastic / cyan"),
 ("04","push-notification","Notificación entrante","Notificaciones","Aviso emergente con deslizamiento y rebote.","UI / pop-in / púrpura"),
 ("05","audio-equalizer","Ecualizador de audio","Audio","Cinco barras con amplitudes y colores oscilantes.","audio / dynamic / cyan"),
 ("06","biometric-scanner","Escáner biométrico","Seguridad","Huella digital con barrido vertical de láser.","biometric / laser / rojo")]
CSS={}
MARKUP={}
RESET="""
:root{--bg:#0d1117;--card-bg:#161b22;--border:#30363d;--text-main:#f0f6fc;--text-dim:#8b949e;--green:#2ea043;--cyan:#38bdf8;--gold:#f59e0b;--red:#ef4444;--purple:#a855f7;--ease-elastic:cubic-bezier(.34,1.56,.64,1);--ease-out:cubic-bezier(.16,1,.3,1)}
*{box-sizing:border-box}html,body{margin:0;width:100%;height:100%;overflow:hidden}
body{background:transparent;color:var(--text-main);font-family:system-ui,"Segoe UI",Arial,sans-serif}
.motion-stage{position:relative;width:320px;height:300px;display:grid;place-items:center;overflow:visible}
"""

CSS["01"]="""
.p01 .stage{position:relative;width:186px;height:220px;display:grid;place-items:center}
.p01 .wave{position:absolute;inset:21px 26px;border:3px solid var(--cyan);border-radius:32px;opacity:0;animation:exp 1.2s cubic-bezier(0,.5,.5,1) infinite}
.p01 .wave.w2{animation-delay:-.4s}
.p01 .phone{position:relative;z-index:2;width:93px;height:177px;padding:12px 8px;display:flex;flex-direction:column;align-items:center;background:#21262d;border:3px solid #596674;border-radius:22px;box-shadow:0 14px 36px #0009;animation:vibe .6s ease-in-out infinite}
.p01 .notch{height:5px;width:31px;background:#30363d;border-radius:4px;margin-bottom:19px}
.p01 .avatar{height:44px;width:44px;border-radius:50%;background:#30363d;display:grid;place-items:center;margin-bottom:25px}
.p01 .call{height:40px;width:40px;border-radius:50%;background:var(--green);display:grid;place-items:center}
@keyframes exp{0%{transform:scale(.9);opacity:.8}100%{transform:scale(1.45);opacity:0}}
@keyframes vibe{0%,100%{transform:translate(0,0) rotate(0)}10%{transform:translate(-3px,2px) rotate(-3deg)}20%{transform:translate(3px,-2px) rotate(3deg)}30%{transform:translate(-3px,-1px) rotate(-2deg)}40%{transform:translate(3px,1px) rotate(2deg)}50%{transform:translate(-2px,2px) rotate(-1deg)}60%{transform:translate(2px,-2px) rotate(1deg)}70%{transform:translate(0,0) rotate(0)}}
"""
MARKUP["01"]='''<div class="stage"><div class="wave"></div><div class="wave w2"></div><div class="phone"><div class="notch"></div><div class="avatar"><svg width="24" height="24" viewBox="0 0 24 24" fill="#8b949e"><circle cx="12" cy="8" r="4"/><path d="M4 20c0-4 4-6 8-6s8 2 8 6"/></svg></div><div class="call"><svg width="20" height="20" viewBox="0 0 24 24" fill="#fff"><path d="M6.62 10.79a15.05 15.05 0 006.59 6.59l2.2-2.2a1 1 0 011.02-.24 11.72 11.72 0 003.66.59 1 1 0 011 1v3.5a1 1 0 01-1 1A17 17 0 013 4a1 1 0 011-1h3.5a1 1 0 011 1 11.72 11.72 0 00.59 3.66 1 1 0 01-.24 1.02l-2.23 2.11z"/></svg></div></div></div>'''
CSS["02"]="""
.p02 .check-svg{width:178px;height:178px;filter:drop-shadow(0 0 13px #2ea04355)}
.p02 .circle{fill:none;stroke:var(--green);stroke-width:5;stroke-linecap:round;stroke-dasharray:283;stroke-dashoffset:283;transform-origin:50px 50px;animation:drawCircle .8s var(--ease-out) forwards}
.p02 .tick{fill:none;stroke:#68f08a;stroke-width:6;stroke-linecap:round;stroke-linejoin:round;stroke-dasharray:60;stroke-dashoffset:60;animation:drawTick .5s var(--ease-elastic) .5s forwards}
@keyframes drawCircle{to{stroke-dashoffset:0;transform:rotate(360deg)}}@keyframes drawTick{to{stroke-dashoffset:0}}
"""
MARKUP["02"]='''<svg class="check-svg" viewBox="0 0 100 100"><circle class="circle" cx="50" cy="50" r="45"/><polyline class="tick" points="28,52 43,67 72,35"/></svg>'''

CSS["03"]="""
.p03 .wrap{width:146px;height:180px;position:relative;display:flex;flex-direction:column;align-items:center;filter:drop-shadow(0 15px 20px #0007)}
.p03 .shackle{width:78px;height:80px;border:12px solid #8b949e;border-bottom:0;border-radius:40px 40px 0 0;position:absolute;top:6px;transform-origin:left bottom;transform:translateY(-22px);animation:lock-snap 1.35s var(--ease-elastic) forwards}
.p03 .body{width:127px;height:104px;background:linear-gradient(145deg,#36424e,#252d38);border:3px solid #566572;border-radius:18px;position:absolute;bottom:0;display:grid;place-items:center;box-shadow:0 10px 35px #0009;animation:lock-hit 1.35s var(--ease-out) forwards}
.p03 .hole{width:13px;height:26px;background:#111a26;border-radius:6px}
@keyframes lock-snap{0%{transform:translateY(-22px) rotate(-10deg);border-color:var(--gold)}40%{transform:translateY(-22px) rotate(0)}50%{transform:translateY(0);border-color:#8b949e}70%,100%{transform:translateY(0);border-color:var(--cyan)}}
@keyframes lock-hit{0%,48%{transform:translateY(0);border-color:#566572}50%{transform:translateY(4px)}58%{transform:translateY(-3px)}70%,100%{transform:translateY(0);border-color:var(--cyan)}}
"""
MARKUP["03"]='<div class="wrap"><div class="shackle"></div><div class="body"><div class="hole"></div></div></div>'
CSS["04"]="""
.p04 .notify{width:295px;background:#21262d;border:1px solid #404953;border-radius:16px;padding:17px;display:flex;align-items:center;gap:14px;box-shadow:0 16px 33px #0009;transform:translateY(-65px);opacity:0;animation:notifyDrop .9s var(--ease-elastic) forwards}
.p04 .icon{width:52px;height:52px;background:var(--purple);border-radius:13px;display:grid;place-items:center;flex-shrink:0;box-shadow:0 0 22px #a855f755}
.p04 .txt h4{margin:0 0 6px;font-size:16px;color:#f0f6fc}.p04 .txt p{margin:0;color:#9aa9bb;font-size:12px;line-height:1.35}
@keyframes notifyDrop{to{transform:translateY(0);opacity:1}}
"""
MARKUP["04"]='''<div class="notify"><div class="icon"><svg width="25" height="25" viewBox="0 0 24 24" fill="none" stroke="#fff" stroke-width="2.5"><path d="M18 8A6 6 0 0 0 6 8c0 7-3 9-3 9h18s-3-2-3-9"/><path d="M13.73 21a2 2 0 0 1-3.46 0"/></svg></div><div class="txt"><h4>Nuevo mensaje</h4><p>Renderizado completado<br>exitosamente.</p></div></div>'''

CSS["05"]="""
.p05 .bars{display:flex;align-items:flex-end;justify-content:center;gap:12px;height:135px}
.p05 .bar{width:20px;height:100%;background:var(--cyan);border-radius:10px;transform-origin:bottom;transform:scaleY(.15)}
.p05 .bar:nth-child(1){animation:jump .8s ease-in-out infinite alternate .1s}
.p05 .bar:nth-child(2){animation:jump .6s ease-in-out infinite alternate .3s}
.p05 .bar:nth-child(3){animation:jump .9s ease-in-out infinite alternate 0s}
.p05 .bar:nth-child(4){animation:jump .7s ease-in-out infinite alternate .4s}
.p05 .bar:nth-child(5){animation:jump .5s ease-in-out infinite alternate .2s}
@keyframes jump{0%{transform:scaleY(.15);background:var(--cyan)}100%{transform:scaleY(1);background:var(--purple)}}
"""
MARKUP["05"]='<div class="bars">'+''.join('<div class="bar"></div>' for _ in range(5))+'</div>'
CSS["06"]="""
.p06 .scan{width:210px;height:210px;position:relative;display:grid;place-items:center;overflow:hidden;border-radius:18px;background:#ffffff05;border:2px dashed #46505e;box-shadow:inset 0 0 34px #192638}
.p06 .finger{width:142px;height:142px;fill:none;stroke:#a1aebf;stroke-width:1.6;stroke-linecap:round;filter:drop-shadow(0 0 11px #99accc44)}
.p06 .laser{position:absolute;left:0;right:0;height:4px;background:var(--red);box-shadow:0 0 15px var(--red),0 0 7px #fff;top:0;animation:sweep 1.8s ease-in-out infinite}
@keyframes sweep{0%,100%{top:0;opacity:0}15%{opacity:1}85%{opacity:1}50%{top:96%}}
"""
MARKUP["06"]='''<div class="scan"><svg class="finger" viewBox="0 0 24 24">
<path d="M12 2a10 10 0 0 0-10 10c0 3 1.5 6 3.5 7.5"/>
<path d="M12 6a6 6 0 0 0-6 6c0 2.5 1.5 4.5 3 5.5"/>
<path d="M12 10a2 2 0 0 0-2 2c0 1.5.5 2.5 1.5 3"/>
<path d="M16 12a4 4 0 0 1-.5 2"/>
<path d="M19.5 9.5A8 8 0 0 1 20 12c0 2.5-1 5-2.5 6.5"/>
</svg><div class="laser"></div></div>'''

CSP=('<meta http-equiv="Content-Security-Policy" content="default-src '
     "'none'; script-src 'none'; connect-src 'none'; "
     "style-src 'unsafe-inline'; img-src data:; font-src data:;\">")
records=[]
for n,slug,name,category,desc,style in items:
    records.append({
        "number":int(n),"id":slug,"name":name,"category":category,
        "description":desc,"style":style,"backend":"browser",
        "collection":"animaciones",
        "preset_file":"animaciones/presets/"+slug+".html",
        "status":"candidate","duration":4.0
    })
    page='<!doctype html><html lang="es"><head><meta charset="utf-8">'+CSP
    page+='<style>'+RESET+CSS[n]+'</style></head><body>'
    page+='<div class="motion-stage p'+n+'">'+MARKUP[n]+'</div></body></html>'
    (R/"presets"/(slug+".html")).write_text(page,encoding="utf-8")
(R/"catalogo.json").write_text(json.dumps({
    "version":1,"name":"Animaciones",
    "description":"Colección HTML/CSS entregada por el usuario y adaptada a GPTFrames",
    "total":len(records),"duration":24,"presets":records
},indent=2,ensure_ascii=False)+"\n",encoding="utf-8")

GALLERY=RESET+"\n".join(CSS.values())+"""
html,body{width:100%;height:100%}
body{background:radial-gradient(ellipse at 23% 27%,#1a242f,#0d1117 65%)}
.mast{position:absolute;left:37px;right:37px;top:20px;color:#b7c3ce;font-size:12px;
letter-spacing:1.2px;font-weight:800;display:flex;align-items:center;justify-content:space-between}
.mast strong{color:var(--cyan)}.rule{position:absolute;top:53px;height:2px;left:37px;right:37px;background:#303b48}
.module{position:absolute;inset:0;visibility:hidden;display:flex;align-items:center;gap:33px;padding:74px 35px 40px 35px}
.preview{width:375px;height:329px;flex:none;border-radius:23px;border:1px solid #3e4752;
background:radial-gradient(ellipse at 45% 38%,#202e3a,#161b22 80%);box-shadow:0 18px 40px #0008;
display:grid;place-items:center;overflow:hidden}
.details{min-width:0;width:355px}.index{font-size:13px;font-weight:770;color:var(--cyan);
letter-spacing:1.5px;margin:0 0 16px}.name{font-size:33px;line-height:1.12;font-weight:820;color:#f0f6fc;
letter-spacing:-.8px;margin:0 0 18px}.description{font-size:17px;line-height:1.4;color:#b9c8d6;margin:0 0 28px}
.label{color:#8997a6;font-size:12px;letter-spacing:1.5px;text-transform:uppercase;margin:0}
.stylename{color:#ffbf67;font-size:16px;font-weight:720;margin:9px 0 0}
.footer{position:absolute;bottom:17px;left:39px;right:39px;font-size:11px;color:#92a0af;display:flex;justify-content:space-between}
"""
panels=[]
total=len(records)
for i,r in enumerate(records):
    n=f"{i+1:02d}"
    a=100*i/total;b=100*(i+1)/total
    GALLERY+=(f"\n.module:nth-child({i+1}){{animation:show{i+1} 24s linear infinite}}\n"
             f"@keyframes show{i+1}{{0%,{a:.4f}%{{visibility:hidden}}"
             f"{a+.0002:.4f}%,{b-.0002:.4f}%{{visibility:visible}}"
             f"{b:.4f}%,100%{{visibility:hidden}}}}\n")
    panels.append(
        f'<section class="module" data-id="{r["id"]}">'
        f'<div class="preview"><div class="motion-stage p{n}">{MARKUP[n]}</div></div>'
        '<article class="details">'
        f'<p class="index">{n} / {total:02d} · {html.escape(r["category"]).upper()}</p>'
        f'<h1 class="name">{html.escape(r["name"])}</h1>'
        f'<p class="description">{html.escape(r["description"])}</p>'
        f'<p class="label">Estilo</p><p class="stylename">{html.escape(r["style"])}</p>'
        '</article></section>')
catalog='<html lang="es"><head><meta charset="utf-8">'+CSP
catalog+='<style>'+GALLERY+'</style></head><body>'
catalog+='<header class="mast"><span>GPTFRAMES / ANIMACIONES</span><strong>COLECCIÓN 01 · 6 MÓDULOS</strong></header>'
catalog+='<div class="rule"></div><main data-segment-seconds="4">'+''.join(panels)+'</main>'
catalog+='<footer class="footer"><span>HTML + CSS PURO · SIN CDN NI JS</span><span>480p · 24 FPS</span></footer>'
catalog+='</body></html>'
(R/"catalogo.html").write_text(catalog,encoding="utf-8")
print("6 ANIMACIONES:",len(catalog),"bytes of catalog HTML")
