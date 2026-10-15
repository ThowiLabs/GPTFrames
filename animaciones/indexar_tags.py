#!/usr/bin/env python3
"""Curated semantics + CSS-motion inspection. Run after all collection builders."""
import json,re,unicodedata
from collections import Counter
from pathlib import Path
ROOT=Path(__file__).resolve().parent
p=ROOT/"catalogo.json"
doc=json.loads(p.read_text())
assert doc["total"]>=32
stop={"a","de","del","con","en","el","la","las","los","y","una","un","que","se","desde","para","tipo","sobre","cada","por","al","com","and","css","html"}
type_override={
"smartphone-vibrate":"teléfono","success-checkmark":"confirmación","padlock-closing":"candado",
"push-notification":"notificación","audio-equalizer":"ecualizador","biometric-scanner":"escáner",
"linux-server":"servidor","moving-car":"vehículo","analog-clock":"reloj",
"tachometer":"medidor","notification-bell":"notificación",
"data-chart":"gráfica","neural-network":"red","data-stream":"efecto",
"rf-transmit":"antena","rf-receive":"antena",
"browser-mockup":"mockup","smartphone-media":"mockup",
"linux-tower-enterprise":"servidor","windows-server-datacenter":"servidor",
"editable-cpu":"procesador","laptop-boot":"portátil","breaking-news-ticker":"cintillo"
}
category_type={"Personajes":"personaje","Dispositivos":"teléfono","Estados":"estado",
"Seguridad":"seguridad","Notificaciones":"notificación","Audio":"audio",
"Hardware":"hardware","Transiciones":"transición","Datos":"visualización",
"Loaders":"loader","Interfaz":"interfaz","Mockups":"mockup",
"Radiofrecuencia":"antena","Noticias":"cintillo"}
extras={
"smartphone-vibrate":["llamada","entrante","vibracion","ondas","telefono","celular"],
"success-checkmark":["palomita","check","tick","validado","confirmado"],
"padlock-closing":["bloqueo","candado","cerrar","seguridad","cerradura"],
"push-notification":["mensaje","alerta","notificacion","aviso"],
"audio-equalizer":["ondas de audio","visualizador","musica","voz","barras"],
"biometric-scanner":["biometria","huella","laser","identidad","reconocimiento"],
"linux-server":["rack","linux","infraestructura","computacion"],
"windows-server-datacenter":["windows server","servidor windows","hyper-v","iis","ad ds"],
"editable-cpu":["intel xeon","intel core i5","microprocesador","lga 2011","socket","hardware"],
"laptop-boot":["arranque","encendido","boot","laptop","portatil"],
"breaking-news-ticker":["noticiero","marquesina","última hora","broadcast","texto editable","ticker","titular"],
"rf-transmit":["antena emisor","transmisión rf","radiofrecuencia","ondas salientes"],
"rf-receive":["antena receptor","recepción rf","radiofrecuencia","ondas entrantes"],
"browser-mockup":["página web","imagen editable","web browser","url","screenshot"],
"smartphone-media":["movil","telefono","celular","imagen editable","pantalla"],
"linux-tower-enterprise":["servidor linux","torre linux","nvme","refrigeracion","ventilador"],
}
def strip_accents(s):
 return "".join(x for x in unicodedata.normalize("NFD",s.lower()) if unicodedata.category(x)!="Mn")
def motion_detect(css):
 found=[]
 matchers=[
 ("rotación",r"rotate\(|rotation:|@keyframes\s+(?:\w*spin|\w*rotate)"),
 ("deslizamiento",r"translate[XY]?\(|transform:\s*translate|@keyframes\s+.*(?:drop|slide|crawl|move)"),
 ("escala",r"scale\(|\bscale:|\banimation.*pulse"),
 ("opacidad",r"\bopacity\b"),
 ("parpadeo",r"@keyframes\s+\w*(?:blink|flicker|glow|flash)\w*"),
 ("trazado",r"stroke-dashoffset|stroke-dasharray"),
 ("cambio de color",r"background-color:|border-color:|color:\s*var\("),
 ("pulso",r"@keyframes\s+\w*(?:pulse|breathe|breath|beat)\w*"),
 ("ondas",r"@keyframes\s+\w*(?:emit|converge|wave|expand)\w*"),
 ("barrido",r"@keyframes\s+\w*(?:scan|laser|sweep|shimmer)\w*"),
 ("rebote",r"@keyframes\s+\w*(?:bounce|snap|impact|pop|jump)\w*")
 ]
 for k,pat in matchers:
  if re.search(pat,css,re.I):found.append(k)
 return found
for item in doc["presets"]:
 html_path=ROOT.parent/item["preset_file"]
 assert html_path.is_file(),html_path
 code=html_path.read_text(encoding="utf-8")
 css="\n".join(re.findall(r"<style\b[^>]*>(.*?)</style>",code,re.I|re.S))
 found=motion_detect(css)
 existing=item.get("motion",[])
 item["motion"]=list(dict.fromkeys(existing+found))
 item["type"]=item.get("type") or type_override.get(item["id"],(
    "personaje" if item["id"].startswith("avatar-") else
    "loader" if item["id"] in ("radial-neon","shimmer-progress") else
    category_type.get(item["category"],"animación")))
 if item["id"].startswith("avatar-"):
  emotion=item["id"].split("-",1)[1]
  extras[item["id"]]=["expresión","emoción","avatar",emotion,"reaccion","carita"]
 item["usage"]=item.get("usage") or {
 "Seguridad":["autenticación","privacidad","ciberseguridad"],
 "Radiofrecuencia":["telecomunicaciones","conectividad","redes"],
 "Hardware":["educación tecnológica","infraestructura","hardware"],
 "Noticias":["reportaje","documental","noticiero"],
 "Mockups":["presentación","demostración","tutorial"],
 "Personajes":["reacciones","explicación","emociones"]
 }.get(item["category"],["vídeo explicativo","motion graphics"])
 human=re.findall(r"[\wáéíóúüñÁÉÍÓÚÜÑ]+", " ".join([item["name"],item["category"],item["style"],item["description"]]), re.UNICODE)
 words=[strip_accents(x) for x in human if len(x)>2 and strip_accents(x) not in stop]
 original=item.get("tags",[])
 item["tags"]=list(dict.fromkeys(
    [str(x).strip().lower() for x in original]+
    extras.get(item["id"],[])+words+
    [strip_accents(item["type"]),strip_accents(item["category"])]
 ))[:36]
 item["css_keyframes"]=list(dict.fromkeys(re.findall(r"@keyframes\s+([\w-]+)",css,re.I)))[:32]
 item["searchable"]=True
 assert item["tags"] and item["type"]
p.write_text(json.dumps(doc,ensure_ascii=False,indent=2)+"\n")
counts=Counter(x["category"] for x in doc["presets"])
print("Indexed",len(doc["presets"]),"assets",len(set(t for x in doc["presets"] for t in x["tags"])),"unique tags")
print("Categories:",dict(counts))
print("CPU editable:",next(x["editable"] for x in doc["presets"] if x["id"]=="editable-cpu"))
