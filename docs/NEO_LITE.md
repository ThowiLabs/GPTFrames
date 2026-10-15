# Neo Lite / ONNX inference

Neo Lite is imported as a Git submodule from https://github.com/ThowiLabs/neo-tts, pinned to the tested commit. Build with NEO_LITE_ONLY=ON. Linux dependencies: eSpeak NG and development headers, OpenSSL, json-c, zstd and ONNX Runtime. scripts/build-neo-lite-linux.sh downloads ONNX Runtime 1.20.1 from its official release, checks its SHA-256, and builds Neo Lite natively. scripts/neo-lite-local.sh configures the local shared library path.

Voice command:

    hyperframes-cpp-native --voice synth --model models/voz.onnx --text-file texto.txt --out voz.wav --neo-lite /path/to/neo-lite-local.sh

The engine accepts single-speaker .onnx and .neo models. The generated output is validated as RIFF PCM mono 16-bit and deterministically resampled to 24000 Hz with linear interpolation, so the existing video compositor can mix it with other soundtracks. This is an intentionally simple resampler, not a high-fidelity DSP engine.

The integration was exercised with the publicly distributed Spanish Piper model es_ES-davefx-medium; actual VITS inference and conversion generated a 3.6-second WAV in the local sandbox. The model is not committed or redistributed. Review its source/model license separately.

Windows video engine builds are validated. The Windows Neo Lite runtime is not bundled yet: follow upstream Neo-TTS cross-windows instructions and configure the proper ONNX Runtime/espeak dependencies. Real Windows execution is pending.
