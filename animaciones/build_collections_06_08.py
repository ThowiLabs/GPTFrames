#!/usr/bin/env python3
"""Build 30 offline deterministic HTML/SVG presets from reviewed, local snapshot sources.

No original JS is executed in generated presets. The source helper has already
materialized the distinct visual states from trusted module declarations.
"""
import re,json,html,unicodedata
from pathlib import Path

R=Path(__file__).resolve().parent
DOC=R/"catalogo.json"
sources=json.loads((R/"snapshot_sources_06_08.json").read_text())
assert len(sources)==30
all_presets=json.loads(DOC.read_text())
current=[v for v in all_presets["presets"] if v.get("batch") not in {"architecture-06","runtime-07","languages-08"}]
assert len(current)==65,(len(current),all_presets["total"])
KIND={
 "architecture":("Arquitectura de software","arquitectura","architecture-06"),
 "runtime":("Programación y ejecución","depuración","runtime-07"),
 "languages":("Lenguajes de programación","lenguaje","languages-08"),
}
DURATION=4
CSP="<meta http-equiv=\"Content-Security-Policy\" content=\"default-src &#39;none&#39;; script-src &#39;none&#39;; connect-src &#39;none&#39;; style-src &#39;unsafe-inline&#39;; img-src data:; font-src data:;\">"
STYLE=r"""
:root{--bg:#080e18;--card:#121d2d;--panel:#0e1827;--surface:#1b2939;--muted:#8fa9bc;--edge:#294159;--line:#344c66;--cyan:#5bdcf5;--green:#55dfae;--amber:#ffcb81;--purple:#b9a3ff;
--border-color:#30363d;--border-bright:#484f58;--text-main:#e6edf3;--text-muted:#8b949e;
--accent-cyan:#00e3ff;--accent-green:#42dea9;--accent-amber:#ffbd6b;--accent-violet:#a855f7;--accent-red:#f85149;
--accent-blue:#58a6ff;--accent-yellow:#d29922;--success:#3fb950;--warning:#d29922;--danger:#f85149;
--bg-surface:#151e2d;--bg-card:#21262d;--bg-panel:#0e1827;--bg-overlay:#1f242c}
*{box-sizing:border-box}
html,body{width:100%;height:100%;margin:0;overflow:hidden}
body{background:radial-gradient(ellipse at 20% 14%,#18293f,#080e18 67%,#050a12);color:#eaf5fc;font-family:system-ui,Arial,sans-serif}
.collection-header{position:absolute;inset:15px 26px auto;display:flex;justify-content:space-between;align-items:center;color:#8baec1;font-size:10px;letter-spacing:1px;font-weight:800}
.collection-header strong{color:#65e8c6}
.progress-line{position:absolute;top:42px;left:26px;right:26px;background:#294159;height:2px}
.catalog{position:absolute;inset:0;overflow:hidden}
.module{position:absolute;inset:0;padding:55px 26px 27px;visibility:hidden;display:flex;flex-direction:column;gap:8px}
.module-header{display:flex;align-items:baseline;gap:12px;min-height:29px}
.index{color:#6de6c8;font:800 12px ui-monospace,Consolas,monospace;white-space:nowrap}
.headline{font-size:24px;letter-spacing:-.5px;color:#edf7ff;font-weight:840;margin:0;line-height:1.2}
.kind{margin-left:auto;font:750 9px ui-monospace,Consolas,monospace;letter-spacing:.4px;color:#8facc2;border:1px solid #314d65;border-radius:13px;padding:5px 10px;white-space:nowrap}
.description{margin:0;color:#aec4d4;font-size:12px;line-height:1.36;height:32px;max-width:785px;overflow:hidden}
.main-layout{display:flex;gap:13px;min-height:0;flex:1}
.visual-card{position:relative;background:#0b1524;border:1px solid #36516b;border-radius:14px;overflow:hidden;flex:1;min-width:0}
.visual-card .pane-title,.source-card .pane-title{height:30px;border-bottom:1px solid #2d4157;display:flex;align-items:center;justify-content:space-between;padding:0 12px;color:#83abc4;font:750 10px ui-monospace,Consolas,monospace;letter-spacing:.45px;background:#111e30}
.visual-card .pane-title span:last-child{color:#61e1ba}
.source-card .pane-title span:last-child{max-width:124px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;text-align:right}
.visual-stage{position:absolute;top:30px;bottom:0;left:0;right:0}
.phase{position:absolute;inset:0;opacity:0;visibility:hidden;display:flex;align-items:center;justify-content:center}
.diagram-svg,.diagram-svg-root{display:block;width:100%;height:100%;overflow:visible}
.source-card{position:relative;width:260px;flex:0 0 260px;min-width:0;background:#0b1524;border:1px solid #36516b;border-radius:14px;overflow:hidden}
.code-scroll{position:absolute;inset:30px 0 0;overflow:hidden;padding:7px 7px 5px 0;background:#080f1e}
.code-line{display:flex;gap:4px;align-items:baseline;font:10px/1.53 ui-monospace,Consolas,monospace;min-height:15.3px;white-space:pre}
.code-line .ln{flex:0 0 23px;text-align:right;color:#42637b}
.code-line .ct{min-width:0;display:block;color:#e0eaf8}
.code-line .ct span{white-space:pre}
.kw,.tok-kw,.c-kw{color:#c3aaff;font-weight:700}
.fn,.tok-fn,.c-fn{color:#77e3d1}.tp{color:#83c8ff}
.st,.tok-str,.c-str{color:#b4d6ff}.num,.tok-num,.c-num{color:#ffc879}.cm,.tok-cmt,.c-cm{color:#8298a8}
.vr,.tok-op,.c-hl{color:#e5b9fb}
.code-scroll .code-line{padding-right:3px}
.code-scroll .code-line:nth-child(n+17){opacity:.55}
.code-more{position:absolute;bottom:0;left:0;right:0;background:linear-gradient(transparent,#0b1524 90%);height:22px;pointer-events:none}
.execution-bar{height:30px;flex:0 0 30px;background:#102238;border:1px solid #274c68;border-radius:8px;padding:7px 10px;color:#a8e3e5;display:flex;justify-content:space-between;align-items:center;gap:13px}
.log{font:10.2px ui-monospace,Consolas,monospace;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.execution-tag{font:800 9px ui-monospace,Consolas,monospace;color:#8ddbbf;white-space:nowrap}
footer{position:absolute;bottom:9px;right:26px;color:#57768d;font:9px ui-monospace,Consolas,monospace}
/* architecture SVG vocabulary */
.node-box{fill:#132236;stroke:#334e67;stroke-width:1.5}
.node-box.active,.node-box.highlight{fill:#1e344a;stroke:#58a6ff}
.node-box.success{fill:#15372d;stroke:#44e2a9}
.node-box.warning{fill:#3a2d1c;stroke:#eab977}
.node-box.error{fill:#401a26;stroke:#ff6675}
.svg-label{fill:#dceef8;font:600 12px ui-monospace,Consolas,monospace}
.svg-sub{fill:#95acc1;font:10px ui-monospace,Consolas,monospace}
.packet{fill:#5ce6fa;filter:drop-shadow(0 0 5px #35dafb)}
/* runtime SVG vocabulary */
.node-text{fill:#e2edf9;font:11px ui-monospace,Consolas,monospace}
.node-subtext{fill:#9fb0c2;font:9px ui-monospace,Consolas,monospace}
.conn-line{stroke:#526b84;stroke-width:1.7;fill:none}
.conn-line.active,.svg-path.flowing{stroke:#5ad8fe;stroke-dasharray:6 4;animation:motionDash 1s linear infinite}
.conn-line.error{stroke:#fa6868}
@keyframes motionDash{to{stroke-dashoffset:-20}}
/* languages SVG vocabulary */
.svg-box{fill:#1a2739;stroke:#39516a;stroke-width:1.6}
.svg-box.active{stroke:#58a6ff;fill:#1d354c}
.svg-box.success{stroke:#36d6a7;fill:#14382f}
.svg-box.warning{stroke:#facc80;fill:#3a291b}
.svg-text{fill:#e8f3fb;font:700 11px ui-monospace,Consolas,monospace;text-anchor:middle;dominant-baseline:middle}
.svg-subtext{fill:#a2b5c8;font:9px ui-monospace,Consolas,monospace;text-anchor:middle;dominant-baseline:middle}
.svg-path{fill:none;stroke:#51708c;stroke-width:1.7}
"""
def slug(s):
 norm=unicodedata.normalize("NFKD",str(s)).encode("ascii","ignore").decode().lower()
 return re.sub(r"-+","-",re.sub("[^a-z0-9]+","-",norm)).strip("-")
def property_tag(content, config, svg=False):
 """Tag exact identifier tokens in visible text, never attributes or markup."""
 if not config or "<" not in content:return content
 mapped={str(k).lower():str(v) for k,v in config.items() if 2<len(str(v))<=48}
 if not mapped:return content
 vals=sorted(mapped.items(),key=lambda x:len(x[1]),reverse=True)
 pat=re.compile("|".join(r"(?<![A-Za-z0-9_])"+re.escape(v)+r"(?![A-Za-z0-9_])" for _,v in vals))
 reverse={v:k for k,v in vals}
 def replace_text(fragment):
  return pat.sub(lambda m:"<"+("tspan" if svg else "span")+" data-field=\""+reverse[m.group()]+"\">"+m.group()+"</"+("tspan" if svg else "span")+">",fragment)
 return re.sub(r"(?<=>)[^<>]+(?=<)",lambda m:replace_text(m.group()),content)
def clean_svg(value):
 if re.search(r"<\s*(script|iframe|foreignobject|image|use)\b|\bon[a-z]+\s*=|url\(\s*https?://",value,re.I):
  raise ValueError("Unsafe SVG content encountered")
 return value
def generic_code(lines,batch,config):
 out=[]
 for i,line in enumerate(lines[:30]):
  if batch=="runtime":
   fragment=html.escape(line,quote=False)
   fragment=re.sub(r"\b(const|let|return|function|class|if|else|int|for|while|def|throw|try|catch|async|await|import|export|new|public|void)\b",r'<span class="kw">\1</span>',fragment)
  else:fragment=line
  fragment=property_tag(fragment,config)
  out.append(f'<div class="code-line"><span class="ln">{i+1:02d}</span><span class="ct">{fragment or "&#160;"}</span></div>')
 return "".join(out)
def phase_visibility(idx,count,name):
 start=idx/count*100;end=(idx+1)/count*100
 if idx==0:return f"@keyframes {name}{{0%,{end-0.1:.4f}%{{opacity:1;visibility:visible}}{end:.4f}%,100%{{opacity:0;visibility:hidden}}}}"
 return (f"@keyframes {name}{{0%,{start-.1:.4f}%{{opacity:0;visibility:hidden}}"
         f"{start:.4f}%,{end-.1:.4f}%{{opacity:1;visibility:visible}}{end:.4f}%,100%{{opacity:0;visibility:hidden}}}}")
new=[]
def render_content(item,serial,within):
 category,type_,batch=KIND[item["batch"]]
 name=html.escape(item["name"])
 desc=html.escape(item["description"])
 filename=html.escape(item["file"])
 config=item["configuration"]
 slug_id=("architecture-" if item["batch"]=="architecture" else "runtime-" if item["batch"]=="runtime" else "language-")+slug(item["name"])
 if item["batch"]=="languages" and item["id"] in {"cpp","cs"}:
  slug_id="language-"+item["id"]
 stages=[];rules=[]
 for i,snap in enumerate(item["snapshots"]):
  svg=clean_svg(snap["svg"])
  svg=property_tag(svg,config,svg=True)
  if not re.match(r"\s*<svg\b",svg,re.I):
   viewbox="0 0 800 440" if item["batch"]=="architecture" else "0 0 800 480"
   svg=('<svg class="diagram-svg" viewBox="'+viewbox+'" preserveAspectRatio="xMidYMid meet"><defs>'+
    '<marker id="arrow" viewBox="0 0 10 10" refX="7" refY="5" markerWidth="6" markerHeight="6" orient="auto"><path d="M0 1 L8 5 L0 9Z" fill="#7894a9"/></marker>'+
    '<marker id="arrow-active" viewBox="0 0 10 10" refX="7" refY="5" markerWidth="6" markerHeight="6" orient="auto"><path d="M0 1 L8 5 L0 9Z" fill="#57d9f3"/></marker>'+
    '<marker id="arrow-error" viewBox="0 0 10 10" refX="7" refY="5" markerWidth="6" markerHeight="6" orient="auto"><path d="M0 1 L8 5 L0 9Z" fill="#fa6878"/></marker>'+
    '</defs>'+svg+'</svg>')
  else:
   svg=svg.replace('<svg ', '<svg class="diagram-svg-root" ',1)
  title=html.escape(str(snap.get("log",""))[:260])
  nph=f"phase_{serial}_{i}"
  rules.append(phase_visibility(i,len(item["snapshots"]),nph))
  rules.append(f".phase-{serial}-{i}{{animation:{nph} {DURATION}s linear both}}")
  stages.append(f'<div class="phase phase-{serial}-{i}">{svg}<span class="sr-only" style="display:none">{title}</span></div>')
 code_html=generic_code(item["codeLines"],item["batch"],config)
 stages_html="".join(stages)
 # Captured log from final phase is displayed only after phases (not all logs at start).
 logs=[]
 for i,snap in enumerate(item["snapshots"]):
  nph=f"log_{serial}_{i}"; rules.append(phase_visibility(i,len(item["snapshots"]),nph))
  rules.append(f".log-{serial}-{i}{{animation:{nph} {DURATION}s linear both}}")
  logs.append(f'<span class="log phase log-{serial}-{i}">{html.escape(str(snap.get("log",""))[:220])}</span>')
 block=(f'<section class="module" data-id="{slug_id}"><div class="module-header">'
  f'<span class="index">{within:02d}/10 · {serial:02d}/30</span>'
  f'<h1 class="headline" data-field="title">{name}</h1>'
  f'<span class="kind">{html.escape(category.upper())}</span></div>'
  f'<p class="description" data-field="description">{desc}</p>'
  f'<div class="main-layout"><div class="visual-card"><div class="pane-title"><span>VISUALIZACIÓN DE EJECUCIÓN</span><span>ETAPAS {len(stages):02d}</span></div>'
  f'<div class="visual-stage">{stages_html}</div></div>'
  f'<div class="source-card"><div class="pane-title"><span>CÓDIGO / INSPECTOR</span><span data-field="file">{filename}</span></div>'
  f'<div class="code-scroll">{code_html}</div><div class="code-more"></div></div></div>'
  f'<div class="execution-bar"><div class="log-stage" style="position:relative;height:100%;flex:1;overflow:hidden">'+''.join(logs)+'</div>'
  f'<span class="execution-tag">DEMO · {DURATION}s</span></div></section>')
 # fix log overlays: position absolute relative to log-stage; .phase flex fits stage
 extra=".log-stage .log{position:absolute;inset:0;display:block;line-height:17px;}"
 metadata={
  "number":len(current)+len(new)+1,"id":slug_id,"name":item["name"],
  "category":category,"type":type_,"description":item["description"],
  "style":"Dark technical / SVG / código editable",
  "backend":"browser","collection":"animaciones",
  "preset_file":f"animaciones/presets/{slug_id}.html","status":"candidate",
  "duration":DURATION,"batch":batch,
  "motion":["revelado por fases","resaltado de estado","flujo de datos","animación SVG"],
  "editable":{"title":{"type":"text","label":"Nombre visible"},
    "description":{"type":"text","label":"Descripción"},
    "file":{"type":"text","label":"Archivo de ejemplo"},
    **{k.lower():{"type":"text","label":k,"default":str(v)} for k,v in config.items() if 2<len(str(v))<=48}},
  "tags":list(dict.fromkeys([slug_id.split("-")[0],type_,slug(item["name"]),slug(category),*slug(item["subtitle"]).split("-")]))[:14],
  "source_collection":f"animaciones/source_{'architecture_06' if item['batch']=='architecture' else 'runtime_07' if item['batch']=='runtime' else 'languages_08'}.html",
  "source_module_id":item["id"],"states":len(stages),
 }
 new.append(metadata)
 return block,"\n".join(rules),extra,slug_id
presets=[]
groups={"architecture":[],"runtime":[],"languages":[]}
for i,item in enumerate(sources):
 within=i%10+1
 chunk,rules,extra,slug_id=render_content(item,i+1,within)
 singles=f'<!doctype html><html lang="es"><head><meta charset="utf-8">{CSP}<style>{STYLE}\n'+rules+'\n'+extra+f'\n.module{{visibility:visible}}</style></head><body><header class="collection-header"><span>GPTFRAMES / RECURSO EDITABLE</span><strong>{html.escape(KIND[item["batch"]][0])}</strong></header><div class="progress-line"></div><main class="catalog" data-segment-seconds="{DURATION}">'+chunk+'</main><footer>SVG / HTML PURO · {DURATION}s</footer></body></html>'
 (R/"presets"/(slug_id+".html")).write_text(singles,encoding="utf-8")
 groups[item["batch"]].append((chunk,rules,extra))
for batch,items in groups.items():
 category=KIND[batch][0]
 css=STYLE+"\n"+"\n".join(item[1] for item in items)+items[0][2]
 for i in range(10):
  st=i*10;end=(i+1)*10
  key=f"show_module_{i}"
  css+=(f"\n.catalog>.module:nth-child({i+1}){{animation:{key} 40s linear infinite}}\n"
        f"@keyframes {key}{{0%,{st:.4f}%{{visibility:hidden;opacity:0}}"
        f"{st+.0001:.4f}%,{end-.0001:.4f}%{{visibility:visible;opacity:1}}"
        f"{end:.4f}%,100%{{visibility:hidden;opacity:0}}}}\n")
 body="".join(item[0] for item in items)
 page=(f'<!doctype html><html lang="es"><head><meta charset="utf-8">{CSP}<style>{css}</style></head><body>'
       f'<header class="collection-header"><span>GPTFRAMES / COLECCIÓN 2026</span><strong>{html.escape(category.upper())} · 10 PRESETS</strong></header>'
       f'<div class="progress-line"></div><main class="catalog" data-segment-seconds="{DURATION}">{body}</main>'
       '<footer>GPTFRAMES / BROWSER RENDER · 480P 24 FPS</footer></body></html>')
 name=f"catalogo_{batch}_06_08.html"
 (R/name).write_text(page,encoding="utf-8")
 print("GALLERY",name,len(page),"bytes",40,"seconds")
all_presets["presets"]=current+new
all_presets["total"]=len(all_presets["presets"])
all_presets["duration"]=sum(float(v.get("duration",2.5)) for v in all_presets["presets"])
DOC.write_text(json.dumps(all_presets,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print("PRESETS",len(new),"TOTAL",all_presets["total"],"FIRST",new[0]["id"],"LAST",new[-1]["id"])
