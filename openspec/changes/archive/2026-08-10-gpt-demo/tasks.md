## 1. Model Loading

- [ ] 1.1 Implement `model_load(path)` — open GGUF, read config metadata, allocate weight structs
- [ ] 1.2 Implement `model_unload()` — free all allocated memory

## 2. Tokenizer

- [ ] 2.1 Implement `tokenizer_load(file)` — extract vocab from GGUF metadata, build lookup structures
- [ ] 2.2 Implement `tokenizer_encode(text)` → token array
- [ ] 2.3 Implement `tokenizer_decode(tokens)` → string

## 3. Forward Pass

- [ ] 3.1 Implement RMS norm (pre-attention and post-attention)
- [ ] 3.2 Implement RoPE rotary embeddings (YaRN scaling)
- [ ] 3.3 Implement attention (Q/K/V projections, softmax, weighted sum)
- [ ] 3.4 Implement MoE routing (top-4 expert selection)
- [ ] 3.5 Implement MoE FFN (swiglu gate, up/down projections for selected experts)
- [ ] 3.6 Implement full layer forward pass (norm → attn → residual → norm → MoE → residual)
- [ ] 3.7 Implement full model forward pass (embed → 24 layers → output projection → argmax)

## 4. CLI & Generation

- [ ] 4.1 Implement prompt formatting (harmony chat template)
- [ ] 4.2 Implement generation loop (tokenize → forward → sample → repeat)
- [ ] 4.3 Implement timing (clock before/after forward pass)
- [ ] 4.4 Implement token sampling (greedy argmax)

## 5. Build & Verification

- [ ] 5.1 Create `build-windows-amd64.bat` — link gguf, math, console, memory, string modules
- [ ] 5.2 Create `build-linux-amd64.sh`
- [ ] 5.3 Verify: demo.exe "What is 2+2?" produces a coherent response
- [ ] 5.4 Verify: timing output printed after generation
- [ ] 5.5 Run code formatter
