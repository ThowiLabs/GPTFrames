#!/usr/bin/env python3
"""Adapt source_linux_04.html into 29 isolated CSS presets (exclude #26 Debian).
Source is user-authored; rebuild with python3 animaciones/build_linux_04.py.
No remote resource dependencies, no JavaScript in output.
"""
from pathlib import Path
import re,json,html,hashlib
R=Path(__file__).resolve().parent
source=R/"source_linux_04.html"
data=source.read_text(encoding="utf-8")
catalog=R/"catalogo.json"
registry=json.loads(catalog.read_text(encoding="utf-8"))
baseline=[x for x in registry["presets"] if x.get("batch")!="linux-motion-04"]
assert len(baseline)==32, f"Unexpected existing catalog count {len(baseline)}"
matches=list(re.finditer(r'<div class="card">',data))
assert len(matches)==30
token=re.compile(r'<div\b[^>]*>|</div\s*>',re.I)
def card_after(start):
    depth=0
    for m in token.finditer(data,start):
        depth+=-1 if m.group().startswith("</") else 1
        if depth==0:return data[start:m.end()]
    raise ValueError(f"Card at {start} was not closed")
source_cards=[card_after(m.start()) for m in matches]
root_vars=re.search(r':root\s*\{.*?\}',data,re.S).group()
items=[
("kernel-panic","Kernel Panic","Kernel y arranque","Alerta del kernel que pulsa con borde rojo.","kernel,error,panic,sistema,linux,alerta","parpadeo"),
("systemd-boot","Inicio con systemd","Kernel y arranque","Unidades de arranque que entran secuencialmente.","systemd,boot,inicio,servicios,consola","entrada"),
("root-command-warning","Aviso de comando root","Kernel y arranque","Terminal que se sacude tras una operación bloqueada.","bash,terminal,sudo,root,advertencia,comando","sacudida"),
("fastfetch-info","Fastfetch y hardware","Kernel y arranque","Información del equipo presentada con un pequeño Tux.","fastfetch,neofetch,linux,sistema,hardware,informacion","revelado"),
("vim-save","Vim guardar y salir","Kernel y arranque","Estado del editor cambia de INSERT a guardando.","vim,editor,guardar,archivo,terminal","secuencia"),
("ssh-randomart","SSH Randomart","Kernel y arranque","Huella visual de clave SSH que aparece por pasos.","ssh,llaves,keygen,randomart,seguridad","escritura"),
("htop-multicore","Carga multinúcleo","Procesos y memoria","Barras de utilización de cuatro núcleos que oscilan.","htop,cpu,nucleos,procesos,carga","barras"),
("memory-swap","RAM y Swap","Procesos y memoria","Indicadores de memoria que crecen y decrecen.","ram,swap,memoria,uso,barras","progreso"),
("oom-killer","OOM Killer","Procesos y memoria","Objetivo circular giratorio con aviso de proceso terminado.","oom,memoria,proceso,killer,alerta","rotacion"),
("fork-bomb-diagram","Árbol de procesos","Procesos y memoria","Ramas de procesos con nodos que laten.","fork,procesos,arbol,sistema,diagrama","pulso"),
("load-average","Gráfica load average","Procesos y memoria","Curva de carga del sistema que se dibuja.","loadavg,grafica,estadisticas,rendimiento","trazado"),
("unix-pipeline","Pipeline de Unix","Procesos y memoria","Comandos encadenados con un pulso de flujo.","pipeline,unix,pipe,comandos,flujo","pulso"),
("pacman-upgrade","Pacman actualizando","Paquetes y red","Pacman animado consume paquetes mientras actualiza.","pacman,arch,paquetes,actualizacion,descarga","desplazamiento"),
("raid-resync","Sincronización RAID","Paquetes y red","Unidades del arreglo RAID con LEDs intermitentes.","raid,discos,almacenamiento,sincronizacion,led","parpadeo"),
("icmp-ping","Ping ICMP","Paquetes y red","Paquete de red rebota entre dos equipos.","ping,icmp,red,latencia,host","trayectoria"),
("cron-job","Trabajo cron","Paquetes y red","Engranaje giratorio que simboliza una tarea programada.","cron,programar,automatizacion,linux,horario","rotacion"),
("firewall-drop","Cortafuegos DROP","Paquetes y red","Un paquete se aproxima a una barrera y desaparece.","firewall,iptables,ufw,seguridad,bloqueo","impacto"),
("symlink-arrow","Enlace simbólico","Paquetes y red","Flecha animada que conecta un alias y su destino.","symlink,enlace,archivos,ln,alias","rebote"),
("chmod-permissions","Permisos chmod","Seguridad y archivos","Permisos cambian visiblemente de 777 a 700.","chmod,permisos,acceso,seguridad,archivo","secuencia"),
("sudo-elevation","Privilegios sudo","Seguridad y archivos","Escudo que aumenta de tamaño y se ilumina.","sudo,root,privilegios,elevacion,escudo","escala"),
("selinux-radar","Radar SELinux","Seguridad y archivos","Barrido circular de estado enforcing.","selinux,enforcing,seguridad,radar,linux","rotacion"),
("chroot-isolation","Aislamiento chroot","Seguridad y archivos","Punto confinado que recorre su celda.","chroot,jail,aislamiento,contenedor,seguridad","trayectoria"),
("gpg-verify","Firma GPG","Seguridad y archivos","Sello VERIFIED estampado con rebote.","gpg,firma,criptografia,verified,seguridad","rebote"),
("directory-tree","Árbol de directorios","Seguridad y archivos","Ramas de archivos reveladas una tras otra.","tree,archivos,directorios,carpetas,linux","entrada"),
("tux-waving","Tux saludando","Distros y hardware","Pingüino Tux mueve una de sus aletas.","tux,linux,pinguino,mascota,saludo","rotacion"),
# Original #26 Debian deliberately excluded at user request.
("arch-geometry","Geometría Arch Linux","Distros y hardware","Triángulos geométricos de Arch se construyen.","arch,linux,triangulos,logo,distro","escala"),
("ubuntu-friends","Ubuntu Circle of Friends","Distros y hardware","Anillo y tres nodos de Ubuntu en rotación.","ubuntu,linux,circulo,logo,distro","rotacion"),
("grub-menu","Selector GNU GRUB","Distros y hardware","Selección del cargador de arranque cambia de opción.","grub,arranque,menu,linux,bootloader","selector"),
("raspberry-gpio","Pines Raspberry Pi GPIO","Distros y hardware","Señales intermitentes en conectores GPIO.","raspberry,pi,gpio,hardware,pines,led","parpadeo")
]
assert len(items)==29
original_numbers=[i for i in range(1,31) if i!=26]
out=R/"presets";out.mkdir(exist_ok=True)
CSP='<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; script-src \'none\'; connect-src \'none\'; style-src \'unsafe-inline\'; img-src data:; font-src data:">'
css_base="""*{box-sizing:border-box}html,body{width:100%;height:100%;overflow:hidden;margin:0}
body{background:transparent;color:#e2e8f0;font-family:ui-monospace,Consolas,Menlo,monospace}
.anim-engine{width:100%;height:100%;display:flex;align-items:center;justify-content:center;overflow:hidden}
.stage{position:relative;width:290px;height:235px;display:flex;align-items:center;justify-content:center}
.linux-asset{width:290px;height:215px;display:flex;flex-direction:column;justify-content:center;align-items:center;transform-origin:center;transform:scale(1.18)}
"""
recorded=[]
for i,(n,(slug,name,cat,desc,tags,motion)) in enumerate(zip(original_numbers,items)):
    source_card=source_cards[n-1]
    css_list=re.findall(r'<style\s*>(.*?)</style>',source_card,re.S|re.I)
    assert len(css_list)==1,(n,slug)
    markup=re.sub(r'<style\s*>.*?</style>','',source_card,flags=re.S|re.I)
    markup=re.sub(r'<span class="card-label">.*?</span>','',markup,count=1,flags=re.S|re.I)
    markup=re.sub(r'^\s*<div class="card">\s*','',markup,count=1)
    assert markup.rstrip().endswith('</div>')
    markup=markup.rstrip()[:-6].strip()
    assert '<script' not in markup.lower() and 'onerror=' not in markup.lower()
    css=css_list[0]
    if slug=="vim-save":
        # CSS content in @keyframes cannot mutate ordinary DOM text.
        markup=markup.replace('<span class="vim-status" style="font-weight:bold;">:wq</span>',
          '<span class="vim-state vim-insert">-- INSERT --</span><span class="vim-state vim-save">:wq</span><span class="vim-state vim-done">"main.rs" written</span>')
        css+="""
        .vim-status{display:none}
        .vim-state{display:block;font-size:10px;font-weight:800}
        .vim-insert{color:#22c55e;animation:vimInsert 2.4s steps(1) infinite}
        .vim-save{color:#fff;animation:vimCommit 2.4s steps(1) infinite}
        .vim-done{color:#06b6d4;animation:vimDone 2.4s steps(1) infinite}
        @keyframes vimInsert{0%,40%{display:block;opacity:1}41%,100%{display:none;opacity:0}}
        @keyframes vimCommit{0%,45%{display:none;opacity:0}50%,79%{display:block;opacity:1}80%,100%{display:none;opacity:0}}
        @keyframes vimDone{0%,85%{display:none;opacity:0}90%,100%{display:block;opacity:1}}
        """
    if slug=="chmod-permissions":
        markup=markup.replace('<span class="perm-bits">-rwx------ (700)</span>',
           '<span class="perm-before">-rwxrwxrwx (777)</span><span class="perm-after">-rwx------ (700)</span>')
        css+="""
        .perm-before,.perm-after{display:inline-block;white-space:nowrap;animation-duration:2.4s;animation-iteration-count:infinite;animation-timing-function:steps(1)}
        .perm-before{color:#e11d48;animation-name:permBefore}
        .perm-after{color:#22c55e;animation-name:permAfter}
        @keyframes permBefore{0%,47%{display:inline-block}48%,100%{display:none}}
        @keyframes permAfter{0%,47%{display:none}48%,100%{display:inline-block}}
        """
    # Avoid original short CSS motion being fully played before the catalog chapter begins.
    body=f'<div class="anim-engine"><div class="stage"><div class="linux-asset">{markup}</div></div></div>'
    page=f'<!doctype html><html lang="es"><head><meta charset="utf-8">{CSP}<style>{css_base}{root_vars}\n{css}</style></head><body>{body}</body></html>'
    (out/(slug+".html")).write_text("\n".join(x.rstrip() for x in page.splitlines())+"\n",encoding="utf-8")
    recorded.append({"number":len(baseline)+len(recorded)+1,"source_number":n,"id":slug,"name":name,
      "category":cat,"type":("linux-sistema" if cat!="Distros y hardware" else "linux-hardware"),
      "description":desc,"style":"dark / Linux / consola","backend":"browser",
      "collection":"animaciones","preset_file":f"animaciones/presets/{slug}.html",
      "status":"candidate","duration":2.5,"batch":"linux-motion-04",
      "tags":list(dict.fromkeys(["linux",*tags.split(",")])), "motion":[motion],
      "usage":["tutorial","sistemas Linux","documental técnico"],"editable":{}})
assert len(recorded)==29 and all(x["id"]!="debian" for x in recorded)
registry["presets"]=baseline+recorded
registry["total"]=len(registry["presets"])
registry["duration"]=sum(float(x.get("duration",2.5)) for x in registry["presets"])
registry["excluded_linux_modules"]=[{"number":26,"label":"Debian Red Swirl","reason":"Omitido por petición del usuario"}]
catalog.write_text(json.dumps(registry,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print("SOURCE",hashlib.sha256(data.encode()).hexdigest(),"CREATED",len(recorded),"TOTAL",len(registry["presets"]),"EXCLUDED",26)
