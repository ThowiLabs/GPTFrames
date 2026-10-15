from pathlib import Path
import json,html
root=Path("assets/presets")
root.mkdir(parents=True,exist_ok=True)
items=[
("01","orbit-rings","Orbit Rings","Loader","Anillos concéntricos de rotación inversa.","tech / circular","verificación, procesando"),
("02","pulse-core","Pulse Core","Loader","Núcleo que emite ondas luminosas de actividad.","glow / minimal","actividad, análisis"),
("03","dot-wave","Dot Wave","Loader","Secuencia de siete puntos con movimiento ondulatorio.","minimal / suave","espera, escritura"),
("04","signal-bars","Signal Bars","Loader","Indicadores de intensidad con oscilación irregular.","dashboard / datos","red, señal"),
("05","progress-capsule","Progress Capsule","Loader","Barra encapsulada con avance y brillo móvil.","editorial / progreso","avance, carga"),
("06","radar-sweep","Radar Sweep","Loader","Barrido circular con puntos de detección.","radar / seguridad","búsqueda, escaneo"),
("07","hex-spin","Hex Spin","Loader","Doble hexágono giratorio con centro iluminado.","geométrico / cyber","cálculo, proceso"),
("08","shard-spinner","Shard Spinner","Loader","Fragmentos orbitales con cadencia y reflejos.","dinámico / premium","energía, conexión"),
("09","phone-verify","Phone Verify","Seguridad","Teléfono estilizado con verificación contenida.","security / claro","móvil, autenticar"),
("10","lock-scan","Lock Scan","Seguridad","Candado con arco móvil y lectura luminosa.","seguridad / interfaz","acceso, cierre"),
("11","shield-pulse","Shield Pulse","Seguridad","Escudo con trazos y pulso protector.","trust / tech","protección, escudo"),
("12","alert-beacon","Alert Beacon","Seguridad","Baliza de alerta con ondas controladas.","warning / editorial","riesgo, alerta"),
("13","toast-success","Toast Success","Notificación","Confirmación emergente con icono y progreso.","ui / éxito","completado, bien"),
("14","toast-warning","Toast Warning","Notificación","Advertencia contextual con indicador de atención.","ui / precaución","advertencia, atención"),
("15","metric-card-glow","Metric Card Glow","Interfaz","Métrica con líneas ascendentes y fondo luminoso.","dashboard / métrica","datos, crecimiento"),
("16","section-badge","Section Badge","Interfaz","Etiqueta editorial con barrido discreto.","editorial / label","capítulo, etiqueta"),
("17","info-chip","Info Chip","Interfaz","Ficha informativa compacta con brillo suave.","minimal / contexto","información, apoyo"),
("18","cta-glow-button","CTA Glow Button","Interfaz","Botón de llamada a la acción con borde luminoso.","moderno / CTA","acción, continuar"),
("19","data-stream","Data Stream","Datos","Columnas de partículas simulando transferencia.","data / sci-fi","flujo, transmisión"),
("20","neural-network","Neural Network","Datos","Nodos conectados con señales que recorren la red.","sistema / conexiones","red, inferencia")
]
styles={}
markup={}
styles['01']=r""".p01 .o{position:absolute;inset:43px;border:8px solid #234c64;border-top-color:var(--a);border-right-color:var(--b);border-radius:50%;animation:spin 2.4s linear infinite}.p01 .i{position:absolute;inset:83px;border:7px dashed #f5c477;border-radius:50%;animation:spin 1.8s linear infinite reverse}.p01 .star{position:absolute;inset:118px;background:var(--a);border-radius:16px;box-shadow:0 0 28px #63ffe0aa;animation:breathe 1.7s ease-in-out infinite}"""
markup['01']='<div class="o"></div><div class="i"></div><div class="star"></div>'
styles['02']=r""".p02 .halo{position:absolute;left:50%;top:50%;width:64px;height:64px;margin:-32px;background:#63f5d4;border-radius:50%;box-shadow:0 0 50px #48eacd80}.p02 .wave{position:absolute;left:50%;top:50%;width:66px;height:66px;border:3px solid #61e3ca;border-radius:50%;margin:-33px;animation:expand 2s ease-out infinite}.p02 .wave:nth-child(2){animation-delay:-.67s}.p02 .wave:nth-child(3){animation-delay:-1.34s}"""
markup['02']='<div class="wave"></div><div class="wave"></div><div class="wave"></div><div class="halo"></div>'
styles['03']=r""".p03{display:flex;align-items:center;justify-content:center;gap:13px}.p03 .dot{width:19px;height:19px;background:#65e5d7;border-radius:50%;box-shadow:0 0 18px #70efd0a6;animation:wave 1.35s ease-in-out infinite}.p03 .dot:nth-child(2n){background:#9bb9ff}"""
markup['03']=''.join(f'<i class="dot" style="animation-delay:{i*.11:.2f}s"></i>' for i in range(7))
styles['04']=r""".p04{display:flex;align-items:center;justify-content:center;gap:11px}.p04 .bar{width:18px;height:120px;border-radius:14px;background:linear-gradient(180deg,#86b4ff,#4af2d8);transform-origin:center;animation:signal 1.1s ease-in-out infinite;box-shadow:0 0 17px #5fffd634}"""
markup['04']=''.join(f'<i class="bar" style="animation-delay:-{i*.16:.2f}s"></i>' for i in range(9))
styles['05']=r""".p05 .capsule{position:absolute;left:28px;right:28px;top:137px;height:34px;border:3px solid #86bfffa0;border-radius:28px;background:#133146;overflow:hidden;box-shadow:0 0 35px #5ae6cb3d}.p05 .fill{height:100%;width:48%;background:linear-gradient(90deg,#27afce,#66ffda);border-radius:28px;animation:progress 2.3s ease-in-out infinite alternate}.p05 .mini{position:absolute;top:90px;left:50%;transform:translateX(-50%);font:700 17px system-ui;color:#b5ddf3;letter-spacing:5px}"""
markup['05']='<div class="mini">PROCESSING</div><div class="capsule"><div class="fill"></div></div>'
styles['06']=r""".p06 .radar{position:absolute;inset:32px;border:2px solid #4b9a99;border-radius:50%;background:repeating-radial-gradient(circle,transparent 0 39px,#5cb6b122 40px 41px);overflow:hidden}.p06 .radar:after{content:"";position:absolute;inset:0;background:conic-gradient(from 0deg,#60ffbe88,transparent 95deg,transparent 360deg);border-radius:50%;animation:spin 3s linear infinite}.p06 .radar:before{content:"";position:absolute;inset:0;background:linear-gradient(90deg,transparent 49.7%,#5fb4b133 50%,transparent 50.3%),linear-gradient(0deg,transparent 49.7%,#5fb4b133 50%,transparent 50.3%)}.p06 .blip{position:absolute;background:#a0f6cf;border-radius:50%;width:12px;height:12px;box-shadow:0 0 20px #9bffad;left:115px;top:87px}.p06 .blip.other{left:186px;top:185px;width:8px;height:8px}"""
markup['06']='<div class="radar"><div class="blip"></div><div class="blip other"></div></div>'
styles['07']=r""".p07 .hex{position:absolute;left:60px;top:60px;width:200px;height:200px;background:linear-gradient(140deg,#4ad4ff,#6affc8);clip-path:polygon(25% 3%,75% 3%,100% 50%,75% 97%,25% 97%,0% 50%);animation:spin 4s linear infinite}.p07 .hex:after{content:"";position:absolute;inset:8px;background:#122a41;clip-path:inherit}.p07 .inner{position:absolute;inset:99px;background:#f5ce79;clip-path:polygon(25% 3%,75% 3%,100% 50%,75% 97%,25% 97%,0 50%);animation:spin 2s linear infinite reverse;box-shadow:0 0 25px #ffe49d}"""
markup['07']='<div class="hex"></div><div class="inner"></div>'
styles['08']=r""".p08 .carousel{position:absolute;inset:34px;animation:spin 3.2s linear infinite}.p08 .shard{position:absolute;left:calc(50% - 10px);top:8px;width:20px;height:53px;transform-origin:10px 118px;background:linear-gradient(#61e3ca,#476fb800);clip-path:polygon(50% 0,100% 60%,80% 100%,20% 100%,0 60%);border-radius:10px}.p08 .eye{position:absolute;inset:135px;border-radius:50%;background:#64e9e7;box-shadow:0 0 36px #4afaff}"""
markup['08']='<div class="carousel">'+''.join(f'<div class="shard" style="transform:rotate({i*45}deg);opacity:{1-i*.075:.3f}"></div>' for i in range(8))+'</div><div class="eye"></div>'
styles['09']=r""".p09 .device{position:absolute;left:111px;top:25px;width:98px;height:260px;border:8px solid #a4c5ee;border-radius:25px;background:linear-gradient(140deg,#153651,#102c41);box-shadow:0 15px 45px #58b8dc30}.p09 .device:before{content:"";position:absolute;left:28px;top:10px;width:28px;height:5px;border-radius:5px;background:#6281a3}.p09 .screen{position:absolute;inset:47px 10px 64px;display:grid;place-items:center;border-radius:12px;background:#1c625f60}.p09 .tick{width:56px;height:56px;border:3px solid #67f5bd;border-radius:50%;padding:9px}.p09 .tick path{stroke-dasharray:60;stroke-dashoffset:60;animation:drawcheck 1.8s ease forwards infinite}"""
markup['09']='<div class="device"><div class="screen"><svg class="tick" viewBox="0 0 44 44"><path d="M8 21 L18 31 L37 11" stroke="#78ffcb" stroke-width="5" stroke-linecap="round" stroke-linejoin="round" fill="none"/></svg></div></div>'
styles['10']=r""".p10 .shackle{position:absolute;left:121px;top:44px;width:77px;height:106px;border:13px solid #a5baf5;border-bottom:0;border-radius:42px 42px 0 0;transform-origin:18px 100%;animation:lock 2.4s ease-in-out infinite}.p10 .body{position:absolute;left:81px;top:130px;width:158px;height:121px;border-radius:22px;background:linear-gradient(135deg,#2fd3ad,#207d9b);box-shadow:0 0 35px #4de7cb55;overflow:hidden}.p10 .body:after{content:"";position:absolute;inset:0;background:linear-gradient(transparent,#fff8,transparent);transform:translateY(-130%);animation:scan 2.4s ease-in-out infinite}.p10 .hole{position:absolute;width:18px;height:43px;left:70px;top:39px;background:#0c3546;clip-path:polygon(50% 0,85% 18%,70% 55%,90% 100%,10% 100%,30% 55%,15% 18%)}"""
markup['10']='<div class="shackle"></div><div class="body"><div class="hole"></div></div>'
styles['11']=r""".p11 .pulse{position:absolute;inset:37px;border:2px solid #55e7d0;border-radius:50%;animation:shieldpulse 2s ease-out infinite}.p11 .shield{position:absolute;left:74px;top:37px;width:172px;height:242px;filter:drop-shadow(0 0 15px #4bd6d968)}.p11 .mark{stroke-dasharray:130;stroke-dashoffset:130;animation:drawcheck 2s ease-in-out infinite}"""
markup['11']='<div class="pulse"></div><svg class="shield" viewBox="0 0 200 260"><path d="M100 12L184 47V120Q184 204 100 250Q16 204 16 120V47Z" fill="#163c54" stroke="#5ae0ce" stroke-width="9"/><path class="mark" d="M53 126L86 158L148 93" fill="none" stroke="#b8ffee" stroke-width="12" stroke-linecap="round" stroke-linejoin="round"/></svg>'
styles['12']=r""".p12 .beam{position:absolute;inset:18px;border-radius:50%;border:3px solid #ffb175;animation:expand 2s ease-out infinite}.p12 .warning{position:absolute;left:64px;top:45px;width:192px;height:212px;clip-path:polygon(50% 4%,99% 92%,1% 92%);background:linear-gradient(#ffde88,#ff8f67);filter:drop-shadow(0 0 23px #ffb97a55)}.p12 .warning:before{content:"";position:absolute;inset:20px;clip-path:inherit;background:#50372e}.p12 .bar{position:absolute;left:149px;top:117px;width:22px;height:63px;border-radius:10px;background:#ffd68b}.p12 .dot{position:absolute;left:149px;top:189px;width:22px;height:22px;border-radius:50%;background:#ffd68b}"""
markup['12']='<div class="beam"></div><div class="warning"></div><div class="bar"></div><div class="dot"></div>'
styles['13']=r""".p13{display:grid;place-items:center}.p13 .toast{width:294px;height:116px;border-radius:20px;border:1px solid #4eeab1a0;background:#153b42;box-shadow:0 18px 50px #3bdbad26;display:flex;gap:16px;align-items:center;padding:22px;animation:floatup 2.5s ease-in-out infinite}.p13 .badge{width:48px;height:48px;border-radius:50%;display:grid;place-items:center;background:#56e5aa;color:#0d3d39;font-size:30px;font-weight:900}.p13 strong,.p14 strong{display:block;font-size:19px;color:#eefefb}.p13 small,.p14 small{display:block;margin-top:7px;font-size:12px;color:#b6dbd8}"""
markup['13']='<div class="toast"><div class="badge">✓</div><div><strong>Operación lista</strong><small>Validación completada</small></div></div>'
styles['14']=r""".p14{display:grid;place-items:center}.p14 .toast{width:294px;height:116px;border-radius:20px;border:1px solid #fbb85a88;background:#3e332a;box-shadow:0 18px 50px #ed93362a;display:flex;gap:16px;align-items:center;padding:22px;animation:floatup 2.8s ease-in-out infinite}.p14 .badge{width:48px;height:48px;border-radius:17px;display:grid;place-items:center;background:#ffba66;color:#3a2214;font-size:27px;font-weight:900}"""
markup['14']='<div class="toast"><div class="badge">!</div><div><strong>Revisión pendiente</strong><small>Se requiere atención</small></div></div>'
styles['15']=r""".p15 .metric{position:absolute;left:26px;top:62px;width:268px;height:197px;padding:27px;background:linear-gradient(140deg,#133f52,#12283c);border:1px solid #60ceca83;border-radius:22px;box-shadow:0 12px 55px #5fddbd28}.p15 .k{font-size:15px;color:#b4cedc}.p15 .n{font-size:57px;font-weight:800;color:#75f5cb;margin-top:2px}.p15 .trend{height:35px;display:flex;align-items:end;gap:7px;margin-top:12px}.p15 .trend i{width:19px;height:var(--height);border-radius:4px;background:linear-gradient(#7decda,#257eaf);animation:signal 2s ease-in-out infinite}"""
markup['15']='<div class="metric"><div class="k">Rendimiento global</div><div class="n">84.7%</div><div class="trend">'+''.join(f'<i style="--height:{[15,19,12,25,22,28,33,29][i]}px;animation-delay:-{i*.19:.2f}s"></i>' for i in range(8))+'</div></div>'
styles['16']=r""".p16{display:grid;place-items:center}.p16 .chip{position:relative;border-radius:70px;border:1px solid #5ddec3;padding:19px 30px;overflow:hidden;color:#8cf9d9;font-size:22px;letter-spacing:4px;font-weight:800;box-shadow:0 0 40px #57dfc029;background:#14393f}.p16 .chip:after,.p17 .chip:after{content:"";position:absolute;inset:-40% auto -40% -55%;width:35%;background:linear-gradient(90deg,transparent,#fff8,transparent);transform:skewX(-20deg);animation:shimmer 2.7s linear infinite}"""
markup['16']='<div class="chip">NUEVA SECCIÓN <span aria-hidden="true">↗</span></div>'
styles['17']=r""".p17{display:grid;place-items:center}.p17 .chip{position:relative;display:flex;gap:17px;align-items:center;padding:20px 24px;width:286px;border:1px solid #81b8eb80;border-radius:23px;background:#172f47;overflow:hidden}.p17 .ico{width:43px;height:43px;background:#477ac5;border-radius:50%;display:grid;place-items:center;color:white;font-size:24px;font-weight:700}.p17 .txt{color:#daeafb;font-size:16px;font-weight:650}"""
markup['17']='<div class="chip"><div class="ico">i</div><div class="txt">Información contextual</div></div>'
styles['18']=r""".p18{display:grid;place-items:center}.p18 .btn{position:relative;background:linear-gradient(130deg,#49c9be,#368adb);padding:20px 32px;border-radius:17px;color:white;font-size:23px;letter-spacing:2px;font-weight:800;box-shadow:0 0 35px #52d8d46e;animation:breathe 2.1s ease-in-out infinite}.p18 .btn:after{content:"";position:absolute;inset:-11px;border:2px solid #65e5e3aa;border-radius:24px;animation:shieldpulse 2.1s ease-out infinite}"""
markup['18']='<div class="btn">CONTINUAR →</div>'
styles['19']=r""".p19{display:flex;justify-content:center;align-items:center;gap:10px;overflow:hidden}.p19 .column{height:190px;width:13px;display:flex;flex-direction:column;gap:11px;animation:datashift 1.9s ease-in-out infinite}.p19 .column i{height:22px;width:13px;background:#5cddd2;border-radius:4px;box-shadow:0 0 15px #4affd84b}.p19 .column:nth-child(odd) i:nth-child(odd){background:#638ff6;opacity:.64}"""
markup['19']=''.join(f'<div class="column" style="animation-delay:-{i*.15:.2f}s">'+''.join('<i></i>' for _ in range(6))+'</div>' for i in range(12))
styles['20']=r""".p20 svg{position:absolute;inset:16px;width:288px;height:288px;overflow:visible}.p20 .link{fill:none;stroke:#406b91;stroke-width:3}.p20 .trace{fill:none;stroke:#6ff6d5;stroke-width:4;stroke-dasharray:26 100;animation:dash 3s linear infinite}.p20 .node{fill:#93f4db;stroke:#143f54;stroke-width:8}.p20 .node.hub{fill:#7cafff}.p20 svg{filter:drop-shadow(0 0 13px #4edbd834)}"""
markup['20']='<svg viewBox="0 0 320 320"><path class="link" d="M34 162L92 58L159 164L229 79L284 157L218 251L159 164L101 267L34 162M92 58L229 79M101 267L218 251"/><path class="trace" d="M34 162L92 58L159 164L229 79L284 157L218 251L159 164L101 267L34 162"/>'+''.join(f'<circle class="node{" hub" if i%3==0 else ""}" cx="{x}" cy="{y}" r="{11 if i%3==0 else 8}"/>' for i,(x,y) in enumerate([(34,162),(92,58),(159,164),(229,79),(284,157),(218,251),(101,267)]))+'</svg>'
animation_css=r"""
@keyframes spin{to{transform:rotate(360deg)}}@keyframes breathe{50%{transform:scale(1.12);filter:brightness(1.3)}}
@keyframes expand{0%{transform:scale(.7);opacity:.8}100%{transform:scale(4.5);opacity:0}}
@keyframes wave{0%,100%{transform:translateY(17px);opacity:.5}50%{transform:translateY(-20px);opacity:1}}
@keyframes signal{0%,100%{transform:scaleY(.38)}50%{transform:scaleY(1.02)}}
@keyframes progress{from{width:12%}to{width:96%}}@keyframes drawcheck{45%,100%{stroke-dashoffset:0}}
@keyframes lock{0%,15%{transform:translateY(-16px) rotate(-11deg)}55%,80%,100%{transform:translateY(0) rotate(0)}}
@keyframes scan{0%,100%{transform:translateY(-120%)}70%{transform:translateY(170%)}}@keyframes shieldpulse{0%{opacity:.8;transform:scale(.84)}100%{opacity:0;transform:scale(1.17)}}
@keyframes floatup{0%,100%{transform:translateY(6px)}50%{transform:translateY(-9px)}}@keyframes shimmer{to{left:160%}}
@keyframes datashift{0%,100%{transform:translateY(-28px);opacity:.3}50%{transform:translateY(25px);opacity:1}}
@keyframes dash{to{stroke-dashoffset:-126}}
"""
base_css=r"""*{box-sizing:border-box}html,body{margin:0;width:100%;height:100%;overflow:hidden;font-family:system-ui,Arial,sans-serif}body{background:transparent;color:#effaff}.effect{position:relative;width:320px;height:320px;flex:none;--a:#62e5ce;--b:#8db2ff}.effect i{display:block}"""
csp="""<meta http-equiv="Content-Security-Policy" content="default-src 'none'; script-src 'none'; connect-src 'none'; style-src 'unsafe-inline'; img-src data:; font-src data:">"""
manifest=[]
for i,(num,slug,title,category,description,style,tags) in enumerate(items):
    short=f"p{num}"
    entry={"number":i+1,"id":slug,"name":title,"category":category,"description":description,
           "style":style,"backend":"browser","tags":[s.strip() for s in tags.split(",")],
           "preset_file":f"assets/presets/{slug}.html","status":"candidate",
           "config":{"duration":1.5,"loop":True,"accent":"#62e5ce"}}
    manifest.append(entry)
    standcss=f".show{{display:grid;place-items:center;width:100vw;height:100vh}}\n"+base_css+"\n"+animation_css+"\n"+styles[num]
    markup_elem=f'<div class="effect {short}" aria-label="{html.escape(title,quote=True)}">{markup[num]}</div>'
    file=f'<!doctype html><html><head><meta charset="utf-8">{csp}<style>{standcss}</style></head><body><div class="show">{markup_elem}</div></body></html>'
    (root/f"{slug}.html").write_text(file,encoding="utf-8")
slot=1.5;total=len(items)*slot
gallery_css=base_css+animation_css+"\n"+"\n".join(styles.values())
gallery_css+="""
body{background:radial-gradient(ellipse at 20% 20%,#152f4c,#09182b 62%);font-family:system-ui,Arial,sans-serif}
.mast{position:absolute;left:34px;right:34px;top:22px;display:flex;justify-content:space-between;align-items:center;color:#b4cbda;font-size:12px;font-weight:750;letter-spacing:1.6px}
.mast em{font-style:normal;color:#72f0d6}
.rule{position:absolute;top:54px;left:34px;right:34px;height:2px;background:#35516a}
.demo{position:absolute;inset:0;visibility:hidden;display:flex;gap:32px;align-items:center;padding:75px 44px 39px 40px}
.preview{width:380px;height:332px;border:1px solid #436b7c;border-radius:28px;display:grid;place-items:center;flex:none;background:radial-gradient(circle at 46% 40%,#193f52aa,#0c2537 75%);box-shadow:0 15px 55px #0004;overflow:hidden}
.info{width:348px;min-width:0}
.kicker{font-size:13px;letter-spacing:2px;color:#80efd8;font-weight:800;margin:0 0 12px}
.heading{font-size:31px;line-height:1.07;letter-spacing:-.6px;font-weight:840;margin:0 0 17px;color:#f1f9ff}
.desc{font-size:17px;line-height:1.4;color:#c7d8e3;margin:0 0 26px}
.metadata{font-size:12px;text-transform:uppercase;letter-spacing:1.6px;color:#9badc2}
.style{margin-top:7px;font-size:15px;color:#f2c982;font-weight:650}
.foot{position:absolute;bottom:17px;left:42px;right:39px;color:#8ba8b9;font-size:11px;display:flex;justify-content:space-between}
"""
for i,item in enumerate(manifest):
  n=i+1;start=i/len(manifest)*100;end=(i+1)/len(manifest)*100
  # display using visibility discrete keyframe, at exact frame boundaries
  a=start+0.001;b=end-0.001
  gallery_css+=f'\n.demo:nth-child({n}){{animation:scene{n} {total:.2f}s linear infinite}}\n@keyframes scene{n}{{0%,{start:.3f}%{{visibility:hidden}}{a:.3f}%,{b:.3f}%{{visibility:visible}}{end:.3f}%,100%{{visibility:hidden}}}}\n'
panels=[]
for item in manifest:
  n=item["number"];slug=item["id"];num=f"{n:02d}"
  panels.append(f'<section class="demo" data-id="{slug}"><div class="preview"><div class="effect p{num}">{markup[num]}</div></div><article class="info"><p class="kicker">{n:02d} / {len(items):02d} · {html.escape(item["category"]).upper()}</p><h1 class="heading">{html.escape(item["name"])}</h1><p class="desc">{html.escape(item["description"])}</p><div class="metadata">Estilo</div><div class="style">{html.escape(item["style"])}</div></article></section>')
page=f'<!doctype html><html lang="es"><head><meta charset="utf-8">{csp}<title>GPTFrames - Presets de movimiento</title><style>{gallery_css}</style></head><body><div class="mast"><span>GPTFRAMES / MOTION ASSET STUDIO</span><em>20 PRESETS ORIGINALES</em></div><div class="rule"></div><main>{"".join(panels)}</main><div class="foot"><span>CATÁLOGO VISUAL · MOTOR CSS / CDP</span><span>480p · 24 FPS</span></div></body></html>'
Path("examples/browser-first-uiverse/catalogue.html").write_text(page,encoding="utf-8")
Path("assets/preset-catalog.json").write_text(json.dumps({"version":1,"source":"Original GPTFrames in-house designs","license":"Project license","count":len(manifest),"presets":manifest},ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print(len(manifest),"presets",len(page),"html bytes","duration",total)
