#!/usr/bin/env python3
"""User-supplied Master Motion Suite -> editable offline browser presets."""
from pathlib import Path
import json,re,html,urllib.parse
ROOT=Path(__file__).resolve().parent
source=(ROOT/"source_user_03.html").read_text(encoding="utf-8")
original=(ROOT/"catalogo.json")
catalog=json.loads(original.read_text())
base=catalog["presets"]
base=[v for v in base if v.get("batch")!="master-motion-suite-03"]
assert len(base)==23,(len(base),[v["id"] for v in base[-5:]])

pattern=re.compile(r'<div\b[^>]*>|</div\s*>',re.I)
def extract_div_from(start:int):
 depth=0
 for m in pattern.finditer(source,start):
  if m.group().lower().startswith("</div"):depth-=1
  else:depth+=1
  if depth==0:
   return source[start:m.end()]
 raise ValueError("Unclosed div")
starts=[m.start() for m in re.finditer(r'<div\s+class="card"\s*>',source,re.I)]
assert len(starts)==8,len(starts)
cards=[extract_div_from(x) for x in starts]
newsstart=source.index('<div class="news-broadcast-bar">')
cards.append(extract_div_from(newsstart))
head_css=re.search(r'<style>\s*(.*?)\s*</style>',source,re.S).group(1)
# Only keep root variables from global stylesheet; no toolbar/grid rules.
root_css=re.search(r':root\s*\{[^}]*\}',head_css,re.S).group(0)
fields=[
 ("rf-transmit","Antena RF — transmisión","Radiofrecuencia","antena","Ondas circulares que salen exactamente del nodo emisor.",
  [("signal_color","color","Color de señal","--c-cyan")],["antena","rf","emitir","transmisión","señal","torre","wireless"],["onda","expansión"]),
 ("rf-receive","Antena RF — recepción","Radiofrecuencia","antena","Ondas convergentes hacia el nodo receptor.",
  [("signal_color","color","Color de señal","--c-green")],["antena","rf","recibir","recepción","señal","torre","wireless"],["onda","convergencia"]),
 ("browser-mockup","Ventana de navegador","Mockups","navegador","Ventana con barras de control, URL e imagen revelada.",
  [("url","text","Dirección web",""),("image","image","Imagen de pantalla","")],["web","browser","pagina","ventana","imagen","captura","url"],["zoom","fade-in"]),
 ("smartphone-media","Pantalla de smartphone","Mockups","telefono","Mockup de teléfono con una imagen que aparece desde abajo.",
  [("image","image","Imagen de pantalla","")],["smartphone","celular","telefono","pantalla","imagen","movil"],["deslizamiento","fade-in"]),
 ("linux-tower-enterprise","Servidor Linux torre","Hardware","servidor","Torre Linux enterprise con ventilador y LEDs de discos NVMe.",
  [("hostname","text","Nombre del equipo","") ,("drive_1","text","Disco superior",""),("drive_2","text","Disco inferior","")],["linux","servidor","torre","nvme","hardware","ventilador","rack"],["rotación","parpadeo"]),
 ("windows-server-datacenter","Windows Server Datacenter","Hardware","servidor","Rack con servicios Hyper-V, IIS y Active Directory.",
  [("system_name","text","Versión de Windows",""),("service_1","text","Primer servicio",""),("status_1","text","Estado del primero",""),("service_2","text","Segundo servicio",""),("status_2","text","Estado del segundo",""),("service_3","text","Tercer servicio",""),("status_3","text","Estado del tercero","")],["windows","servidor","hyper-v","iis","active directory","datacenter","dashboard"],["pulso"]),
 ("editable-cpu","CPU personalizable","Hardware","procesador","Microprocesador con modelo, frecuencia y socket editables.",
  [("cpu_name","text","Nombre del CPU",""),("cpu_model","text","Modelo del CPU",""),("cpu_socket","text","Socket","")],["cpu","procesador","intel","xeon","core i5","hardware","circuito"],["circuito","recorrido"]),
 ("laptop-boot","Encendido de laptop","Hardware","portatil","Computadora portátil con secuencia de encendido y luz de teclado.",
  [("boot_title","text","Mensaje del sistema",""),("boot_status","text","Estado de arranque","")],["laptop","computadora","portatil","arranque","boot","teclado"],["revelado","iluminación"]),
 ("breaking-news-ticker","Cintillo informativo","Noticias","cintillo","Titular superior y marquesina de noticias en movimiento.",
  [("label","text","Etiqueta",""),("headline","text","Titular",""),("ticker","text","Texto desplazable","")],["noticias","cintillo","breaking news","marquee","titular","texto","ticker","broadcast"],["desplazamiento","parpadeo"])
]
output=ROOT/"presets"
output.mkdir(exist_ok=True)
CSP='<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; script-src \'none\'; connect-src \'none\'; style-src \'unsafe-inline\'; img-src data:; font-src data:">'
RESET='''*{box-sizing:border-box}html,body{margin:0;width:100%;height:100%;overflow:hidden}
body{background:transparent;color:#f8fafc;font-family:system-ui,-apple-system,"Segoe UI",Arial,sans-serif}
.anim-engine{width:100%;height:100%;display:flex;justify-content:center;align-items:center;overflow:hidden;position:relative}
.stage{width:320px;height:275px;display:flex;flex-direction:column;align-items:center;justify-content:center;position:relative}
'''
def inject(inside,pattern,field,attribute=False):
 new,count=re.subn(pattern,lambda m:m.group(1)+f' data-field="{field}"'+m.group(2),inside,count=1,flags=re.I|re.S)
 if count!=1:raise RuntimeError(f"Cannot expose {field}: {pattern}")
 return new

def set_attrs(content,slug):
 # add data-field markers to text or image properties
 if slug=="browser-mockup":
  content=inject(content,r'(<div class="url-input")([^>]*>)',"url")
  content=inject(content,r'(<img\s+src="https://images\.unsplash\.com/[^"]+")([^>]*>)',"image")
 if slug=="smartphone-media":
  content=inject(content,r'(<img\s+src="https://images\.unsplash\.com/[^"]+")([^>]*>)',"image")
 if slug=="linux-tower-enterprise":
  content=inject(content,r'(<div style="font-size:8px; font-family:monospace; color:var\(--c-linux\); text-align:center;")([^>]*>)',"hostname")
  for i,n in enumerate(("drive_1","drive_2"),1):
   target=r'(<span style="font-size:7px;font-family:monospace;color:#64748b;")([^>]*>)'
   # two matches
   matches=list(re.finditer(target,content,re.I|re.S))
   assert len(matches)==2
   m=matches[i-1]; content=content[:m.start(2)]+f' data-field="{n}"'+content[m.start(2):]
 if slug=="windows-server-datacenter":
  content=inject(content,r'(<span style="font-size:10px; font-family:monospace; color:#fff;")([^>]*>)',"system_name")
  for i in range(1,4):
   # default spans in a single card, map each to slot
   pass
  content=content.replace('<span style="color:#64748b;">Hyper-V</span>','<span data-field="service_1" style="color:#64748b;">Hyper-V</span>')
  content=content.replace('<span style="color:#64748b;">IIS Core</span>','<span data-field="service_2" style="color:#64748b;">IIS Core</span>')
  content=content.replace('<span style="color:#64748b;">AD DS</span>','<span data-field="service_3" style="color:#64748b;">AD DS</span>')
  content=content.replace('>ACTIVE</strong>',' data-field="status_1">ACTIVE</strong>')
  content=content.replace('>RUNNING</strong>',' data-field="status_2">RUNNING</strong>')
  content=content.replace('>SYNCED</strong>',' data-field="status_3">SYNCED</strong>')
 if slug=="editable-cpu":
  content=content.replace('id="cpuLabel"','data-field="cpu_name"')
  content=content.replace('<span class="cpu-sub">','<span class="cpu-sub" data-field="cpu_model">')
  content=content.replace('>LGA-2011-v3</span>',' data-field="cpu_socket">LGA-2011-v3</span>')
 if slug=="laptop-boot":
  content=content.replace('>SYSTEM BOOT</span>',' data-field="boot_title">SYSTEM BOOT</span>')
  content=content.replace('>READY</span>',' data-field="boot_status">READY</span>')
 if slug=="breaking-news-ticker":
  content=content.replace('ÚLTIMA HORA','<span data-field="label">ÚLTIMA HORA</span>')
  content=content.replace('class="news-main-headline"','class="news-main-headline" data-field="headline"')
  # Repeat ticker content for seamless motion. Both copies have the editable field.
  def duplicate_ticker(match):
   text=match.group(1).strip()
   if not text: raise ValueError("The source ticker is empty")
   return ('<div class="marquee-content"><span class="marquee-copy" data-field="ticker">'+
           text+'</span><span class="marquee-copy" data-field="ticker" aria-hidden="true">'+
           text+'</span></div>')
  content,changed=re.subn(r'<div class="marquee-content">\s*(.*?)\s*</div>',
                           duplicate_ticker,content,count=1,flags=re.S)
  assert changed==1, "Ticker markup not found"
 return content

def fallback_svg(mode):
 if mode=="browser":
  raw='''<svg xmlns="http://www.w3.org/2000/svg" width="600" height="320"><defs><linearGradient id="g"><stop stop-color="#124b70"/><stop offset="1" stop-color="#062238"/></linearGradient></defs><rect width="600" height="320" fill="url(#g)"/><g fill="none" stroke="#4be0ef" stroke-width="3"><rect x="65" y="65" width="190" height="120" rx="15"/><path d="M65 105h190M270 180h170M310 150h130M310 120h94"/></g><circle cx="465" cy="245" r="38" fill="#18a5b4" opacity=".5"/></svg>'''
 else:
  raw='''<svg xmlns="http://www.w3.org/2000/svg" width="260" height="460"><defs><linearGradient id="g"><stop stop-color="#3b126b"/><stop offset="1" stop-color="#080f34"/></linearGradient></defs><rect width="260" height="460" fill="url(#g)"/><g stroke="#5fe9ff" stroke-width="4" fill="none"><circle cx="130" cy="190" r="83"/><path d="M36 330h188M52 350h155M90 380h80"/></g><circle cx="130" cy="190" r="27" fill="#48a4ff" opacity=".7"/></svg>'''
 return "data:image/svg+xml,"+urllib.parse.quote(raw,safe="")
added=[]
for number,(card,meta) in enumerate(zip(cards,fields),start=24):
 slug,name,cat,typ,desc,schema,tags,movement=meta
 styles=re.findall(r"<style[^>]*>(.*?)</style>",card,re.S|re.I)
 if number!=32:assert len(styles)==1,(slug,len(styles))
 inside=re.sub(r"<style[^>]*>.*?</style>","",card,flags=re.I|re.S)
 inside=re.sub(r'<span class="card-label">.*?</span>','',inside,count=1,flags=re.I|re.S)
 # Strip only outer .card to avoid layout inherited from original grid.
 inside=re.sub(r'^<div class="card">\s*','',inside)
 if number!=32:
  assert inside.rstrip().endswith("</div>")
  inside=inside.rstrip()[:-6]
 if number==32:
  # ticker extracted complete
  pass
 if "<script" in inside.lower() or "onerror" in inside.lower() or "onclick" in inside.lower():
  raise RuntimeError("Unsafe HTML")
 inside=set_attrs(inside,slug)
 if slug in ("browser-mockup","smartphone-media"):
  inside=re.sub(r'(<img[^>]*\bsrc=")https://images\.unsplash\.com/[^"]+',
                lambda m:m.group(1)+fallback_svg("browser" if slug=="browser-mockup" else "mobile"),inside)
  if slug=="browser-mockup":
   inside=inside.replace("https://images.unsplash.com/photo-workspace","gptframes.local/inicio")
 if "http://" in inside or "https://" in inside:
  # Display text with 'https://...' is allowed if not an external resource
  if '<img src="https:' in inside:raise RuntimeError("External image not allowed")
 css=RESET+root_css+"\n"+(styles[0] if styles else re.findall(r"<style[^>]*>(.*?)</style>",source[:newsstart],re.S|re.I)[-1])
 if slug=="breaking-news-ticker":
  # Show readable headlines immediately; two copies give a continuous
  # marquee from t=0, rather than hiding the news offscreen for 18 seconds.
  css+="""
  .anim-engine .marquee-track-container{height:42px;overflow:hidden;background:#030712;border-top:1px solid #1c4255}
  .anim-engine .marquee-content{display:flex;width:max-content;white-space:nowrap;
    animation:newsCrawl 12s linear infinite!important;will-change:transform;
    font-size:15px;color:#73f4fa;letter-spacing:.35px}
  .anim-engine .marquee-copy{display:block;flex:0 0 auto;white-space:nowrap;
    padding:0 52px 0 0;font:700 15px/42px ui-monospace,SFMono-Regular,monospace}
  .anim-engine .news-main-headline{font-size:17px;line-height:1.25;letter-spacing:.15px}
  .anim-engine .breaking-tag{font-size:13px;white-space:nowrap}
  @keyframes newsCrawl{from{transform:translateX(0)}to{transform:translateX(-50%)}}
  """
 # Original animation selectors rely on .anim-engine; append .stage if desired.
 structure=f'<div class="anim-engine"><div class="stage">{inside}</div></div>'
 if slug=="breaking-news-ticker":
  structure=f'<div class="anim-engine"><div class="stage" style="width:100%;max-width:800px">{inside}</div></div>'
 page='<!doctype html><html lang="es"><head><meta charset="utf-8">'+CSP+'<style>'+css+'</style></head><body>'+structure+'</body></html>'
 (output/(slug+".html")).write_text("\n".join(line.rstrip() for line in page.splitlines())+"\n",encoding="utf-8")
 added.append({"number":number,"id":slug,"name":name,"category":cat,"type":typ,
  "description":desc,"style":"dark / premium / tech", "backend":"browser",
  "collection":"animaciones","preset_file":f"animaciones/presets/{slug}.html",
  "status":"candidate","duration":3.5,"batch":"master-motion-suite-03",
  "editable":{key:dict({"type":type_,"label":label},**({"css_var":cssvar} if type_=="color" else {})) for key,type_,label,cssvar in schema},
  "tags":tags,"motion":movement,"aliases":[],
  "usage":["educación","tecnología","documental"]})
assert len(added)==9
catalog["presets"]=base+added
catalog["total"]=len(catalog["presets"])
catalog["description"]="Colección de recursos visuales editables, categorizados y buscables."
catalog["duration"]=sum(float(p.get("duration",2.5)) for p in catalog["presets"])
original.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+"\n")
print("ADAPTED",len(added),"NEW TOTAL",catalog["total"])
