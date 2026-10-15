#!/usr/bin/env python3
"""Download editorial assets via Wikimedia's documented Special:FilePath endpoint."""
from pathlib import Path
from urllib.parse import quote
from urllib.request import Request, urlopen
import json, time
ROOT=Path(__file__).resolve().parent
DEST=ROOT/"assets"
DEST.mkdir(exist_ok=True)
ITEMS=[
 ("go-logo","Go Logo Blue.svg","Public domain","The Go Authors"),
 ("rust-logo","Rust programming language black logo.svg","CC BY 4.0","Rust Foundation"),
 ("cpp-logo","ISO C++ Logo.svg","Public domain","ISO C++ logo contributors"),
 ("server","Datacenter Server Racks (22370909788).jpg","CC BY 2.0","Carl Lender"),
 ("processor","Southbridge VIA VT82C686A.jpg","CC BY-SA 3.0","MOS6502"),
 ("code","Code on computer monitor (Unsplash).jpg","CC0","Markus Spiske"),
 ("racks","Rear of rack at NERSC data center - closeup.jpg","CC0","Derrick Coetzee"),
 ("css","CSS code on a screen (Unsplash).jpg","CC0","Sai Kiran Anagani"),
]
records=[]
for name,title,license_,author in ITEMS:
    url="https://commons.wikimedia.org/wiki/Special:FilePath/"+quote(title,safe="")+"?width=1440"
    request=Request(url,headers={"User-Agent":"GPTFrames/1.0 (local editorial rendering)","Accept":"image/*"})
    try:
        with urlopen(request, timeout=40) as stream:
            raw=stream.read(18*1024*1024+1)
            mime=stream.headers.get("Content-Type","")
        if len(raw)>18*1024*1024:raise ValueError("asset >18MB")
        if raw.startswith(b"\x89PNG\r\n\x1a\n"):ext="png"
        elif raw.startswith(b"\xff\xd8\xff"):ext="jpg"
        else:raise ValueError("unrecognized raster signature "+mime)
        file=DEST/(name+"."+ext)
        file.write_bytes(raw)
        records.append({"key":name,"file":"assets/"+file.name,"source_title":title,"source_url":"https://commons.wikimedia.org/wiki/File:"+quote(title.replace(" ","_")),"license":license_,"author":author})
        print(name,ext,len(raw))
    except Exception as e:
        print("FAIL",name,type(e).__name__,e)
    time.sleep(.6)
(ROOT/"assets-sources.json").write_text(json.dumps(records,ensure_ascii=False,indent=2)+"\n")
print("Downloaded",len(records),"assets")
