#!/usr/bin/env python3
"""Regression checks for the 30 browser-first educational assets."""
from pathlib import Path
import json,collections,re

R=Path(__file__).resolve().parent
j=json.loads((R/"catalogo.json").read_text())
assert j["total"]==95==len(j["presets"])
ids=[v["id"] for v in j["presets"]]
assert len(set(ids))==95,"Duplicate animation IDs"
assert [v["number"] for v in j["presets"]]==list(range(1,96))
latest=j["presets"][65:]
assert len(latest)==30
expected={
 "architecture-06":("Arquitectura de software","catalogo_architecture_06_08.html"),
 "runtime-07":("Programación y ejecución","catalogo_runtime_06_08.html"),
 "languages-08":("Lenguajes de programación","catalogo_languages_06_08.html"),
}
assert sum(v["states"] for v in latest)==158
for batch,(cat,gallery) in expected.items():
 subset=[v for v in latest if v["batch"]==batch]
 assert len(subset)==10,cat
 assert all(v["category"]==cat for v in subset)
 page=(R/gallery).read_text()
 assert 'data-segment-seconds="4"' in page
 assert page.count('<section class="module"')==10,gallery
 assert page.count('@keyframes show_module_')==10
 assert "script-src &#39;none&#39;" in page and "<script" not in page.lower()
 assert len(page.encode())<1024*1024
for v in latest:
 slug=v["id"]
 assert re.fullmatch(r"[a-z0-9-]+",slug) and len(slug)<=64
 assert v["source_module_id"] and v["source_collection"]
 source=R.parent/v["source_collection"]
 assert source.is_file(),v
 file=R.parent/v["preset_file"]
 assert file.is_file()
 data=file.read_text()
 assert "<script" not in data.lower()
 assert 'default-src &#39;none&#39;' in data
 assert "script-src &#39;none&#39;" in data
 assert len(data.encode())<1024*1024
 assert data.count('class="phase phase-')==v["states"]
 assert all(field in v["editable"] for field in ("title","description","file"))
 assert len(v["tags"])>=3
print("OK",len(latest),"new presets;",sum(v["states"] for v in latest),"states;",len(ids),"total")
print("CATEGORIES",dict(collections.Counter(v["category"] for v in latest)))
