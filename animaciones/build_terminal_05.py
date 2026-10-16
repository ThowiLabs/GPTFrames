#!/usr/bin/env python3
"""Convert four animated JS terminals to deterministic CSS-only local presets."""
from pathlib import Path
import html,json,re
R=Path(__file__).resolve().parent
src=(R/"source_terminal_05.html").read_text()
out=R/"catalogo.json"; catalog=json.loads(out.read_text())
baseline=[x for x in catalog["presets"] if x.get("batch")!="linux-terminal-05"]
assert len(baseline)==61
def js_strings(name):
 m=re.search(r'const\s+'+name+r'\s*=\s*\[(.*?)\];',src,re.S)
 assert m,name
 return re.findall(chr(96)+'(.*?)'+chr(96),m.group(1),re.S)
bash_lines=js_strings("outputLines")
spec_lines=js_strings("specs")
code_lines=js_strings("codeLines")
boots=re.findall(r'\{\s*time:\s*"([^"]+)",\s*type:\s*"([^"]+)",\s*text:\s*"([^"]+)"\s*\}',src)
assert len(bash_lines)>=15 and len(spec_lines)>=12 and len(code_lines)==7 and len(boots)>=22
CSP='<meta http-equiv="Content-Security-Policy" content="default-src &#39;none&#39;; script-src &#39;none&#39;; connect-src &#39;none&#39;; style-src &#39;unsafe-inline&#39;; img-src data:; font-src data:">'
CORE="""
*{box-sizing:border-box}html,body{margin:0;width:100%;height:100%;overflow:hidden}
:root{--bg:#070b13;--surface:#0b101c;--head:#121a2a;--edge:#293c54;--text:#dce9f4;--dim:#9aaec4;--c-green:#22c55e;--c-cyan:#06b6d4;--c-blue:#38bdf8;--c-yellow:#facc15;--c-orange:#f97316;--c-red:#ef4444;--c-purple:#c084fc;--text-muted:#64748b}
body{background:transparent;color:var(--text);font-family:ui-monospace,"Cascadia Code",Consolas,monospace}
.anim-engine{width:100%;height:100%;display:grid;place-items:center}
.card{width:100%;height:100%;background:var(--surface);border:1px solid var(--edge);border-radius:14px;overflow:hidden;display:flex;flex-direction:column;box-shadow:0 15px 42px #0008}
.head{height:42px;background:var(--head);border-bottom:1px solid var(--edge);display:flex;align-items:center;justify-content:space-between;padding:0 16px;flex:none}
.dots{display:flex;gap:7px}.dots b{width:11px;height:11px;border-radius:50%;background:#ef4444}.dots b:nth-child(2){background:#eab308}.dots b:nth-child(3){background:#22c55e}
.title{font-size:11px;color:#a7bcd0;font-weight:700}.tag{font-size:10px;border:1px solid #245849;color:#69f0a6;background:#0b2721;padding:5px 9px;border-radius:20px}
.body{flex:1;min-height:0;padding:16px 18px;overflow:hidden;font-size:11.5px;line-height:1.55;position:relative}
.cursor{display:inline-block;width:7px;height:13px;background:#eaf4ff;vertical-align:-2px;animation:blink .85s steps(2,start) infinite}
@keyframes blink{0%,100%{opacity:1}50%{opacity:0}}
.prompt-user{color:var(--c-green);font-weight:700}.prompt-at{color:#72849b}.prompt-host{color:var(--c-cyan);font-weight:700}.prompt-path{color:var(--c-yellow)}
.boot-time{color:var(--c-cyan)}.boot-ok{color:var(--c-green);font-weight:700}.boot-info{color:var(--c-purple)}
.line{opacity:0;transform:translateY(6px);animation:appear .22s ease-out var(--at,0s) forwards}
@keyframes appear{to{opacity:1;transform:translateY(0)}}
.typed{display:inline-block;vertical-align:bottom;max-width:100%;white-space:nowrap;overflow:hidden;clip-path:inset(0 100% 0 0);animation:type .95s steps(26,end) var(--start,.3s) forwards}
@keyframes type{to{clip-path:inset(0 0 0 0)}}
"""
def header(title,label):
 return '<div class="card"><header class="head"><div class="dots"><b></b><b></b><b></b></div><span class="title">'+html.escape(title)+'</span><span class="tag">'+label+'</span></header>'
def prompt():
 return '<span class="prompt-user" data-field="user">tux</span><span class="prompt-at">@</span><span class="prompt-host" data-field="host">linux-srv</span>:<span class="prompt-path">~</span>$ '
def step(text,t):
 return '<div class="line" style="--at:%.2fs">%s</div>'%(t,text)
def emit(slug,markup,css):
 page='<!doctype html><html lang="es"><head><meta charset="utf-8">'+CSP+'<style>'+CORE+css+'</style></head><body><main class="anim-engine">'+markup+'</main></body></html>'
 assert '<script' not in page.lower()
 (R/"presets"/(slug+".html")).write_text(page)
# 1: Bash service and progressively streamed results
lines=''.join(step(line,2.7+i*.18) for i,line in enumerate(bash_lines))
bash=header("bash — tux@linux-srv: ~ (80×24)","SHELL")
bash+='<div class="body"><div class="bash-scroll"><div>'+prompt()+'<span class="typed" data-field="command" style="--start:.45s;animation-duration:1.8s">systemctl status nginx.service</span><span class="cursor"></span></div><div class="log">'+lines+'</div></div></div></div>'
emit("terminal-bash-nginx",bash,""".bash-scroll{animation:scrollBash 7.2s ease-in-out both}.log{margin-top:12px}.log .line{white-space:nowrap;font-size:10.5px;line-height:1.65}@keyframes scrollBash{0%,35%{transform:translateY(0)}100%{transform:translateY(-146px)}}""")
# 2: timed kernel/systemd messages.
logs=[]
for i,(timestamp,kind,msg) in enumerate(boots):
 if kind=="kernel":inner='[ <span class="boot-time">'+timestamp+'</span> ] '+html.escape(msg)
 elif kind=="sysd":inner='[ <span class="boot-info">'+timestamp+'</span> ] <span style="color:var(--c-purple)">'+html.escape(msg)+'</span>'
 elif kind=="ok":inner='[ <span class="boot-ok"> OK </span> ] '+html.escape(msg)
 else:inner='<span style="color:var(--c-cyan)" data-field="login">'+html.escape(msg)+'</span> <span class="cursor"></span>'
 logs.append(step(inner,.2+i*.25))
boot=header("systemd-boot — v256.4-1-arch — tty1","BOOT")+'<div class="body boot-body"><div class="boot-scroll">'+''.join(logs)+'</div></div></div>'
emit("terminal-systemd-liveboot",boot,""".boot-body{font-size:10.5px}.boot-scroll{animation:scrollBoot 7.2s ease-in-out both}.boot-scroll .line{white-space:nowrap;margin-bottom:3px}@keyframes scrollBoot{0%,24%{transform:translateY(0)}100%{transform:translateY(-314px)}}""")
# 3: fastfetch styled Arch info, one row per item.
specs=''.join(step(x,1.5+i*.20) for i,x in enumerate(spec_lines))
logo='''   /\\
  /  \\
 /\\   \\
/      \\
/  ,,  \\
/  | |  \\
/_-''  ''-_\\'''
fetch=header("fastfetch — zsh (100×30)","HARDWARE")
fetch+='<div class="body"><div>'+prompt()+'<span class="typed" data-field="command" style="--start:.3s;animation-duration:.9s">fastfetch</span><span class="cursor"></span></div><div class="fetch-columns"><pre class="logo">'+html.escape(logo)+'</pre><div class="specs">'+specs+'</div></div></div></div>'
emit("terminal-fastfetch-reveal",fetch,""".fetch-columns{display:flex;gap:27px;align-items:flex-start;margin-top:15px}.logo{color:var(--c-cyan);margin:0;font-size:15px;line-height:1.32}.specs{min-width:0;font-size:10.6px;line-height:1.25}.specs .line{margin-bottom:4px}.nf-key{color:var(--c-yellow);font-weight:700}.palette-row{display:flex;gap:4px}.palette-box{width:12px;height:9px;border-radius:2px}""")
# 4: Vim INSERT -> :wq -> written -> shell. No JS-dependent states.
code=''.join(step(x,1.16+i*.27) for i,x in enumerate(code_lines))
vim=header("nvim ~/daemon.py","EDITOR")
vim+='''<div class="vim"><div class="editor"><aside class="gutter">1<br>2<br>3<br>4<br>5<br>6<br>7<br>~<br>~</aside><div class="viewport"><div class="source">'''+code+'''</div><div class="exit">[nvim: exit 0]<div>'''+prompt()+'''python3 <span data-field="filename">daemon.py</span></div><div style="color:#4ade80">[+] Initializing Linux Daemon...</div><div>'''+prompt()+'''<span class="cursor"></span></div></div></div></div>
<div class="status"><span class="mode-normal">NORMAL</span><span class="mode-insert">INSERT</span><span style="margin-left:auto;color:#b7c8dd">daemon.py [utf-8]</span><span>7,1 100%</span></div>
<div class="cmd"><span class="insert">-- INSERT --</span><span class="save">:wq</span><span class="written" data-field="write_message">"daemon.py" 7L, 168B written</span></div></div></div>'''
emit("terminal-vim-edit-save",vim,""".vim{flex:1;min-height:0;display:flex;flex-direction:column;background:#070a12;font-size:11.3px}.editor{flex:1;display:flex;min-height:0}.gutter{background:#0a101d;border-right:1px solid #203044;color:#455970;padding:12px 9px;width:35px;flex:none;text-align:right}.viewport{flex:1;position:relative;overflow:hidden;padding:12px}.source,.exit{position:absolute;inset:12px}.source{animation:sourceLeave 7.5s steps(1,end) both}.exit{animation:exitEnter 7.5s steps(1,end) both;line-height:1.65}.source .line{line-height:1.58}.status{height:28px;background:#1e293b;display:flex;align-items:center;gap:11px;padding:0 7px;color:#cbd5e1;font-weight:700;font-size:10px}.mode-normal,.mode-insert{padding:4px 9px;border-radius:3px}.mode-normal{background:var(--c-blue);color:#051520;animation:normalMode 7.5s steps(1,end) both}.mode-insert{background:var(--c-green);color:#051520;animation:insertMode 7.5s steps(1,end) both}.cmd{height:29px;border-top:1px solid #243449;position:relative}.cmd span{position:absolute;top:6px;left:12px}.cmd .insert{color:var(--c-green);animation:insertMode 7.5s steps(1,end) both}.cmd .save{animation:saveMode 7.5s steps(1,end) both}.cmd .written{color:var(--c-cyan);animation:writtenMode 7.5s steps(1,end) both}
@keyframes normalMode{0%,10%{opacity:1}11%,57%{opacity:0}58%,100%{opacity:1}}
@keyframes insertMode{0%,10%{opacity:0}11%,57%{opacity:1}58%,100%{opacity:0}}
@keyframes saveMode{0%,63%{opacity:0}64%,75%{opacity:1}76%,100%{opacity:0}}
@keyframes writtenMode{0%,76%{opacity:0}77%,91%{opacity:1}92%,100%{opacity:0}}
@keyframes sourceLeave{0%,91%{opacity:1}92%,100%{opacity:0}}
@keyframes exitEnter{0%,91%{opacity:0}92%,100%{opacity:1}}
""")
items=[
("terminal-bash-nginx","Bash — diagnóstico nginx","Tecleo progresivo de systemctl status nginx.service y salida del servicio.",7.2,{"user":"Usuario","host":"Host","command":"Comando"},["bash","nginx","systemctl","terminal","shell","estado"]),
("terminal-systemd-liveboot","Arranque kernel y systemd","Mensajes del kernel y servicios con entrada progresiva y desplazamiento.",7.2,{"login":"Texto de inicio"},["linux","systemd","boot","arranque","dmesg","kernel"]),
("terminal-fastfetch-reveal","Fastfetch — información Linux","Comando fastfetch, logo Arch y especificaciones del equipo en cascada.",5.5,{"user":"Usuario","host":"Host","command":"Comando"},["fastfetch","neofetch","terminal","arch","hardware"]),
("terminal-vim-edit-save","Editor Vim — guardar archivo","Simula INSERT, edición, :wq, mensaje de guardado y salida a Bash.",7.5,{"user":"Usuario","host":"Host","filename":"Archivo","write_message":"Mensaje guardado"},["vim","neovim","codigo","guardar","editor","python"])]
new=[]
for i,(slug,name,desc,duration,properties,tags) in enumerate(items,62):
 new.append({"number":i,"id":slug,"name":name,"category":"Terminales Linux","type":"terminal",
    "description":desc,"style":"Linux / ANSI / terminal","backend":"browser","collection":"animaciones",
    "preset_file":"animaciones/presets/"+slug+".html","status":"candidate","batch":"linux-terminal-05",
    "duration":duration,"tags":tags,"motion":["escritura","revelado","secuencia","scroll"],
    "editable":{k:{"type":"text","label":v} for k,v in properties.items()},
    "usage":["tutorial Linux","documental técnico","demostración"]})
catalog["presets"]=baseline+new
catalog["total"]=len(catalog["presets"])
catalog["duration"]=sum(float(v.get("duration",2.5)) for v in catalog["presets"])
out.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+"\n")
print("NEW",len(new),"TOTAL",catalog["total"],"LINES",len(bash_lines),len(boots),len(spec_lines),len(code_lines))
