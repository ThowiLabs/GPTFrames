#pragma once
// Standalone render viewer UI. Served by native httplib; zero web runtime deps.
#include <string>

namespace gptframes::mcp {
inline const char* viewer_html(){
 return R"GPTF(<!doctype html><html lang="es"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>GPTFrames · Render</title>
<style>
:root{color-scheme:dark;font:16px system-ui,-apple-system,"Segoe UI",sans-serif;background:#07111c;color:#edf5fa}
*{box-sizing:border-box}body{margin:0}header{border-bottom:1px solid #294056;padding:18px max(22px,calc((100vw - 1050px)/2))}
main{max-width:1050px;margin:36px auto;padding:0 22px 48px}
small,.muted{color:#abc0d2}h1{font-size:clamp(25px,5vw,42px);margin:6px 0 10px}
.panel{padding:24px;background:#111f2e;border:1px solid #30465c;border-radius:20px;margin-top:20px}
.preview{width:100%;aspect-ratio:16/9;border-radius:14px;background:linear-gradient(135deg,#162d45,#07111c);
display:flex;align-items:center;justify-content:center;overflow:hidden}
.preview video{width:100%;height:100%;object-fit:contain;display:none}
.track{height:13px;background:#34465a;border-radius:20px;overflow:hidden;margin:20px 0 9px}
#bar{background:linear-gradient(90deg,#50d8c2,#8ba5fc);width:0;height:100%;transition:width .4s ease}
.row{display:flex;align-items:center;justify-content:space-between;gap:12px;flex-wrap:wrap}
button,.button{appearance:none;border:0;border-radius:12px;display:inline-block;background:#5cdecb;color:#061721;
padding:12px 18px;font-weight:700;cursor:pointer;text-decoration:none;font:inherit}
button.secondary{background:#25384d;color:#e2eefa}
button:disabled{opacity:.5;cursor:not-allowed}
#videoPanel{display:none}#err{color:#ffb7bc;white-space:pre-wrap}
code{color:#b3d1e9;overflow-wrap:anywhere}
</style></head><body>
<header><strong>GPTFrames</strong> <small> / Render Studio</small></header>
<main><small>RENDER NATIVO · H.264 / AAC</small><h1 id="headline">Preparando visualizador</h1>
<p class="muted" id="subtitle">Conectando con el trabajo…</p>
<section class="panel"><div class="preview"><div id="placeholder" class="muted">Vista previa disponible al terminar</div>
<video id="video" controls playsinline preload="metadata"></video></div>
<div class="row" style="margin-top:22px"><strong id="status">Esperando autorización</strong><strong id="percentage">0%</strong></div>
<div class="track"><div id="bar"></div></div>
<div class="row"><small id="metadata">Estado protegido</small><button id="cancel" class="secondary" disabled>Cancelar render</button></div>
<p id="err" role="alert"></p>
<div id="videoPanel" class="row" style="margin-top:24px"><strong>¡Video terminado!</strong>
<a class="button" id="download" download="gptframes.mp4">Descargar MP4</a></div>
</section><p class="muted">Enlace de acceso privado, válido por 24 horas. No compartas la URL si el video contiene datos privados.</p></main>
<script>
(()=>{"use strict";
 const id=location.pathname.split("/").filter(Boolean).pop(),base="/renders/"+id;
 const el=x=>document.getElementById(x);
 let timer=null;
 const labels={queued:"En cola",running:"Renderizando",completed:"Completado",cancelled:"Cancelado",failed:"Falló",interrupted:"Interrumpido"};
 async function access(){
  const secret=new URLSearchParams(location.hash.slice(1)).get("access");
  if(secret){
   history.replaceState(null,"",location.pathname);
   const response=await fetch(base+"/session",{method:"POST",headers:{"Content-Type":"application/json","X-GPTFrames-Action":"session"},body:JSON.stringify({token:secret})});
   if(!response.ok)throw Error("El enlace ha caducado o no es válido.");
  }
  await refresh();
 }
 async function refresh(){
  try{
   const response=await fetch(base+"/status",{cache:"no-store",credentials:"same-origin"});
   if(!response.ok)throw Error(response.status===403?"No tienes acceso a este trabajo. Usa tu URL original.":"No fue posible consultar el estado.");
   const job=await response.json();
   const p=Math.min(100,Math.max(0,Number(job.progress)||0));
   el("headline").textContent=job.project||"Render";
   el("subtitle").textContent="Trabajo "+job.id;
   el("status").textContent=labels[job.status]||job.status;
   el("percentage").textContent=p.toFixed(0)+"%";
   el("bar").style.width=p+"%";
   el("metadata").textContent="Duración solicitada: "+(job.seconds>0?job.seconds+" segundos":"Completa")+" · Formato MP4";
   el("cancel").disabled=job.status!=="queued"&&job.status!=="running";
   el("err").textContent=job.error||"";
   if(job.ready){
    el("videoPanel").style.display="flex";
    el("placeholder").style.display="none";
    const video=el("video");video.style.display="block";
    if(!video.src)video.src=base+"/video";
    el("download").href=base+"/video?download=1";
   }
   if(["completed","failed","cancelled","interrupted"].includes(job.status)){clearInterval(timer);timer=null;}
   else if(!timer)timer=setInterval(refresh,1500);
  }catch(err){clearInterval(timer);timer=null;el("err").textContent=err.message;}
 }
 el("cancel").onclick=async()=>{
  if(!confirm("¿Cancelar este render?"))return;
  const r=await fetch(base+"/cancel",{method:"POST",headers:{"X-GPTFrames-Action":"cancel"}});
  if(!r.ok)el("err").textContent="No se pudo cancelar.";
  await refresh();
 };
 access().catch(e=>{el("err").textContent=e.message;el("headline").textContent="Acceso no disponible";});
})();
</script></body></html>)GPTF";
}
} // namespace gptframes::mcp
