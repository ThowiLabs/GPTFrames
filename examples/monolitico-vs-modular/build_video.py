#!/usr/bin/env python3
"""Produce a deterministic 56s 480p educational video on software architecture."""
from pathlib import Path
import html,json
R=Path(__file__).resolve().parent
DURATION=8
def code_block(lines):
    return '<div class="code-window"><div class="code-bar"><span class="traffic"><i></i><i></i><i></i></span><span>Python 3.13 · código ilustrativo</span></div><div class="code-body">'+''.join(
      f'<div class="code-line" style="--delay:{.7+i*.45:.2f}s"><span class="line-num">{i+1:02d}</span><code>{line}</code></div>' for i,line in enumerate(lines))+'</div></div>'
def tag(text,cl=""):
    return f'<span class="tag {cl}">{html.escape(text)}</span>'
def info_card(title,desc,color,idx):
    return f'<div class="info-card {color}" style="--delay:{.9+idx*.65:.2f}s"><div class="info-head">{title}</div><div class="info-sub">{desc}</div></div>'
scenes=[]
def scene(n,eyebrow,title,deck,main,small=""):
    scenes.append(f'<section class="module scene-{n}">'
      f'<div class="overline"><span>{n+1:02d} / 07</span><span>{html.escape(eyebrow.upper())}</span></div>'
      f'<h1>{title}</h1><p class="deck">{deck}</p>'
      f'<div class="scene-content">{main}</div>'
      f'<p class="scene-takeaway">{small}</p></section>')
scene(0,"Concepto fundamental","MONOLÍTICO <em>VS</em> MODULAR",
      "Dos decisiones distintas: cómo se despliega un sistema y cómo organizamos su código.",
      '<div class="compare-hero"><div class="hero-block mono"><span class="hero-label">MONOLÍTICO</span>'
      '<div class="tower"><span>API</span><span>LÓGICA</span><span>DATOS</span><span>AVISOS</span></div>'
      '<small>Una aplicación desplegada junta</small></div>'
      '<div class="versus">VS</div>'
      '<div class="hero-block modules"><span class="hero-label">MODULAR</span>'
      '<div class="module-tiles"><div>pagos.py</div><div>inventario.py</div><div>avisos.py</div></div>'
      '<small>Código separado por responsabilidades</small></div></div>',
      "Un monolito también puede ser modular.")
scene(1,"Ejemplo en Python","UNA FUNCIÓN HACE DE TODO",
      "Ejemplo de un monolito con código acoplado: tres tareas mezcladas en un solo lugar.",
      '<div class="two-col"><div class="code-wide">'+code_block([
        '<b>def</b> <span class="fun">procesar_pedido</span>(pedido):',
        '    <span class="comment"># 1. Cobrar</span>',
        '    banco.<span class="fun">cobrar</span>(pedido.total)',
        '    <span class="comment"># 2. Actualizar inventario</span>',
        '    stock[pedido.id] -= 1',
        '    <span class="comment"># 3. Avisar al cliente</span>',
        '    correo.<span class="fun">enviar</span>(pedido.email)'
      ])+'</div><div class="side-cards">'+info_card("PAGOS","Reglas financieras","orange",0)+info_card("INVENTARIO","Existencias","cyan",1)+info_card("AVISOS","Comunicación","purple",2)+'</div></div>',
      "Funciona, pero mezclar responsabilidades complica el mantenimiento.")
scene(2,"Consecuencias","CAMBIAS UNA PARTE…",
      "En código fuertemente acoplado, modificar un bloque puede afectar funcionalidades que no querías tocar.",
      '<div class="impact-layout"><div class="impact-source"><div class="impact-title">procesar_pedido()</div>'
      '<div class="impact-change">+ validar cupón</div><div class="impact-risk">Cambio en cobro</div></div>'
      '<div class="impact-rays"><span>→</span><span>→</span><span>→</span></div>'
      '<div class="impact-targets">'+info_card("PRUEBAS","Más dependencias","orange",0)+info_card("INVENTARIO","Revisar efectos","cyan",1)+info_card("AVISOS","Riesgo de regresión","purple",2)+'</div></div>',
      "Problema del acoplamiento, no de que el sistema se despliegue junto.")
scene(3,"Separación de responsabilidades","CADA MÓDULO, UNA TAREA",
      "La misma aplicación puede tener archivos con responsabilidades bien definidas.",
      '<div class="file-grid">'
      '<div class="file-card amber"><div class="file-icon">PY</div><h2>pagos.py</h2><pre><b>def</b> <span>cobrar</span>(pedido):\n    return banco.cobrar(\n        pedido.total)</pre></div>'
      '<div class="file-card cyan"><div class="file-icon">PY</div><h2>inventario.py</h2><pre><b>def</b> <span>reservar</span>(pedido):\n    return stock.restar(\n        pedido.id)</pre></div>'
      '<div class="file-card purple"><div class="file-icon">PY</div><h2>avisos.py</h2><pre><b>def</b> <span>notificar</span>(pedido):\n    return correo.enviar(\n        pedido.email)</pre></div></div>',
      "El criterio es la responsabilidad, no simplemente crear más archivos.")
scene(4,"Composición del sistema","main.py LOS COORDINA",
      "El flujo principal usa funciones claras, que pueden probarse y reutilizarse por separado.",
      '<div class="two-col main-assembly"><div class="code-wide">'+code_block([
        '<b>from</b> pagos <b>import</b> cobrar',
        '<b>from</b> inventario <b>import</b> reservar',
        '<b>from</b> avisos <b>import</b> notificar',
        '',
        '<b>def</b> <span class="fun">procesar_pedido</span>(pedido):',
        '    <span class="fun">cobrar</span>(pedido)',
        '    <span class="fun">reservar</span>(pedido)',
        '    <span class="fun">notificar</span>(pedido)'
      ])+'</div><div class="chain"><div class="chain-node">main.py</div><span>↓</span><div>pagos.py</div><span>↓</span><div>inventario.py</div><span>↓</span><div>avisos.py</div></div></div>',
      "Separar permite pruebas unitarias por responsabilidad.")
scene(5,"Diferencia fundamental","NO SON OPUESTOS",
      "Un monolito puede ser modular: varios módulos, un proceso y un despliegue.",
      '<div class="single-runtime"><div class="runtime-header"><span class="status-dot"></span> UNA APLICACIÓN · UN DESPLIEGUE</div>'
      '<div class="runtime-modules"><div><strong>pagos</strong><small>módulo</small></div><div><strong>inventario</strong><small>módulo</small></div><div><strong>avisos</strong><small>módulo</small></div></div>'
      '<div class="runtime-tail">Sin red entre módulos · Sin servicios separados</div></div>',
      "Microservicios sí requieren separar ejecución y comunicación entre servicios.")
scene(6,"Conclusión práctica","SEPARA RESPONSABILIDADES",
      "No modularices por moda. Cuando crezca la complejidad, diseña piezas claras y fáciles de probar.",
      '<div class="closing"><div class="tree-view"><div class="tree-head">tienda/</div><div>├── main.py</div>'
      '<div class="tree-file">├── pagos.py</div><div class="tree-file">├── inventario.py</div>'
      '<div class="tree-file">├── avisos.py</div><div>└── tests/</div></div>'
      '<div class="closing-points">'+info_card("CLARIDAD","Una tarea por módulo","cyan",0)+info_card("PRUEBAS","Dependencias más simples","purple",1)+info_card("EVOLUCIÓN","Cambios localizados","orange",2)+'</div></div>',
      "Arquitectura = límites claros + bajo acoplamiento.")
assert len(scenes)==7
css=r"""
:root{--bg:#060b16;--panel:#0d1c2f;--edge:#2b425d;--text:#ecf7ff;--cyan:#49d8f4;--amber:#ffc371;--purple:#b49bff}
*{box-sizing:border-box}html,body{margin:0;width:100%;height:100%;overflow:hidden}
body{color:var(--text);font-family:system-ui,-apple-system,"Segoe UI",sans-serif;background:radial-gradient(ellipse at 28% 26%,#152b47 0%,#081325 57%,#050914 100%)}
.page-header{position:absolute;top:16px;left:32px;right:32px;display:flex;align-items:center;justify-content:space-between;color:#91acbf;font-size:10px;font-weight:800;letter-spacing:1.3px}
.page-header strong{color:#6de6d0}
.progress{position:absolute;top:46px;left:32px;right:32px;height:3px;background:#23384f;border-radius:6px;overflow:hidden}
.progress:before{content:"";display:block;background:linear-gradient(90deg,#52f0c6,#40b6ef);height:100%;width:0;animation:progressBar 56s linear both}
@keyframes progressBar{to{width:100%}}
.module{position:absolute;inset:0;visibility:hidden;padding:67px 35px 42px}
.overline{display:flex;justify-content:space-between;align-items:center;font-size:11px;color:#58dfca;font-weight:800;letter-spacing:1.15px}
.module h1{font-size:32px;letter-spacing:-.8px;line-height:1.1;margin:14px 0 7px;font-weight:840;color:#f4fbff}
.module h1 em{font-style:normal;color:var(--cyan)}
.deck{color:#b9ccde;font-size:13.5px;margin:0;line-height:1.38;max-width:776px}
.scene-content{margin-top:19px;height:251px;display:flex;align-items:stretch;justify-content:center}
.scene-takeaway{color:#a9dacd;font-size:11.7px;font-weight:550;position:absolute;bottom:24px;left:36px;margin:0}
.footer{position:absolute;bottom:9px;right:34px;color:#5a7892;font-size:9px;letter-spacing:1px}
.compare-hero{display:flex;align-items:center;justify-content:space-between;gap:27px;width:100%}
.hero-block{border:1px solid #34516d;background:#0d1c2b;border-radius:16px;flex:1;height:100%;padding:14px 19px 10px;box-shadow:0 10px 28px #0006;text-align:center;opacity:0;animation:arrive .7s ease-out .5s forwards}
.hero-block.modules{animation-delay:1.45s}
.hero-label{font-size:14px;color:#ebf7ff;font-weight:800;letter-spacing:1px}
.hero-block small{font-size:10px;color:#9fb7cc}
.tower{border:2px solid #ffc371;background:#382719;border-radius:11px;width:160px;margin:13px auto 11px;display:flex;flex-direction:column;padding:6px}
.tower span{border-bottom:1px solid #765a3e;padding:7px;color:#ffdcab;font-size:12px;font-weight:800}.tower span:last-child{border:0}
.module-tiles{display:flex;align-items:center;flex-direction:column;gap:9px;margin:21px 0 17px}
.module-tiles div{background:#0b3b43;border:1px solid #3ccad3;border-radius:8px;padding:10px 10px;width:180px;color:#96f1f3;font-size:13px;font-weight:800}
.versus{color:#89a5bd;font-size:20px;font-weight:800}
@keyframes arrive{from{opacity:0;transform:translateY(20px) scale(.97)}to{opacity:1;transform:translateY(0) scale(1)}}
.two-col{display:flex;gap:17px;width:100%;height:100%}
.code-wide{width:69%;min-width:0;height:100%}
.code-window{height:100%;background:#070f1e;border:1px solid #38516a;border-radius:14px;overflow:hidden;box-shadow:0 12px 26px #0005}
.code-bar{height:37px;background:#101d2e;display:flex;align-items:center;justify-content:space-between;border-bottom:1px solid #233a54;padding:0 13px;color:#789eb5;font-size:10.8px;font-weight:700}
.traffic{display:flex;gap:5px}.traffic i{display:block;width:8px;height:8px;background:#f87171;border-radius:50%}.traffic i:nth-child(2){background:#fbbf24}.traffic i:nth-child(3){background:#34d399}
.code-body{padding:12px 10px 12px 5px;font-size:12.2px;font-family:ui-monospace,SFMono-Regular,Consolas,monospace}
.code-line{opacity:0;transform:translateX(-10px);animation:showLine .35s ease-out var(--delay,.5s) forwards;white-space:pre;min-height:25px;display:flex;align-items:center}
@keyframes showLine{to{opacity:1;transform:translateX(0)}}
.line-num{color:#436079;text-align:right;width:35px;margin-right:14px;flex-shrink:0}
.code-line code{color:#e7f3fc}.code-line b{color:#b9a3ff;font-weight:750}.code-line .fun{color:#61e4cb}.code-line .comment{color:#7293ab}
.side-cards,.impact-targets,.closing-points{flex:1;display:flex;flex-direction:column;gap:11px}
.info-card{background:#0c2035;border:1px solid #284664;border-left:4px solid var(--cyan);border-radius:11px;padding:11px 13px;opacity:0;animation:arrive .55s ease-out var(--delay,.8s) forwards;flex:1}
.info-card.orange{border-left-color:var(--amber)}.info-card.purple{border-left-color:var(--purple)}
.info-head{font-size:13px;letter-spacing:.3px;font-weight:800;color:#e9f6fe}
.info-sub{font-size:10.6px;color:#adc0d0;margin-top:3px;line-height:1.25}
.impact-layout{width:100%;display:flex;align-items:center;gap:12px}
.impact-source{width:37%;height:203px;border:1px solid #996b40;background:#31251e;border-radius:16px;display:flex;justify-content:center;flex-direction:column;gap:15px;padding:18px}
.impact-title{color:#fbd298;font-family:monospace;font-size:17px;font-weight:800}
.impact-change{color:#ffae6a;background:#57351f;border-radius:7px;padding:11px;font-family:monospace;font-size:14px;animation:pulse 1.9s ease-in-out infinite}
.impact-risk{color:#bca08d;font-size:11px}
@keyframes pulse{50%{box-shadow:0 0 0 7px #ffc37113}}
.impact-rays{display:flex;flex-direction:column;align-items:center;gap:18px;color:#f7a872;font-size:30px}
.impact-targets{width:43%;max-height:218px}
.file-grid{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));width:100%;gap:13px}
.file-card{position:relative;border:1px solid #37546b;background:linear-gradient(180deg,#11243c,#0b1527);border-radius:14px;padding:16px;opacity:0;animation:arrive .75s ease-out forwards}
.file-card:nth-child(1){animation-delay:.75s}.file-card:nth-child(2){animation-delay:1.65s}.file-card:nth-child(3){animation-delay:2.55s}
.file-card .file-icon{display:grid;place-items:center;width:32px;height:32px;background:#143749;border:1px solid #3a9bc0;color:#a8edff;font-weight:850;font-size:12px;border-radius:8px}
.file-card h2{font-size:16px;color:#eaf7ff;letter-spacing:-.3px;margin:13px 0}
.file-card pre{font-family:monospace;white-space:pre;font-size:10.2px;color:#adc5d8;line-height:1.68}
.file-card pre b{color:#b5a4ff}.file-card pre span{color:#62e4ca}
.file-card.amber{border-top:4px solid var(--amber)}.file-card.cyan{border-top:4px solid var(--cyan)}.file-card.purple{border-top:4px solid var(--purple)}
.main-assembly .code-wide{width:75%}
.chain{flex:1;display:flex;flex-direction:column;align-items:stretch;text-align:center;justify-content:center;gap:3px}
.chain div{font-family:monospace;font-size:13px;font-weight:800;color:#d9f9fc;background:#15344a;padding:9px 6px;border:1px solid #327b91;border-radius:7px;opacity:0;animation:arrive .3s ease-out forwards}
.chain div:nth-of-type(1){animation-delay:1.3s}.chain div:nth-of-type(2){animation-delay:2.2s}.chain div:nth-of-type(3){animation-delay:3.2s}.chain div:nth-of-type(4){animation-delay:4.2s}
.chain span{color:#62d8cd;line-height:.9;opacity:0;animation:arrive .3s ease-out 2.2s forwards}
.single-runtime{position:relative;width:760px;border:2px solid #57ccb2;border-radius:18px;background:linear-gradient(180deg,#0d2631,#0c192a);height:226px;box-shadow:0 0 0 8px #50d2ba0e;padding:20px;opacity:0;animation:arrive .65s ease-out .65s forwards}
.runtime-header{font-size:15px;letter-spacing:.8px;color:#a3f8dc;text-align:center;font-weight:800}
.status-dot{display:inline-block;width:8px;height:8px;border-radius:50%;background:#50efba;margin-right:9px}
.runtime-modules{display:flex;gap:16px;margin-top:24px}
.runtime-modules div{flex:1;display:flex;flex-direction:column;align-items:center;gap:11px;border:1px solid #49738c;background:#143146;border-radius:10px;padding:18px 9px;color:#dbf7ff;opacity:0;animation:arrive .45s ease-out forwards}
.runtime-modules div:nth-child(1){animation-delay:1.3s}.runtime-modules div:nth-child(2){animation-delay:2.15s}.runtime-modules div:nth-child(3){animation-delay:3s}
.runtime-modules strong{font-size:16px}.runtime-modules small{color:#92bdd0}
.runtime-tail{text-align:center;color:#8aafbb;font-size:11px;margin-top:12px}
.closing{display:flex;width:100%;gap:26px}
.tree-view{background:#071522;border:1px solid #3c5971;border-radius:14px;flex:1;padding:15px 30px;font-family:monospace;font-size:17px;line-height:1.86;color:#d5e8f5;opacity:0;animation:arrive .75s ease-out .6s forwards}
.tree-head{font-weight:900;color:#9bf3c6}.tree-file{color:#8fe7ff}
.closing-points{width:45%}
"""
for i in range(7):
 st=i/7*100;ed=(i+1)/7*100
 css+=f"\n.module:nth-child({i+1}){{animation:showScene{i+1} 56s linear infinite}}\n@keyframes showScene{i+1}{{0%,{st:.5f}%{{visibility:hidden}}{st+.0001:.5f}%,{ed-.0001:.5f}%{{visibility:visible}}{ed:.5f}%,100%{{visibility:hidden}}}}\n"
CSP='<meta http-equiv="Content-Security-Policy" content="default-src &#39;none&#39;; script-src &#39;none&#39;; connect-src &#39;none&#39;; style-src &#39;unsafe-inline&#39;; img-src data:; font-src data:">'
page='<!doctype html><html lang="es"><head><meta charset="utf-8">'+CSP+'<style>'+css+'</style></head><body>'
page+='<header class="page-header"><span>DOSSIER DIGITAL / GPTFRAMES STUDIO</span><strong>PROGRAMACIÓN · ARQUITECTURA</strong></header><div class="progress"></div>'
page+='<main data-segment-seconds="8">'+''.join(scenes)+'</main>'
page+='<footer class="footer">2026 · DEMOSTRACIÓN DIDÁCTICA</footer></body></html>'
(R/"monolitico-modular.html").write_text(page)
print("HTML_GENERATED",len(page),"SCENES",len(scenes),"FRAMES",56*24)
