# gpt-demo Architecture

GPT-OSS-20B MoE model inference on CPU using the Fundamental library.

## Inference pipeline

```ascii
  prompt text
      │
      ▼
  tokenizer (BPE trie lookup)
      │
      ▼
  ┌─────────────────────────────────────────────────────────────┐
  │                    generation loop                          │
  │                                                             │
  │  for each token:                                           │
  │    1. submit compute graph with {token_id, pos}            │
  │    2. wait for graph to finish                             │
  │    3. argmax(logits) → next token                          │
  │    4. if EOS: break                                        │
  └─────────────────────────────────────────────────────────────┘
      │
      ▼
  detokenize → print response
```

## Forward pass (one token, one layer)

```ascii
  Layer N: (dims: hidden=2880, ffn=2880, n_heads=64, n_kv=8, n_exp=32, topk=4)

  ┌──────────────────── Attention ────────────────────────┐
  │                                                        │
  │  hidden ── RMS_norm ──┬── Q_matvec(4096×2880) ── RoPE_Q ──┐
  │                       ├── K_matvec(512×2880)  ── RoPE_K ──┤
  │                       └── V_matvec(512×2880)  ────────────┤
  │                                                            │
  │       Q/RoPE, K/RoPE, V are independent                   │
  │                                                            │
  │  KV_cache_store(kbuf, vbuf, pos)                           │
  │                                                            │
  │  attn_scores = rows_dot(q, k_cache) × inv_sqrt(hd)        │
  │  + SWA mask + sink_append                                  │
  │  softmax(scores)                                           │
  │  weighted_sum(scores, v_cache) → attn_out                  │
  │                                                            │
  │  O_proj(2880×4096)                                         │
  │  hidden' = residual + proj                                 │
  └────────────────────────────────────────────────────────────┘
      │
      ▼
  ┌──────────────────── MoE FFN ──────────────────────────┐
  │                                                        │
  │  RMS_norm(hidden')                                      │
  │       │                                                 │
  │       ├── Router(32×2880) → topk[4] → weights[4]       │
  │       │                                                 │
  │       ├── Expert 0: gate(2880×2880) → silu(·) × up     │
  │       │              → down(2880×2880) → dv_0          │
  │       │                                                 │
  │       ├── Expert 1: (same, independent) → dv_1         │
  │       │                                                 │
  │       ├── Expert 2: (same, independent) → dv_2         │
  │       │                                                 │
  │       └── Expert 3: (same, independent) → dv_3         │
  │                                                        │
  │           4 experts are embarrassingly parallel         │
  │                                                        │
  │  expert_out = w0*dv_0 + w1*dv_1 + w2*dv_2 + w3*dv_3   │
  │  hidden'' = attn_res + expert_out                      │
  └────────────────────────────────────────────────────────┘
      │
      ▼  (×24 layers)
      │
  ┌──────────────────── Output ────────────────────────────┐
  │  RMS_norm(hidden'')                                      │
  │  Q8_matvec(output_weight, 201088×2880) → logits         │
  └──────────────────────────────────────────────────────────┘
```

## Compute profile per token

```ascii
  Operation                FLOPs         %      Vectorized
  ──────────────────────────────────────────────────────
  Q/K/V matvecs            ~30M          2%     AVX2 (fun_math_matrix_vector_f32)
  O projection             ~24M          1%     AVX2
  Attention scores/total   ~14M         <1%     AVX2 (fun_math_rows_dot_f32)
  Expert MXFP4 matvecs    ~995M         78%     AVX2 (fun_math_matrix_vector_mxfp4_f32)
  Output projection       ~579M         19%     AVX2 (fun_math_q8_matrix_vector_f32)
  ──────────────────────────────────────────────────────
  Total per token        ~1.64B        100%

  Per-token memory read traffic: ~1.6 GB (expert weights: 1.27 GB, output: 0.3 GB)
  Measured: 0.45 tok/s (~2.2s/token) single-threaded
```

## Parallelism shape

```ascii
  Within one layer (expert block dominates):

  time ──────────────────────────────────────────────────────────►

    RMS  █
         Q ████  RoPE_Q █
         K █     RoPE_K █  KV █  Attn ████  O ████  Res █  RMS █  Router █
         V ████                                   /  /       /     /            \
                                                 /  /       /     /              \
                                             E0 ████████████████████               \
                                             E1 ████████████████████                \
                                             E2 ████████████████████                 Acc █
                                             E3 ████████████████████

                              4 experts in parallel  ←──  the big win
```

## Thread scaling (projected)

```ascii
  ┌──────────┬────────────┬─────────┬────────────────────────┐
  │ Threads  │ Expert     │ tok/s   │ Note                    │
  │          │ time/layer │         │                         │
  ├──────────┼────────────┼─────────┼─────────────────────────┤
  │ 1        │ 71ms       │ 0.45    │ current baseline        │
  │ 2        │ 36ms       │ 0.75    │                         │
  │ 4        │ 18ms       │ 1.10    │ 1 tok/s target          │
  │ 8        │  9ms       │ 1.28    │ diminishing returns     │
  │ ∞        │  0ms       │ 4.40    │ Amdahl ceiling          │
  └──────────┴────────────┴─────────┴─────────────────────────┘

  Ceiling = time with zero expert cost / time currently = 2.2 / 0.5 = 4.4 tok/s
```

## Compute graph

The forward pass is expressed as a `FunComputeGraph` — a DAG of tasks built once at model load and executed per token.

```ascii
  Graph shape (per layer, repeated ×24):

  ┌─ t_embed ── dequant token → hidden
  ├─ t_rope   ── compute cos[pos], sin[pos]

  ┌─ t_q ── t_rq ──┐
  ├─ t_k ── t_rk ──┤
  ├─ t_v ───────────┤
  │      t_kv       │
  │      t_attn     │
  │       t_o       │
  │     t_res       │
  │    t_norm2      │
  │    t_router     │
  ├─ t_e0 ─┐        │
  ├─ t_e1 ─┤        │
  ├─ t_e2 ─┤        │
  ├─ t_e3 ─┘        │
  │    t_acc ───────┤──► next layer
  └─────────────────┘

  ┌─ t_out_norm ── t_logits ──► logits[]
```

Each task carries an opaque context struct (caller-allocated) with static fields (weight pointers, dimensions) and dynamic fields (buffer pointers updated by `bind` at submit time). The function pointer on each task (`matvec_f32`, `rotary_f32`, `mxfp4_f32`, etc.) calls the corresponding `fun_math_*` primitive.

## Backend extensibility

```ascii
  ┌─────────────────────────────────────────────────────────────┐
  │                                                             │
  │  CPU AVX2 (today)           GPU CUDA (future)               │
  │  ──────────────             ────────────────                │
  │                                                             │
  │  t_q→_matvec_f32_exec       t_q→_matvec_f32_cuda            │
  │    fun_math_matrix_vector     cudaMemcpy(hidden→d_hidden)   │
  │    _f32(w, x, bias, out)    cudaLaunch(matvec_kernel, ...)  │
  │                              cudaStreamSynchronize(stream)  │
  │                                                             │
  │  Same graph. Same submit.   Same graph. Same submit.        │
  │  Same wait.                 Same wait.                      │
  │                                                             │
  │  CPU+GPU hybrid:                                            │
  │    t_attn→_attn_cuda                                       │
  │    t_e0→_expert_avx2, t_e1→_expert_avx2, ...              │
  │    GPU runs attention while CPU workers do experts.         │
  │                                                             │
  │  The function pointer is the abstraction boundary.          │
  │                                                             │
  └─────────────────────────────────────────────────────────────┘
```

## Build

```ascii
  Windows:  build-windows-amd64.bat   (MinGW gcc, -mavx2 -mfma)
  Linux:    build-linux-amd64.sh      (gcc, -mavx2 -mfma)

  Links: gguf, math (AVX2), memory, console, string, compute, thread_pool, sync
```

## Modules used

| Module       | Role                                          |
|--------------|-----------------------------------------------|
| `compute`    | task-graph executor (new)                     |
| `thread_pool`| worker thread management (used by compute)    |
| `sync`       | mutex/condvar (used by thread_pool)           |
| `math`       | all matrix/vector/transcendental primitives   |
| `gguf`       | model file parsing, tensor access, dequant    |
| `memory`     | buffer allocation                             |
| `console`    | output printing                               |
| `string`     | number formatting, token decoding             |
