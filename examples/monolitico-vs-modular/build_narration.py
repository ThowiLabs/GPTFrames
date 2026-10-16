#!/usr/bin/env python3
"""Preparation only: narration is synthesized by compiled Neo Lite."""
from pathlib import Path
import json
R=Path(__file__).resolve().parent
R.mkdir(parents=True,exist_ok=True)
sentences=[
"¿Qué cambia entre código monolítico y modular? Imagina una tienda en Python. Las dos versiones funcionan, pero se organizan distinto.",
"En este ejemplo de monolito poco modular, una misma función cobra, actualiza el inventario y envía una notificación. Toda la lógica está mezclada.",
"Al modificar el cobro, puedes afectar otras partes. También resulta difícil probar cada responsabilidad sin ejecutar la función completa.",
"En el diseño modular, pagos, inventario y notificaciones viven en archivos separados. Cada módulo tiene una tarea bien definida.",
"El archivo principal solo los coordina. Ahora podemos probar pagos por separado y reutilizar inventario sin duplicar el código.",
"Modular no significa microservicios. Estos módulos pueden vivir en un mismo proceso y desplegarse juntos, sin llamadas por red.",
"La clave no es hacer más archivos por hacerlo. Es separar responsabilidades cuando eso vuelve el código más claro y fácil de mantener."
]
meta=[]
for i,line in enumerate(sentences):
 p=R/f"voice-{i:02d}.txt"
 p.write_text(line+"\n",encoding="utf-8")
 meta.append({"scene":i,"text":line,"duration":8})
(R/"narracion.json").write_text(json.dumps(meta,ensure_ascii=False,indent=2)+"\n")
print("CREATED",len(meta),"SCENES",sum(x["duration"] for x in meta),"SECONDS")
