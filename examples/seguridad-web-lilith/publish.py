#!/usr/bin/env python3
"""Publish ONLY finished video, narration and public metadata; no model weights."""
from pathlib import Path
import hashlib,json,shutil,subprocess
R=Path(__file__).resolve().parent
D=json.loads((R/"storyboard.json").read_text())
public=R.parent/"go-rust-cpp-1080p"/"public"/"seguridad-web-lilith"
public.mkdir(parents=True,exist_ok=True)
mp4=R/"output"/"Vulnerabilidades-Web-Lilith-1080p-30fps.mp4"
if not mp4.is_file():raise RuntimeError("MP4 todavía no está listo")
probe=json.loads(subprocess.check_output(["ffprobe","-v","error","-show_entries","stream=codec_name,codec_type,width,height,avg_frame_rate,nb_frames","-show_entries","format=duration,size","-of","json",str(mp4)]))
video=next(s for s in probe["streams"] if s["codec_type"]=="video")
audio=next(s for s in probe["streams"] if s["codec_type"]=="audio")
assert video["width"]==1920 and video["height"]==1080
assert video["avg_frame_rate"]=="30/1" and int(video["nb_frames"])==D["duration"]*30
assert float(probe["format"]["duration"])==D["duration"]
assert video["codec_name"]=="h264" and audio["codec_name"]=="aac"
shutil.copy2(mp4,public/mp4.name)
shutil.copy2(R/"output/lilith-voiceonly.wav",public/"lilith-voiceonly.wav")
names=[]
for sc in D["scenes"]:
    names.append({"start":sc["start"],"label":sc["title"].replace("\n"," / ").capitalize()})
source=(R/"index.template.html").read_text().replace("__CHAPTERS_JSON__",json.dumps(names,ensure_ascii=False))
(public/"index.html").write_text(source,encoding="utf8")
subprocess.run(["ffmpeg","-nostdin","-hide_banner","-loglevel","error","-y",
"-i",str(R/"output/preview-fast.png"),"-vf","scale=1280:720","-q:v","3",str(public/"poster.jpg")],check=True)
sources={
"title":D["title"],"content_sources":[
 {"title":"OWASP Top 10:2025","url":"https://top10.owasp.org/2025/","license":"Contenido fuente; citar, no reproducido textualmente"},
 {"title":"OWASP Broken Access Control","url":"https://top10.owasp.org/2025/A01_2025-Broken_Access_Control/"},
 {"title":"OWASP XSS Prevention Cheat Sheet","url":"https://cheatsheetseries.owasp.org/cheatsheets/Cross_Site_Scripting_Prevention_Cheat_Sheet.html"},
 {"title":"OWASP SQL Injection Prevention Cheat Sheet","url":"https://cheatsheetseries.owasp.org/cheatsheets/SQL_Injection_Prevention_Cheat_Sheet.html"},
 {"title":"OWASP Secure Code Review Cheat Sheet","url":"https://cheatsheetseries.owasp.org/cheatsheets/Secure_Code_Review_Cheat_Sheet.html"},
 {"title":"OWASP Database Security Cheat Sheet","url":"https://cheatsheetseries.owasp.org/cheatsheets/Database_Security_Cheat_Sheet.html"}],
 "voice":{"name":"Lilith","author":"HirCoir","source":"https://huggingface.co/HirCoir/es_MX-lilith2","revision":"8d7ca9584a4019a13f4a661d27951b93c9fdea36","declared_license":"Apache-2.0","format":"NEO Lite compiled locally from ONNX","public_model_download":False},
 "source_archive_note":"El archivo ZIP privado solicitado no pudo verificarse; se utilizó la distribución Lilith del autor en Hugging Face. No se afirma identidad binaria con los pesos de YahirHub.",
 "graphics":"Diagramas propios originales diseñados para este documental","purpose":"Divulgación y prevención, sin procedimientos operativos de explotación",
 "duration_seconds":D["duration"],"fps":30,"output_resolution":"1920x1080","intermediate_compositing":"1280x720 por límites de memoria CPU; escalado a 1920x1080 en H.264",
}
(public/"fuentes-y-licencias.json").write_text(json.dumps(sources,indent=2,ensure_ascii=False)+"\n",encoding="utf8")
print("DOCUMENTARY_PUBLISHED",public, "frames",video["nb_frames"],"bytes",mp4.stat().st_size,flush=True)
