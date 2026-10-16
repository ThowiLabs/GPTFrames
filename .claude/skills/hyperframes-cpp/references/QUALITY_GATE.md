# GPTFrames — visual quality gate (mandatory)

No rendering success, correct codec, or unit-test pass proves good composition. The video must be visually inspected before delivery, especially on a phone.

## Narrated motion is a delivery gate

Apply [NARRATED_MOTION.md](NARRATED_MOTION.md). Require a beat map and `motion_events.json` linking visible actions to spoken anchors. Reject complete diagrams shown before their explanation, premature results, decorative loops used as the explanation, and sequences whose only changes are title fades. Inspect cue boundaries and effective visibility inside browser widgets as well as native layers. Check actual movement and state changes, not merely different pixels.

Review the final encoded video with speech. Contact sheets and automatic alignment cannot certify audiovisual synchronization. If voice or cuts change, regenerate all dependent marks and subtitles. A blocker here requires fixing the montage, even when schema/lint/codec checks pass.

## Acceptance checklist

**Images**
- Source aspect ratio preserved. Use image `fit: contain` for all screenshots, logos, icons, educational diagrams and non-background pictures. `cover` is intentionally destructive; `stretch` distorts.
- No important text, face, icon, device, graph axis or logo is outside the frame.
- No opaque rectangle masks more than half a photograph. Put text below/next to image, or use deliberately small non-destructive translucent overlays.
- Photographic sources are large enough to actually convey subject matter. A photo taking a 385x145 strip on a vertical 720x1280 video is not a substantive illustration.
- Image compression/alpha/key color are visually checked at output resolution.
- Do not enlarge a tiny source until blurry. Choose better source or vector asset.

**Texts**
- Two clear typographic levels, visible on mobile; labels readable at normal playback size.
- Titles rarely more than two lines. Avoid truncated subtitles, overflow and cards whose content is clipped.
- Code typography must be big enough, with syntax highlights only on meaningful tokens.
- Never use a filled border panel with no message inside it, never hide text behind icons.
- Contrast tested against the actual panel, not the overall scene background.

**Story / motion**
- A coherent hook, setup, code/example, comparison and specific takeaway.
- Every visual points to something being explained; avoid repetitive animations as filler.
- Narrated elements appear when the corresponding words are spoken, not before.
- Cards, header, icon, image, border and text in one semantic unit share entrance cue.
- If six elements appear in a comparison, reveal them in order of explanation and reflow with the native layout engine or a deterministic CSS grid.
- All CSS seek paths deterministic; verify different timestamps actually differ.
- Title transitions and audio pauses should feel natural; no visually dead segment during spoken content.

**Technical accuracy**
- Names of frameworks/languages, Linux commands and API interfaces are exact on screen, even if spoken phonetically.
- A modular monolith is possible; deployment topology and code modularity are separate topics.
- Assets and images have valid rights for the planned publication.

## Severity and automatic tests

Run:
```bash
ENGINE=build/native-full/hyperframes-cpp-native
"$ENGINE" --project validate project.json
"$ENGINE" --project lint project.json > layout.json
python3 tools/qa_video_project.py project.json --engine "$ENGINE" --out ./qa
```

The QA tool emits `report.json`, `frames/*.png` and, if FFmpeg is installed, a reviewable `contact-sheet.jpg`. It samples start, middle and end of **each** scene. Inspect each frame at original resolution.

Treat these linter codes as blockers:
- `image_cropped`: >20% of source lost to `cover`.
- `image_distorted`: `stretch` deforms the source.
- `image_occluded`: a later opaque rectangle hides >55% of photo.
- `outside_canvas`: important element outside viewport.
- `text_overflow`: the text won't fit without shrinking.
- `text_collision`: labels overlap.

Investigate `small_text`, `small_box` and `low_contrast` warnings as medium severity; many false positives are possible during transitions or for decorative footers, so human review decides acceptance.

Use `--strict` in the QA tool to make blocker warnings fail the command. Fix source project, don't silence warnings by removing lint.

## Sampling

For scenes with duration D at absolute T, sample at T + 0.14D, T + 0.52D, and T + 0.85D (clamped to within scene). For a short reactive transition, add further samples just before and after the cue. Include scene number/time in filename; don't accidentally use the global video time for a local scene CSS widget.

For 24fps, record the actual FPS and verify no repeat/blank frames. Use `ffprobe` and full-stream decode as QA, not as required native dependencies.

## Failure example: Alpine/Docker vertical video

In the user-supplied sample (720x1280/24fps) the Docker logo has native ratio 682:163 (~4.18:1), while the box in scene 1 is 260:112 (~2.32:1). `cover` discards about 45% of the logo horizontally; `contain` keeps everything and letterboxes vertically. In scene 3 an opaque card covers the rack image. In scene 6 the server photograph is too small to contribute. Correct both image settings and authored layout; linter tests are necessary but not sufficient.
