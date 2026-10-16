#!/usr/bin/env python3
"""Generate offline, seekable SVG bar charts. Authoring helper, not a runtime dependency."""
import argparse
import html
import json
import math
import re
from pathlib import Path

SOURCE_SHA256 = "ad65db98d2a55a9b1d2c071cc1c44fc88dc9a603ffdccf55a6989183cf1fd134"
DEFAULT = {
 "title":"Comparar métodos con datos ilustrativos",
 "subtitle":"La altura de cada barra corresponde al valor de su categoría.",
 "tag":"DATOS ILUSTRATIVOS","unit":"Unidades por segundo",
 "source":"Ejemplo didáctico · Sin mediciones de hardware real",
 "image_caption":"",
 "axis_max":200,"tick_step":50,
 "categories":[{"label":"Método A","value":38,"color":"#94A3B8"},{"label":"Método B","value":65,"color":"#22D3EE"},{"label":"Método C","value":124,"color":"#C084FC"},{"label":"Método D","value":182,"color":"#34D399"}],
 "summary_mode":"ratio","value_suffix":"","duration":12
}
def fmt(x):
 return f"{x:.2f}".rstrip("0").rstrip(".").replace(".",",")
def checked(data):
 d=dict(DEFAULT);d.update(data)
 cats=d["categories"]
 if not isinstance(cats,list) or not 1<=len(cats)<=6:raise ValueError("Use one to six categories")
 def number(x):
  if isinstance(x,bool) or not isinstance(x,(int,float)) or not math.isfinite(x):raise ValueError("Values must be finite numbers")
  return float(x)
 maximum=number(d["axis_max"]);step=number(d["tick_step"])
 if maximum<=0 or step<=0 or maximum/step>10:raise ValueError("Use a positive axis and at most ten tick intervals")
 if not math.isclose(maximum/step,round(maximum/step)):raise ValueError("axis_max must be a multiple of tick_step")
 for c in cats:
  if not isinstance(c.get("label"),str) or not 1<=len(c["label"])<=22:raise ValueError("Use short category labels (1..22 characters)")
  value=number(c["value"])
  if not 0<=value<=maximum:raise ValueError("Values must fit the zero-based axis")
  if not re.fullmatch(r"#[0-9a-fA-F]{6}",c.get("color","")):raise ValueError("Colors must be #RRGGBB")
 for key in ["title","subtitle","tag","unit","source","image_caption"]:
  if not isinstance(d[key],str) or len(d[key])>160:raise ValueError("Invalid text field: "+key)
 if len(d["title"])>65 or len(d["subtitle"])>115:raise ValueError("Title/subtitle are too long for this layout")
 if d.get("summary_mode") not in ("ratio","sum"):raise ValueError("Unknown summary mode")
 if not isinstance(d.get("value_suffix"),str) or len(d["value_suffix"])>8:raise ValueError("Invalid value suffix")
 duration=number(d["duration"])
 if duration<4 or duration>120:raise ValueError("Duration must be 4..120 seconds")
 t=d.get("timing",{})
 n=len(cats)
 starts=t.get("categories",[2.2+i*(4.2/max(1,n-1)) for i in range(n)])
 if len(starts)!=n:raise ValueError("One category cue per bar is required")
 growth=t.get("growth",[.8]*n)
 if len(growth)!=n:raise ValueError("One growth duration per bar is required")
 values=t.get("values",[float(starts[i])+.18+float(growth[i]) for i in range(n)])
 if len(values)!=n:raise ValueError("One value cue per bar is required")
 timing={"header":float(t.get("header",.05)),"axes":float(t.get("axes",1.1)),"image":float(t.get("image",.4)),"summary":float(t.get("summary",8.4)),"categories":[float(x) for x in starts],"growth":[float(x) for x in growth],"values":[float(x) for x in values]}
 for k in ["header","axes","image","summary"]:
  if not math.isfinite(timing[k]) or not 0<=timing[k]<duration:raise ValueError("Cue outside timeline")
 for i in range(n):
  start=timing["categories"][i];grow=timing["growth"][i];value=timing["values"][i]
  if not all(math.isfinite(x) for x in [start,grow,value]) or start<0 or grow<=0 or value<start or max(start+.18+grow,value+.35)>duration:raise ValueError("Invalid bar cue or insufficient duration")
  if timing["summary"]<max(start+.18+grow,value+.35):raise ValueError("Summary must follow completed bars and values")
 if timing["summary"]+1.1>duration or timing["header"]+.45>duration or timing["axes"]+.5>duration or timing["image"]+.6>duration:raise ValueError("Insufficient duration for cue completion")
 d["timing"]=timing
 return d

def generate(data):
 d=checked(data);cats=d["categories"];t=d["timing"];suffix=d.get("value_suffix","");maximum=float(d["axis_max"]);step=float(d["tick_step"])
 def e(v):return html.escape(str(v),quote=True)
 total=sum(c["value"] for c in cats)
 highlight=max(range(len(cats)),key=lambda i:cats[i]["value"])
 if d.get("summary_mode")=="sum":
  comparison=f"Mayor proporción: {fmt(cats[highlight]['value'])}{suffix}"
  conclusion=" + ".join(fmt(c["value"]) for c in cats)+f" = {fmt(total)}{suffix}"
 else:
  base=cats[0]["value"]
  if base>0:
   ratio=cats[highlight]["value"]/base
   comparison=f"{fmt(ratio)} × respecto a {cats[0]['label']}"
   conclusion=f"{cats[highlight]['label']} equivale a {fmt(ratio)} veces {cats[0]['label']}."
  else:
   comparison="Comparación desde cero";conclusion="La escala y las barras se recalculan con los datos."
 comparison=d.get("comparison",comparison);conclusion=d.get("conclusion",conclusion)
 if len(comparison)>60 or len(conclusion)>105:raise ValueError("Summary text too long")
 def fade(at,duration=.4):return f"animation:reveal {duration:g}s ease {at:g}s both"
 def text(x,y,value,size=24,field=None,anchor="start",color="#F8FAFC",weight=600,extra=""):
  return f'<text x="{x:g}" y="{y:g}" font-size="{size}" font-weight="{weight}" fill="{color}" text-anchor="{anchor}"'+(f' data-field="{field}"' if field else '')+f' {extra}>{e(value)}</text>'
 tick_count=round(maximum/step);geometry=[]
 def plot(mode,left,right):
  base=520.;top=225.;height=base-top;n=len(cats);slot=(right-left)/n;barw=min(110,slot*.50)
  parts=[f'<g class="plot-{mode}" id="plot-{mode}">']
  parts.append(f'<g data-cue="axes" style="{fade(t["axes"],.5)}">')
  parts.append(text(70,174,d["unit"],23,"unit",color="#B3C6D9"))
  for k in range(tick_count+1):
   value=k*step;y=base-value/maximum*height
   parts.append(text(123,y+8,fmt(value),22,anchor="end",color="#9AAFC4"))
   parts.append(f'<line x1="{left:g}" x2="{right:g}" y1="{y:g}" y2="{y:g}" stroke="#B3C6D9" stroke-opacity="{.55 if k==0 else .16}" stroke-width="{2 if k==0 else 1}"'+(' stroke-dasharray="4 6"' if k else '')+'/>')
  parts.append('</g>')
  for i,c in enumerate(cats):
   x=left+slot*(i+.5)-barw/2;cx=x+barw/2;h=c["value"]/maximum*height;y=base-h;at=t["categories"][i];growth=t["growth"][i]
   geometry.append({"layout":mode,"category":i+1,"value":c["value"],"x":x,"y":y,"width":barw,"height":h,"baseline":base,"axis_max":maximum})
   parts.append(f'<g id="{mode}-category-{i+1}">')
   parts.append(f'<rect id="{mode}-bar-{i+1}" data-cue="bar_{i+1}" data-value="{c["value"]}" x="{x:g}" y="{y:g}" width="{barw:g}" height="{h:g}" rx="8" fill="url(#gradient-{i})" class="bar" style="animation:grow {growth:g}s cubic-bezier(.16,1,.3,1) {at+.18:g}s both"/>')
   parts.append(f'<g data-cue="value_{i+1}" style="{fade(t["values"][i],.35)}">'+text(cx,y-15,fmt(c["value"])+suffix,30,anchor="middle",color=c["color"],weight=800)+ '</g>')
   parts.append(f'<g data-cue="category_{i+1}" style="{fade(at,.35)}">'+text(cx,560,c["label"],24,f"category_{i+1}","middle")+ '</g>')
   if i==highlight:
    parts.append(f'<rect data-cue="highlight" x="{x-6:g}" y="{y-6:g}" width="{barw+12:g}" height="{h+12:g}" rx="13" fill="none" stroke="{c["color"]}" stroke-width="3" style="animation:highlight 1.1s ease {t["summary"]:g}s both"/>')
   parts.append('</g>')
  parts.append('</g>');return "".join(parts)
 gradients="".join(f'<linearGradient id="gradient-{i}" x1="0" y1="0" x2="0" y2="1"><stop offset="0%" stop-color="{c["color"]}"/><stop offset="100%" stop-color="{c["color"]}" stop-opacity=".52"/></linearGradient>' for i,c in enumerate(cats))
 header=f'<g data-cue="header" style="{fade(t["header"],.45)}"><rect x="70" y="28" width="286" height="30" rx="15" fill="#152639"/><circle cx="86" cy="43" r="4" fill="#34D399"/>'+text(100,49,d["tag"],17,"tag",color="#AFC7D9")+text(70,109,d["title"],41,"title",weight=800)+text(70,143,d["subtitle"],22,"subtitle",color="#A5BBD0")+'</g>'
 image=f'<g class="image-only" data-cue="image" style="{fade(t["image"],.6)}"><foreignObject x="935" y="184" width="275" height="320"><div xmlns="http://www.w3.org/1999/xhtml" class="media-container"><img data-field="image" alt=""/></div></foreignObject>'+text(1072.5,548,d["image_caption"],21,"image_caption","middle",color="#A5BBD0")+'</g>'
 # Use fixed SVG translations on outer groups; animate inner groups so CSS never replaces positioning transforms.
 summary=f'<g transform="translate(70 608)"><g data-cue="summary" style="{fade(t["summary"],.55)}"><rect width="1140" height="55" rx="13" fill="#112739" stroke="#28526B"/><circle cx="27" cy="27.5" r="6" fill="{cats[highlight]["color"]}"/>'+text(48,36,conclusion,24,"conclusion")+'</g></g>'
 badge=f'<g transform="translate(925 594)"><g data-cue="comparison" style="{fade(t["summary"],.45)}">'+text(285,0,comparison,21,"comparison","end",color=cats[highlight]["color"],weight=800)+'</g></g>'
 footer=text(70,697,d["source"],18,"source",color="#92ACC2")+text(1210,697,"GPTFrames · Datos ilustrativos",18,anchor="end",color="#92ACC2")
 css="""*{box-sizing:border-box}html,body{margin:0;width:100%;height:100%;overflow:hidden;background:transparent;font-family:system-ui,Arial,sans-serif}svg.chart{display:block;width:100vw;height:100vh}text{font-family:system-ui,Arial,sans-serif}.bar{transform-box:fill-box;transform-origin:center bottom}.plot-image,.image-only{display:none}svg.chart:has(img[data-field="image"][src^="data:image/"]) .plot-wide{display:none}svg.chart:has(img[data-field="image"][src^="data:image/"]) .plot-image,svg.chart:has(img[data-field="image"][src^="data:image/"]) .image-only{display:inline}.media-container{width:100%;height:100%;padding:18px;display:grid;place-items:center;background:#101F31;border:1px solid #294257;border-radius:20px}.media-container img{width:100%;height:100%;object-fit:contain;display:block}.media-container img:not([src]){display:none}@keyframes reveal{from{opacity:0;transform:translateY(8px)}to{opacity:1;transform:translateY(0)}}@keyframes grow{from{transform:scaleY(0)}to{transform:scaleY(1)}}@keyframes highlight{0%{opacity:0}35%{opacity:1}70%{opacity:.4}100%{opacity:.8}}"""
 wide=plot("wide",145,1200);narrow=plot("image",145,870)
 doc='<!doctype html><html lang="es"><head><meta charset="utf-8"><meta http-equiv="Content-Security-Policy" content="default-src \'none\'; script-src \'none\'; connect-src \'none\'; style-src \'unsafe-inline\'; img-src data:; font-src data:;"><title>'+e(d["title"])+'</title><style>'+css+'</style></head><body><svg xmlns="http://www.w3.org/2000/svg" id="gptframes-root" class="chart" viewBox="0 0 1280 720" preserveAspectRatio="xMidYMid meet"><defs>'+gradients+'</defs>'+header+wide+narrow+image+badge+summary+footer+'</svg></body></html>'
 fields={key:{"type":"text","default":value} for key,value in {**{k:d[k] for k in ["title","subtitle","tag","unit","source","image_caption"]},"comparison":comparison,"conclusion":conclusion,**{f"category_{i+1}":c["label"] for i,c in enumerate(cats)}}.items()}
 fields["image"]={"type":"image","label":"Imagen opcional PNG/JPEG/WebP en assets/","default":""}
 events=[{"cue":"header","at":t["header"],"duration":.45},{"cue":"axes","at":t["axes"],"duration":.5},{"cue":"image","at":t["image"],"duration":.6,"optional":True}]
 for i in range(len(cats)):
  events += [{"cue":f"category_{i+1}","at":t["categories"][i],"duration":.35},{"cue":f"bar_{i+1}","at":t["categories"][i]+.18,"duration":t["growth"][i]},{"cue":f"value_{i+1}","at":t["values"][i],"duration":.35}]
 events += [{"cue":"summary","at":t["summary"],"duration":.55},{"cue":"comparison","at":t["summary"],"duration":.45},{"cue":"highlight","at":t["summary"],"duration":1.1}]
 return doc,{"config":d,"duration":d["duration"],"editable":fields,"geometry":geometry,"events":events,"source_sha256":SOURCE_SHA256}

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument("--data",type=Path);p.add_argument("--out",required=True,type=Path);p.add_argument("--metadata",type=Path)
 args=p.parse_args();data=json.loads(args.data.read_text()) if args.data else {}
 doc,meta=generate(data);args.out.parent.mkdir(parents=True,exist_ok=True);args.out.write_text(doc)
 if args.metadata:args.metadata.parent.mkdir(parents=True,exist_ok=True);args.metadata.write_text(json.dumps(meta,ensure_ascii=False,indent=2)+"\n")
 print(json.dumps({"output":str(args.out),"bars":len(meta["config"]["categories"]),"fields":len(meta["editable"]),"duration":meta["duration"]}))
if __name__=="__main__":main()
