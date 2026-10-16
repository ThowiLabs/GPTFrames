---
name: hyperframes-cpp
description: "Produce high-quality narrated GPTFrames videos with native C++ compositor, Chromium CSS/SVG, searchable presets, voice-synchronized explanatory motion, Neo voice and mandatory audiovisual QA; avoid clipped images and text."
---

# GPTFrames — professional video production agent skill

**Mission:** Create technically accurate, attractive, clear educational videos about programming, Linux, software architecture, networking and other computing topics. Success is a visually reviewed video, not a successful MP4 export.

## Mandatory editing standard: build the explanation with the voice

Read **references/NARRATED_MOTION.md** before storyboarding any narrated video. Apply it to every new video and revision unless the user explicitly requests static treatment. Reveal and transform individual semantic elements at the words/phrases that explain them. A complete diagram shown from the start, decorative loops, title fades or a single animation per slide do **not** meet this standard.

Write a beat map and an editable `motion_events.json` with element IDs, spoken anchors, scene-local times and timing provenance. Build meaningful sequences: tasks travel to units, a geometry becomes an image, operands enter in order, the result appears when spoken. Estimate word times from the real audio, then review audiovisual sync; do not claim phoneme accuracy from an energy aligner.

Search and reuse relevant assets, but adapt/register a reviewed variant or author a compatible deterministic HTML/CSS/SVG sequence when existing timing cannot match the narration. Preserve originals. Use CLI/source control when needed; arbitrary HTML is not accepted by MCP `save_project`. Ask for a missing essential animation only after checking viable existing/code alternatives.

Block delivery for premature results, static completed diagrams during enumeration, filler loops, cue failures, overlapping captions or unexplained dead segments. Inspect actual frames around beats, watch/listen to encoded output and full-decode the finished MP4. The agent performs this review under the user's production request; do not invent a separate user-approval requirement.

## Animated data charts

Read **references/ANIMATED_CHARTS.md** when using graphs. The reviewed `chart-bars-vertical` supports colored vertical bars, editable labels and an optional contain-fitted image. Generate numerical SVG geometry from data with `tools/generate_bar_chart.py`; changing a text number does not recalculate bars. Register variants, synchronize categories, bar growth, values and computed conclusion to measured voice, and verify both image layouts. Illustrative data must stay labeled. Only declared text/image fields are live MCP properties.

## Discover your actual tools BEFORE designing

1. Locate compiled engine (for this repo: `build/native-full/hyperframes-cpp-native`). Verify options with `--browser-check` and project documentation; older builds may not support newer features.
2. With GPTFrames MCP call `resource_status`, `list_animation_facets`, `search_animations`, `list_voice_models` and `describe_animation` to discover current animations, categories, code/CSS motion tags and editable properties. Resource discovery refreshes automatically, or use `rescan_resources`; do not assume an asset is available without verifying it.
3. Prefer verified existing presets over reinventing visuals. The catalog in `animaciones/catalogo.json` and original resources in `assets/preset-catalog.json` are the truth. A detected-but-unreviewed HTML is not executable.
4. Pick format: **848x480/24fps** for landscape review, **720x1280/24fps** for shorts, or 1920x1080 full-HD source. Keep source and output aspect ratio equal.

**MCP tools (23 in current code; always discover schema):** resources: `resource_status`, `rescan_resources`, `search_animations`, `list_animation_facets`, `list_animations`, `describe_animation`, `list_asset_presets`, `describe_asset_preset`, `list_voice_models`; projects: `list_projects`, `create_project`, `read_project`, `save_project`, `validate_project`, `review_layout`; media: `synthesize_voice`, `search_images`, `download_commons_image`; render: `preview_frame`, `render_video`, `start_render`, `render_status`, `cancel_render`.

**Engine capabilities:** native JSON scenes with rectangles, groups/auto-layout, animations/keyframes, lines, code/text, grids, gradients, cameras, images with borders/masks, still chroma key, primitive symbols and optionally ThorVG Lottie. Browser-first registered `html_widget` presets use self-contained HTML/CSS/SVG and frame-seekable animations via Chromium/CDP; editable fields only where the preset catalog declares them. No arbitrary MP4/WebM video layer in native JSON, no direct MP3/M4A import for native mixer, no guarantee of arbitrary HTML/JS compatibility. The Neo Lite executable generates speech from installed `.neo`/`.onnx` voices to WAV. Use the actual live `list_voice_models` inventory rather than hardcoding model names.

Read **references/WORKFLOWS.md** for implementation, **references/QUALITY_GATE.md** for visual acceptance, **references/ASSET_INVENTORY.md** for tool and catalog specifics. Project documentation: `docs/JSON_COMPOSITOR.md`, `docs/VIDEO_AUTHORING_PROMPT.md`, `docs/VOICE_SYNC_V5.md`, `docs/MOTION_STUDIO.md`.

## Required production workflow

**1. Editorial storyboard.** Write truthful narration FIRST. Each scene expresses one specific concept; include a meaningful diagram, code snippet, photograph, visualization or real example rather than repetitive empty cards. A professional short has a hook, working explanation, concrete example, comparison and takeaway. Keep visible code technically correct. Specify what viewers must see and hear at each second.

**2. Voice and timing.** Generate speech **per scene** using `synthesize_voice` or native Neo Lite. Measure real WAV durations, verify technical pronunciation and accents by listening, allow ~0.4–0.8 seconds breathing room. Keep on-screen code strings exact, spoken phonetics separate (e.g. C++ spoken "ce más más"). Use `speechTiming` and `cueIndex` when available, remembering automatic alignments are approximate. Never show empty cards before their captions or reveal concepts before they're spoken. Enter caption/icon/card as **one atomic group**.

**3. Choose assets deliberately and fit them to the beat map.** Search the catalog (motion tags + category + description) before drawing primitive symbols. Use licensed photos with source/provenance; prefer self-contained SVG/HTML diagrams. Avoid primitive emojis in a technical documentary.

**4. Strict image framing.** Native `image.fit` accepts:
- `contain`: **default**, preserves the whole image and aspect ratio. Use for logos, code screenshots, technology icons, UI screenshots, charts, diagrams and photo subjects that must remain complete.
- `cover`: intentionally crops to fill a box; use for true photographic backgrounds only, after checking the actual crop. Never crop faces, text, logos, devices or a significant section of a machine.
- `stretch`: distorts the image; almost never acceptable.
- Split photos and text into their own spatial regions. Never draw an opaque rectangle over most of an earlier photo. Use clear margins: roughly 6–8% in vertical content. A Docker logo 682x163 inside a 260x112 box loses substantial visual area with `cover`.
- Frame image at its true source aspect ratio, verify legibility and dimensions with actual rendered pixels, not just intended JSON geometry.

**5. Composition rules.** Maintain consistent spacing and typography, readable text on phone screens, meaningful use of cyan/green/violet highlights and semantic motion. Avoid walls of text, 7pt labels, random borders, idle 2-second pauses, excessive shaking and badly sized art. Aim for a focal point per scene. If image and text cannot fit, simplify or split the scene; do not shrink until the linter is silent. Keep visual information within safe bounds.

**6. Validate and preview EVERY scene.**
```bash
ENGINE=build/native-full/hyperframes-cpp-native
"$ENGINE" --project validate /path/to/project.json
"$ENGINE" --project lint /path/to/project.json
python3 tools/qa_video_project.py /path/to/project.json --engine "$ENGINE" --out /tmp/gptframes-qa
```
QA makes scene screenshots and a contact sheet and saves the raw lint report. **Fix** `image_cropped`, `image_occluded`, `image_distorted`, `text_overflow`, `text_collision` and `outside_canvas` before publishing; investigate `small_text` even if there are no hard failures. Check early/mid/late frames and the instant before/after each narrated cue. Automatic lint cannot detect all bad compositions—human vision is mandatory.

**7. Preview first, render only after visual review.**
```bash
"$ENGINE" --project frame /path/to/project.json /tmp/check.png 3.5
"$ENGINE" --project render /path/to/project.json /tmp/preview.mp4 --size 848x480
```
The 848x480 preview applies to matching **landscape aspect ratio** projects; portrait projects need an appropriately proportioned size (or native 720x1280). Preserve codec dimensions constraints (width multiple of 16, height even). `--browser-widget-render` is for SHORT CSS demos, presently <=10 seconds; don't use it for long documentaries. For full multi-scene projects use native JSON and registered browser widgets, or the engine's supported full HTML route.

**8. Finish and verify.** Wait for renderer exit zero. Probe streams with FFprobe and full-decode via FFmpeg for QA (not production runtime). Listen to actual speech, check every frame cluster, inspect the final MP4—not only scene PNGs. Provide verified link, dimensions, FPS, duration, audio status and editability. Never call a bad video "professional" merely because tests passed. Deliver the authorized finished revision. Ask for feedback only after delivery; do not add a routine permission stop.

## Problems demonstrated by Alpine Linux example

- Default `cover` cut informative photos/logos; now `contain` by default in the engine.
- Scene 3 drew an opaque rectangle **after** its rack photograph, obscuring >55% of the image. The new linter flags `image_occluded`.
- Scene 6 compressed a datacenter photo into 385x145 on a 720x1280 canvas: visually insignificant despite being technically within bounds.
- Tiny text, low visual density and visual timing inconsistencies need editorial fixes, not only code fixes.

Use `references/QUALITY_GATE.md` and `tools/qa_video_project.py` on every video.

## Security and delivery limits

Do not execute untrusted JS in Chromium running as root. Do not import arbitrary URLs or CDN scripts; browser presets must be reviewed, CSP-constrained, locally hosted and deterministic. Keep source licenses and media permissions. User project/media/voice weights stay outside Git source commits; never expose auth secrets. Don't restart the connected MCP for asset changes or change the main branch without an explicit request.
