# GPTFrames — current asset/tool inventory and discovery

**Never assume a constant number of presets/voices**. Discover at runtime:
- `resource_status` returns current inventory and counts; `rescan_resources` refreshes immediately without MCP restart.
- `list_animation_facets` shows categories, types, tags and CSS motion styles.
- `search_animations` retrieves matching ranked presets (search concepts such as `Docker terminal`, `SQL query`, `microservices`, `systemd boot`).
- `describe_animation` returns exact preset ID, path, metadata, editable field names, durations, CSS animations and review status.
- `list_voice_models` yields installed `.neo`/`.onnx` voices (names only; quality tested separately).
- `search_images` is discovery; `download_commons_image` is only supported for licensed Wikimedia PNG/JPEG with attribution record. Openverse is metadata discovery only.

MCP tooling exposed in current code:
1. Assets & discovery: `rescan_resources`, `resource_status`, `search_animations`, `list_animation_facets`, `list_animations`, `describe_animation`, `list_asset_presets`, `describe_asset_preset`, `list_voice_models`.
2. Project management: `list_projects`, `create_project`, `read_project`, `save_project`, `validate_project`, `review_layout`.
3. Output: `preview_frame`, `render_video`, `start_render`, `render_status`, `cancel_render`.
4. Sound and images: `synthesize_voice`, `search_images`, `download_commons_image`.

The `render_video` MCP sync limits may differ from local CLI (currently 60s sync); for longer exports use `start_render` and poll `render_status` or run native CLI directly. Check schema and returned job status—never invent command arguments.

The engine project `--project validate|lint|frame|render` works on native JSON projects and can use browser-first HTML presets when compiled with browser support. The `--browser-widget-frame/render` CLI is for short independent CSS preset QA; current standalone MP4 mode is capped at 10 seconds.

The catalog has original motion graphics, Linux operations, Linux terminal animations, architecture, runtime debugging, language-specific examples and educational diagrams. Resources may be reclassified, improved or added. Use an already verified component whenever possible rather than adding a crude duplicate.

Native `symbol` support is deliberately limited (phone/check/lock/angry-person) and visually basic. ThorVG Lottie is build-time optional. Chroma key is available on still image layers, not video clip layers. Browser-first HTML is restricted to trusted registered presets, with no arbitrary remote images/scripts.

**Audio:** Neo Lite is a separately compiled TTS engine. Current models are in `models/`; use `list_voice_models` to discover `.neo`. The repository itself does not redistribute voice models. Voice synthesis may take time; check the result WAV for actual duration, sample rate, channel count and pronunciation.

Safety: HTML from unknown sources is `detected-unverified` until validated and registered. Do not load untrusted remote pages in a root Chromium without OS-level sandboxing. Read all media licenses before publishing. Do not attribute originality without evidence, nor remove attribution required by a license.

## Animated data charts

`chart-bars-vertical` (Gráficas y datos): colored vertical bars, text fields and optional image. Author numerical variants with `tools/generate_bar_chart.py`. See [ANIMATED_CHARTS.md](ANIMATED_CHARTS.md) for exact data, image and cue contracts. Total collection now 99 animations, plus 20 original assets.
