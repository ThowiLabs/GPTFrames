# Animated bar charts

Use reviewed preset `chart-bars-vertical` (collection `animaciones`) for vertical bars with colored gradients, phased labels/values, a computed comparison and an optional image. This is the delivered chart type; do not advertise unimplemented line, donut or area presets.

## Authoring contract

The canonical data is `animaciones/chart-bars-vertical.data.json`. Generate static offline HTML using Python only at authoring time:
```bash
python3 tools/generate_bar_chart.py --data my-chart.data.json --out /project/resources/animaciones/presets/my-chart.html --metadata /project/chart-meta.json
```
Register the generated file and its returned `editable` metadata in the project resource catalog before using it. A filename alone is not registration. Preserve the original default preset. Set `GPTFRAMES_ROOT=/project/resources` for CLI operations using that private catalog, or use the authorized resource root of the MCP. See `animaciones/ejemplo-graficas.json` for a native widget project.

Configuration supports 1–6 categories with `label`, finite nonnegative `value`, and `color` in #RRGGBB; positive `axis_max`, `tick_step`, at most ten exact intervals; short titles and labels; `value_suffix`; and `summary_mode` ratio or sum. All values must fit the axis, which starts at zero. The generator computes heights, positions, tick labels and the numerical conclusion for both layouts. If values, scale, number of categories or palette change, regenerate and review the SVG. Never replace just a displayed number while leaving old bar geometry. Ratio means maximum category divided by first category, with a defined zero-baseline fallback.

The live native/MCP properties are only declared text fields (`title`, `subtitle`, `tag`, `unit`, `source`, `image_caption`, `comparison`, `conclusion`, `category_1` …) and optional `image`. Numeric values and timing are deliberately not live string properties. If editing labels or a conclusion manually, keep them consistent with the data and recheck layout. Native changes accept at most 16 fields, using underscore keys.

## Optional image

Keep charts full-width by default. Insert an image only when the user supplies or explicitly requests a relevant image. Do not add generic device illustrations, placeholder panels or automatic captions such as “Ilustración del proyecto”. The default image caption is empty; add one only when it provides real information about an intentionally selected image.

Use `"properties":{"image":"assets/illustration.png"}` on a fresh widget instance. The image must be a regular PNG/JPEG/WebP file directly under the project's `assets/` directory, no subdirectory, symlink or traversal, at most 3 MiB. SVG is not accepted by this native file resolver; convert an authorized illustration to PNG first. The compositor embeds it as a data URL. CSS then switches to the narrower precomputed plot and shows a contain-fitted image panel. Without an image property, the chart takes the full plot width and the panel/caption are absent. Do not pass an empty path to clear an image; use a fresh instance without that property.

## Narrated motion is mandatory

Generate/measure the real voice first. Set config `duration` and `timing`: header, axes, image, summary, and one time per category in `categories`, `growth` and `values`. Category labels enter when named, bars grow from the zero baseline, numbers appear with the spoken values, then computed summary/highlight enters at the conclusion. Do not run the default 12-second timing under unrelated narration. Keep `motion_events.json` with spoken anchors, scene-local offsets and measurement provenance. Energy-based word alignment is approximate.

Review actual native frames before/during/after each cue in both image modes. Verify effective ancestor opacity/display, numerical heights, zero/max cases, full image framing, long labels, backward seeks, and absence of text collisions. SVG/CSS uses deterministic finite animations, no JS, no network, CSP. Do not claim `save_project` can inject arbitrary HTML or regenerate numeric data automatically.

## Data integrity and provenance

Default numbers are illustrative, not measured benchmarks. Keep their tag/source visible. Do not attribute these invented values to hardware/frameworks or claim model quality. For real measurements, record actual source, unit, conditions and uncertainty; do not imply causation unsupported by the data.

The adaptation derives from the user-provided Gemini HTML (SHA256 ad65db98d2a55a9b1d2c071cc1c44fc88dc9a603ffdccf55a6989183cf1fd134), replacing incompatible selectors/field names and ambiguous SVG/CSS coordinates. The original is retained unchanged in the editable demo package.
