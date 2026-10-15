#!/usr/bin/env python3
"""Export separate transparent text/cards/photo motion layers with Chromium.
Unlike the original static screenshots, the components can be animated
independently by FFmpeg's per-frame alpha, position and parallax.
"""
import json,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parent
DOC=json.loads((ROOT/"storyboard.json").read_text())
FRAMES=ROOT/"motion-layers"
FRAMES.mkdir(exist_ok=True)
GROUPS={
 "hero":[".big-hero",".hero-icons"],
 "metrics":[".section-banner .photo",".bar-copy"],
 "language":[".language-text .hint,.language-text h2,.language-text .section-sub",".lang-tags",".logo-orbit"],
 "benefits":[".cards-section h2,.cards-section .section-sub",".benefit-cards .feature-chip:nth-child(1)",".benefit-cards .feature-chip:nth-child(2)",".benefit-cards .feature-chip:nth-child(3)",".photo-shape"],
 "drawbacks":[".risk-image",".risk-copy .hint,.risk-copy h2",".risk-cards"],
 "cpptrade":[".trade-copy h2,.trade-copy .section-sub",".trade-chips",".trade-image"],
 "table":[".table-kicker,.comp-table .theads", ".comp-table .tr:nth-child(2),.comp-table .tr:nth-child(3)", ".comp-table .tr:nth-child(4),.comp-table .tr:nth-child(5)", ".comp-table .tr:nth-child(6),.comp-table .tr:nth-child(7),.comp-table .tr:nth-child(8)",".table-note"],
 "scenarios":[".case-layout h2,.case-layout .hint",".case-list",".case-photo"],
 "verdict":[".winner .hint,.winner h2",".winner>p:not(.hint)",".win-columns",".subscribe-cta"]
}
def capture(html_file,save):
    subprocess.run(["chromium","--headless","--no-sandbox","--disable-dev-shm-usage","--disable-gpu",
      "--hide-scrollbars","--force-device-scale-factor=1","--window-size=1920,1080",
      "--default-background-color=00000000","--screenshot="+str(save),
      "file://"+str(html_file)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=30)
    if not save.is_file() or save.stat().st_size<1000:raise RuntimeError("Screenshot fallo: "+str(save))
for scene in DOC["scenes"]:
    i=scene["index"]+1
    original=(ROOT/"slides"/f"scene-{i:02d}.html").read_text()
    # Text/imagery on background remain unaltered; remove every content
    # component from background to make all content separately animated.
    back=original.replace("</style>",".content{visibility:hidden!important}</style>")
    source=FRAMES/f"temp-{i:02d}.html"
    source.write_text(back)
    capture(source,FRAMES/f"bg-{i:02d}.png")
    layer_base="""html,body{background:transparent!important}
.stage{background:transparent!important}
.stage::before,.stage::after,.topbar,footer{display:none!important}
.content *{visibility:hidden!important}
"""
    selectors=GROUPS[scene["style"]]
    for j,selector in enumerate(selectors,1):
        selected=[]
        for item in selector.split(","):
            item=item.strip()
            selected.append(".content "+item)
            selected.append(".content "+item+" *")
        css=layer_base+",".join(selected)+"{visibility:visible!important}"
        source.write_text(original.replace("</style>",css+"</style>"))
        capture(source,FRAMES/f"layer-{i:02d}-{j}.png")
    source.unlink()
    print(f"ANIMATED_SCENE_ASSETS {i:02d}/{len(DOC['scenes'])} layers={len(selectors)}",flush=True)
(FRAMES/"manifest.json").write_text(json.dumps({"groups":GROUPS,"number_of_scenes":len(DOC["scenes"])},indent=2))
print("ALL_MOTION_LAYERS_COMPLETE",flush=True)
