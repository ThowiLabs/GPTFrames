#!/usr/bin/env python3
"""Review a GPTFrames native JSON video project from actual rendered frames.

Authoring/QA only: no Python runtime dependency inside GPTFrames C++.
Works with standard library + native engine; FFmpeg is OPTIONAL for contact sheet.
Does not modify the input project or user media.
"""
import argparse
import json
import math
import shutil
import subprocess
import sys
from collections import Counter
from pathlib import Path

CRITICAL = {"image_cropped", "image_occluded", "image_distorted", "outside_canvas", "text_overflow", "text_collision"}
def run(cmd, timeout=120):
    cp = subprocess.run([str(x) for x in cmd], text=True, capture_output=True, timeout=timeout)
    if cp.returncode != 0:
        raise RuntimeError(f"Command failed ({cp.returncode}): {str(cmd[:4])}: {cp.stderr[-700:]}")
    return cp.stdout.strip()
def main():
    p = argparse.ArgumentParser(description="Render every scene's start, middle and end for editorial QA.")
    p.add_argument("project", type=Path)
    p.add_argument("--engine", type=Path, default=Path("build/native-full/hyperframes-cpp-native"))
    p.add_argument("--out", type=Path, default=Path("/tmp/gptframes-review"))
    p.add_argument("--strict", action="store_true", help="Exit 2 when any critical layout alert is detected.")
    p.add_argument("--limit-scenes", type=int, default=0, help="Only QA this many scenes (quick tests).")
    p.add_argument("--no-frames", action="store_true", help="Static lint report only.")
    opts = p.parse_args()
    project = opts.project.resolve(strict=True)
    engine = opts.engine.resolve(strict=True)
    output = opts.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    source = json.loads(project.read_text(encoding="utf-8"))
    if not isinstance(source.get("scenes"), list) or not source["scenes"]:
        raise RuntimeError("Expected GPTFrames native JSON with scenes")
    run([engine, "--project", "validate", project])
    warnings = json.loads(run([engine, "--project", "lint", project]))
    problems = warnings.get("warnings", [])
    severe = [v for v in problems if v.get("code") in CRITICAL]
    report = {
        "project": str(project),
        "dimensions": source.get("canvas", {}),
        "fps": source.get("fps", 24),
        "scenes": len(source["scenes"]),
        "linter_warnings": len(problems),
        "critical": len(severe),
        "warning_counts": dict(Counter(v.get("code", "unknown") for v in problems)),
        "critical_items": severe,
        "frames": [],
        "review_complete": False,
        "human_approval_required": True
    }
    frames = output / "frames"
    if not opts.no_frames:
        frames.mkdir(parents=True, exist_ok=True)
        active = source["scenes"][:(opts.limit_scenes or len(source["scenes"]))]
        for i, scene in enumerate(active, start=1):
            start, duration = float(scene["start"]), float(scene["duration"])
            assert math.isfinite(start) and math.isfinite(duration) and duration > 0
            for phase_index, (name, fraction) in enumerate([("early", .14), ("middle", .52), ("late", .85)]):
                moment = start + duration * fraction
                image = frames / f"scene-{i:03d}-{phase_index:02d}-{name}.png"
                run([engine, "--project", "frame", project, image, f"{moment:.4f}"], timeout=150)
                if not image.is_file() or image.stat().st_size < 1000:
                    raise RuntimeError(f"Missing/empty rendered frame: {image}")
                report["frames"].append({"scene": i, "time": round(moment, 3), "phase": name, "image": str(image)})
            print(f"REVIEW_SCENE {i}/{len(active)} {scene['start']:.2f}s", flush=True)
        ffmpeg = shutil.which("ffmpeg")
        if ffmpeg and report["frames"]:
            # The letterboxed contact sheet is only for review; it does not modify final video.
            count = len(report["frames"])
            rows = int(math.ceil(count / 5))
            contact = output / "contact-sheet.jpg"
            cmd = [ffmpeg, "-nostdin", "-v", "error", "-y", "-framerate", "1",
                   "-pattern_type", "glob", "-i", str(frames / "scene-*.png"),
                   "-vf", f"scale=208:-2:flags=lanczos,tile=5x{rows}:padding=4:margin=5:color=black",
                   "-frames:v", "1", str(contact)]
            try:
                run(cmd, timeout=120)
                report["contact_sheet"] = str(contact)
            except Exception as exc:
                report["contact_sheet_error"] = str(exc)[:250]
    report["review_complete"] = bool(opts.no_frames or len(report["frames"]) == 3 * min(opts.limit_scenes or len(source["scenes"]), len(source["scenes"])))
    (output / "report.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({
        "warnings": report["linter_warnings"],
        "critical": report["critical"],
        "types": report["warning_counts"],
        "rendered_frames": len(report["frames"]),
        "report": str(output / "report.json"),
        "contact_sheet": report.get("contact_sheet")
    }, ensure_ascii=False, indent=2))
    if opts.strict and severe:
        print("BLOCKED: critical layout warnings must be addressed.", file=sys.stderr)
        return 2
    return 0
if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as exc:
        print(f"QA_ERROR: {exc}", file=sys.stderr)
        sys.exit(1)
