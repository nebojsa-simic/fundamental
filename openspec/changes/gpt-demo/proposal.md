## Why

The math module provides SIMD-accelerated vector functions (silu_f32, rms_norm_f32, swiglu_f32, softmax_f32) that power LLM inference. The GGUF module enables loading quantized models. The GPT demo ties them together: load a GPT-OSS-20B model, accept a prompt from the command line, run a single forward pass to generate a response, and print it along with evaluation time and tokens/sec. This validates the entire math + GGUF stack end-to-end.

## What Changes

- New `demos/gpt-demo/main.c` — CLI entry, prompt formatting, generation loop, timing
- New `demos/gpt-demo/model.h` / `model.c` — forward pass (attention, MoE FFN, RMS norm, RoPE)
- New `demos/gpt-demo/tokenizer.h` / `tokenizer.c` — BPE encode/decode from GGUF metadata
- New `demos/gpt-demo/build-windows-amd64.bat` — link gguf, math, console, memory, string
- New `demos/gpt-demo/build-linux-amd64.sh` — same for Linux
- No new library code — purely application-level

## Capabilities

### New Capabilities

- `gpt-demo`: End-to-end LLM inference demo using Fundamental modules. Accepts prompt via command line, tokenizes, runs forward pass, samples tokens, detokenizes, prints response with timing statistics.

## Impact

- New directory: `demos/gpt-demo/`
- Dependencies: gguf, math, memory, console, string modules
- No header or source changes to library proper
- Does NOT validate against production tokenizer outputs — the tokenizer is a minimal BPE implementation sufficient for demo purposes
