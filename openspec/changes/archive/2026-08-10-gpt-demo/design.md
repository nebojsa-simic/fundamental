## Context

The GPT-OSS-20B model uses the Harmony response format, GQA (64 Q heads / 8 KV heads), MoE (32 experts, top-4), RMS norm, RoPE with YaRN scaling (factor 32), and alternating sliding-window/full attention.

For a single-request demo without KV cache, the forward pass processes all tokens in the prompt at once (prefill), then generates one token at a time autoregressively. Without KV cache, each generation step reprocesses the full sequence. This is correct but O(n²) — acceptable for a demo with ~10 token responses.

## Goals / Non-Goals

**Goals:**
- Load GPT-OSS-20B GGUF model and respond to a command-line prompt
- Implement full transformer forward pass using Fundamental math module
- BPE tokenizer from GGUF embedded vocab
- Print timing: evaluation time, tokens generated, tokens/sec
- Production-quality C code, no shortcuts

**Non-Goals:**
- KV cache (full recompute each step)
- Streaming output (print all at once after generation)
- Configurable sampling (greedy argmax only)
- Multi-turn conversation
- Batched inference

## Decisions

### Decision 1: No KV cache

Each generation step recomputes attention over the full sequence. For a demo with short prompts (< 50 tokens) and short responses (< 100 tokens), this is acceptable. The full sequence length is bounded by the chat template overhead plus user prompt plus response.

### Decision 2: Greedy argmax sampling

After the output projection, select the token with highest logit. No temperature, no top-k, no nucleus sampling. Simplest correct approach for a demo.

### Decision 3: Harmony chat template (hardcoded)

The full Harmony format is complex (reasoning levels, tool definitions, channel markers). For the demo, use a minimal hardcoded template:
```
<|start|>user<|message|>{prompt}<|end|><|start|>assistant<|message|>
```

This is sufficient for simple queries. Full template support is deferred.

### Decision 4: Attention sinks skipped

The `attn_sinks` tensor is a streaming optimization for long contexts. Skip it for single-request processing.

### Decision 5: RoPE with YaRN scaling

YaRN extends RoPE to handle sequences longer than the training context (4096 → 131072). For demo prompts under 4096 tokens, standard RoPE with YaRN's factor=32 and original_max=4096 is mathematically equivalent to standard RoPE at positions 0..4095. We only need positions within this range, so we can implement standard RoPE with theta=150000 and head_dim=64.

Wait — YaRN changes the effective theta per dimension. The corrected theta values are computed per frequency band. For positions within the original context (0..4095), theta values are identical to standard RoPE. For positions beyond, they're scaled differently.

Since we only process prompts < 4096 tokens, standard RoPE works.

### Decision 6: QKV projection shapes

- Q: 2880 → 4096 (64 heads × 64 dim) — larger than hidden_size
- K: 2880 → 512 (8 heads × 64 dim) — GQA
- V: 2880 → 512
- O: 4096 → 2880

After Q projection, reshape to [seq_len, 64 heads, 64 dim]. After K projection, reshape to [seq_len, 8 heads, 64 dim] and expand to 64 heads (repeat each KV head 8 times).

### Decision 7: MoE expert selection

Router: 2880 → 32 (linear + bias). Select top-4 logits. For each selected expert, run swiglu gate: gate = silu_expert(x), up = up_expert(x), output = gate * up. Then down-project: expert_out = down_expert(output). Weighted combine: final = Σ softmax(router_logits[e]) * expert_out[e].

### Decision 8: Weight dequant strategy

Dequant weights on first use per layer, cache in float32 buffers, reuse for subsequent tokens. This means:
- Layer 0 attention weights: dequant once, used for every token
- Layer 0 MoE weights: dequant top-4 experts once per token (different experts each step)
- Attention weights are Q8_0 (small, fast dequant)
- MoE weights are MXFP4 (more complex dequant)

## Memory Budget

```
Model weights (mmap'd):        ~11 GB (OS pages on demand)
Dequant buffers (per layer):   ~200 MB (attention Q8_0 + 4× expert MXFP4)
Activations (per layer):       ~200 KB (hidden, Q, K, V, attention, FFN)
Total working set:             ~200 MB + activations
```

## Completion Criteria

The demo MUST produce a coherent English response when prompted with "What is 2+2?". The output should be approximately "4" or "The answer is 4." or similar. Any coherent response validates the full stack.
