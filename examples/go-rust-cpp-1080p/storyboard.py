#!/usr/bin/env python3
"""Guion y storyboards del documental. Sin indicadores de página."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parent
scenes=[
 dict(d=16,k="EL GRAN DEBATE",title="GO  /  RUST  /  C++",subtitle="¿QUIÉN ES EL REY DEL RENDIMIENTO?",style="hero",asset="racks",accent="#4FE3DA",
 narration="Tres lenguajes, tres formas de perseguir la velocidad. Go simplifica construir servidores. Rust protege la memoria sin un recolector de basura. C más más ofrece control extremo y décadas de optimización. ¿Cuál gana? La respuesta depende de qué problema queremos resolver."),
 dict(d=18,k="EL CONTEXTO",title="LA VELOCIDAD NO ES UN NÚMERO",subtitle="LATENCIA · THROUGHPUT · MEMORIA · DESARROLLO",style="metrics",asset="code",accent="#7D9DFF",
 narration="Un programa rápido puede procesar muchas peticiones por segundo, responder con poca latencia, usar poca memoria o permitir desplegar cambios rápidamente. Ningún lenguaje domina todas esas métricas. La arquitectura, los algoritmos y la calidad de implementación suelen importar más que un ranking aislado."),
 dict(d=18,k="LENGUAJE · GO",title="GO: SIMPLICIDAD QUE ESCALA",subtitle="SERVICIOS, REDES Y EQUIPOS PRODUCTIVOS",style="language",asset="go-logo",accent="#53DFFF",
 narration="Go nació para hacer más sencillo construir software concurrente y servicios de red. Su sintaxis es pequeña, compila con rapidez y facilita generar binarios desplegables. Las goroutines permiten expresar miles de tareas concurrentes con una experiencia bastante directa para los equipos."),
 dict(d=18,k="PUNTOS FUERTES",title="GO BRILLA EN BACKEND",subtitle="GOROUTINES · CANALES · OPERACIÓN SENCILLA",style="benefits",asset="server",accent="#53DFFF",
 narration="Entre las ventajas de Go destacan la legibilidad, herramientas integradas, bibliotecas para servicios HTTP y una concurrencia cómoda. Por eso aparece con frecuencia en infraestructura, microservicios y herramientas de nube. Es muy efectivo cuando la claridad de operación importa tanto como la velocidad."),
 dict(d=16,k="LA OTRA CARA",title="EL COSTO DE LA COMODIDAD",subtitle="GARBAGE COLLECTION · AJUSTE FINO · MEMORIA",style="drawbacks",asset="css",accent="#53DFFF",
 narration="Pero Go utiliza recolección de basura. Aunque su recolector ha mejorado, puede consumir memoria y afectar la previsibilidad en determinadas cargas. Para control exhaustivo de asignaciones, latencias extremadamente estrictas o acceso al hardware, Rust y C más más suelen ofrecer más posibilidades."),
 dict(d=18,k="LENGUAJE · RUST",title="RUST: VELOCIDAD CON GARANTÍAS",subtitle="OWNERSHIP · BORROW CHECKER · SIN GC",style="language",asset="rust-logo",accent="#F3B27B",
 narration="Rust apuesta por seguridad de memoria con reglas verificadas en compilación. Ownership y borrowing permiten evitar muchas clases de fallos sin depender de un recolector en tiempo de ejecución. Su rendimiento puede situarse cerca de C más más cuando ambos programas están igualmente bien optimizados."),
 dict(d=18,k="PUNTOS FUERTES",title="MEMORIA MÁS SEGURA",subtitle="THREAD SAFETY · ABSTRACCIONES DE COSTO BAJO",style="benefits",asset="code",accent="#F3B27B",
 narration="Rust ayuda a prevenir usos después de liberar memoria, referencias inválidas y varias condiciones de carrera desde el sistema de tipos. Esto lo vuelve atractivo para sistemas nuevos, componentes críticos y servicios de alto rendimiento. El código inseguro existe, pero debe marcarse explícitamente."),
 dict(d=16,k="LA OTRA CARA",title="SEGURIDAD QUE EXIGE APRENDER",subtitle="CURVA INICIAL · COMPILACIÓN · COMPLEJIDAD",style="drawbacks",asset="racks",accent="#F3B27B",
 narration="Ese nivel de control tiene un precio. Los conceptos de ownership, lifetimes y tipos pueden resultar difíciles al principio. Algunos proyectos también requieren compilaciones largas, y migrar bibliotecas existentes no siempre es inmediato. Rust gana robustez, pero exige inversión en aprendizaje."),
 dict(d=18,k="LENGUAJE · C++",title="C++: CONTROL SIN CONCESIONES",subtitle="MOTORES · SISTEMAS · TIEMPO REAL",style="language",asset="cpp-logo",accent="#A697FF",
 narration="C más más es una herramienta central en motores de videojuegos, aplicaciones exigentes, bibliotecas científicas y software que necesita acercarse mucho al hardware. Permite controlar recursos, memoria y representación de datos. Su ecosistema es enorme y lleva décadas optimizándose en múltiples plataformas."),
 dict(d=18,k="POTENCIA Y RIESGOS",title="EL PRECIO DEL CONTROL TOTAL",subtitle="RENDIMIENTO · COMPLEJIDAD · SEGURIDAD",style="cpptrade",asset="server",accent="#A697FF",
 narration="Cuando se necesita interoperar con grandes bases de código, controlar el rendimiento con precisión o aprovechar bibliotecas maduras, C más más ofrece ventajas enormes. Sin embargo, su complejidad permite errores de memoria, comportamiento indefinido y dificultades de mantenimiento si las prácticas del equipo no son rigurosas."),
 dict(d=28,k="COMPARATIVA",title="LA TABLA DEFINITIVA",subtitle="TENDENCIAS GENERALES, NO BENCHMARKS UNIVERSALES",style="table",asset="",accent="#5DE9CE",
 narration="Pongámoslo lado a lado. Los tres pueden producir software muy rápido. C más más entrega libertad máxima para controlar recursos. Rust combina alto rendimiento con comprobaciones estrictas de memoria. Go prioriza simplicidad y productividad, especialmente en servidores. En seguridad de memoria por defecto destaca Rust; en velocidad de desarrollo para backend suele destacar Go. La mejor elección siempre depende de la carga concreta."),
 dict(d=20,k="ELECCIÓN PRÁCTICA",title="¿CUÁL USARÍAS MAÑANA?",subtitle="ELIGE SEGÚN TU PROBLEMA REAL",style="scenarios",asset="code",accent="#FFC870",
 narration="Si construyes microservicios y necesitas iterar rápido, empieza evaluando Go. Si diseñas nuevos componentes críticos donde seguridad y rendimiento importan, Rust es un candidato muy sólido. Si trabajas con motores gráficos, sistemas heredados o bibliotecas C más más, aprovechar ese ecosistema puede ser la decisión inteligente."),
 dict(d=21,k="VEREDICTO",title="EL REY NO ES UNIVERSAL",subtitle="RUST GANA EN EQUILIBRIO; C++ EN CONTROL; GO EN PRODUCTIVIDAD",style="verdict",asset="racks",accent="#64FFD6",
 narration="Mi veredicto: Rust se lleva la corona del equilibrio moderno entre rendimiento y seguridad de memoria. C más más sigue siendo un gigante del control y del ecosistema de alto rendimiento. Go brilla cuando importa entregar servicios fiables con un equipo productivo. El verdadero rey es la herramienta adecuada para el trabajo.")
]
for i,s in enumerate(scenes):
    s["index"]=i
    s["start"]=sum(z["d"] for z in scenes[:i])
    (ROOT/"text").mkdir(exist_ok=True)
    (ROOT/"text"/("narracion-%02d.txt"%(i+1))).write_text(s["narration"]+"\n",encoding="utf-8")
(ROOT/"storyboard.json").write_text(json.dumps({"width":1920,"height":1080,"fps":24,"duration":sum(x["d"] for x in scenes),"scenes":scenes},ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
print("Total:",sum(x["d"] for x in scenes),"seconds /",round(sum(x["d"] for x in scenes)/60,2),"min; words:",sum(len(x["narration"].split()) for x in scenes))
