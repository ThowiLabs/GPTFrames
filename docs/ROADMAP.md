# Roadmap and compatibility policy

`main` contains the current experimental engine. Development and demonstrations may use independent branches or repositories. The main repository does not host video projects or generated media.

| Area | Status |
|---|---|
| Native time-based easing | Implemented |
| 9:16 resampling / resolution selection | Implemented |
| Bounded HTML scene parser and CSS px properties | Implemented, limited |
| litehtml raster adapter | Experimental |
| Browser-assisted Chromium/Chrome/Edge capture | Implemented, requires installed browser |
| H.264/AAC MP4 using embedded C codecs | Experimental, Linux validated |
| Static Linux / cross-compiled Windows builds | Implemented, Windows runtime untested |
| Native JSON scenes, keyframes, layers and WAV track mixing | Implemented, bounded version-1 schema |
| Native Neo Lite speech inference | Implemented, optional local model |
| C++ MCP over stdio / authenticated loopback HTTP | Implemented, eight tools |
| Wikimedia / Openverse discovery, Commons import with attribution | Implemented; Openverse discovery-only |
| Native font embedding and regression fixtures | Implemented in C++ |
| MP3/M4A and other audio inputs | Pending |
| Broader CSS, generic DOM and media compositing | Pending |
| Real Windows QA, fuzzing, CI and security hardening | Pending |
| MP4 codec licensing/patent review | Pending |

No installed FFmpeg binary is required for native output. Browser mode **intentionally** launches an installed compatible browser and is not a general secure sandbox for untrusted HTML. Dependencies added to the build must be redistributable and support static Linux/Windows builds.

## Priorities for general-purpose YouTube videos

The C++ migration preserves the current rendering and tool scope. Higher production quality requires additional composition and production features:

| Priority | Gap | C++ implementation direction |
| --- | --- | --- |
| 1 | Reusable visual direction | Editable typography, spacing, color and layout presets; fit checks and safe areas for 9:16 / 16:9. |
| 1 | Narration timing and subtitles | Measure real audio, add word alignment, caption tracks and subtitle export. |
| 1 | Media workflow | Provider adapters and search filters for orientation, size and usage rights; caching, asset manifests and attribution export. Wikimedia import and Openverse discovery are the current providers. |
| 1 | General YouTube duration | Configurable budgets, cancellation and chunked rendering. Current MCP render limit is 60 seconds; the native JSON compositor permits up to 240 seconds. |
| 2 | Audio finishing | Native compressed-audio import, envelopes, fades, ducking and loudness normalization. Current JSON mixing exposes WAV track start/gain. |
| 2 | Rich browser authoring | MCP operations for trusted HTML compositions, reproducible preview/export and broader browser animation/media timing. Keep the installed browser under C++ control. |
| 2 | Review before delivery | Sample frames, missing-asset checks, text overflow/contrast checks and output inspection. |
| 2 | Production operation | Render job status, authenticated artifact retrieval, bounded queues and Windows runtime QA. |

Browser content remains HTML/CSS/JavaScript executed by Chromium/Chrome/Edge. The engine, MCP, provider clients and production utilities remain C++; additional providers and production features should follow that boundary.
