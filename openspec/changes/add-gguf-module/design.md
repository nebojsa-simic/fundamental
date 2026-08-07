## Context

GGUF is a binary format: magic bytes, version, tensor count, metadata KV count, then a sequence of string-keyed metadata values, a sequence of tensor info entries, alignment padding, then raw tensor data. This module needs to parse the header/metadata section, provide key-value queries, expose tensor metadata, and dequantize weights to float32.

The GPT-OSS-20B GGUF file uses three quantization types:
- **F32**: direct float32, requires no dequant
- **Q8_0**: 8-bit block quantization, block size 32, shared scale per block
- **MXFP4**: 4-bit micro-exponent format, block size 32, one E8M0 scale byte per block

## Goals / Non-Goals

**Goals:**
- Memory-map GGUF files for zero-copy tensor access
- Parse GGUF v3 header (magic, version, tensor count, KV count)
- Expose metadata KV: get string, int32, uint32, float32 by key
- Expose tensor info: get shape, type, offset, size by name
- Dequant Q8_0 blocks to float32
- Dequant MXFP4 blocks to float32
- Tests with a minimal synthetic GGUF file

**Non-Goals:**
- Q4_K, Q6_K, IQ-quant dequant (defer to when needed)
- File writing or GGUF creation
- GGUF v2 (older format)
- Tensor data caching or prefetching

## Decisions

### Decision 1: Memory-mapped file access

Tensors are accessed via OS memory mapping (Windows: CreateFileMappingW + MapViewOfFile, Linux: mmap). The mmap handle lives in the GGufFile struct. Tensor data is accessed by offsetting into the mapped view. No read() syscalls, no stdio. Dequant copies from the mapped region into caller-allocated float32 buffers.

### Decision 2: Linear metadata KV scan

GGUF metadata is stored as a flat list of key-value pairs. This module provides `fun_gguf_get_metadata_i32(file, key)` which scans the list linearly. For 35 KV entries, this is negligible. No hash map — keeps the module self-contained.

### Decision 3: Q8_0 dequant

Block size 32, each block has: 2 bytes (d = half-float scale) + 32 bytes (int8 values). Dequant: `float[i] = d * q8[i]` for i in 0..31. The half-float scale is converted to float32 on-the-fly.

### Decision 4: MXFP4 dequant

Block size 32, layout: 1 byte (E8M0 scale: 8-bit exponent, zero mantissa, value = 2^exp) + 16 bytes (32 × 4-bit values packed 2 per byte). Each 4-bit nibble decodes as: 1 sign + 2 exponent + 1 mantissa bits. Dequant: `float[i] = (-1)^S × 2^scale_exp × 2^E × (1 + M/2)`.

### Decision 5: Minimal test file

Generate `tests/gguf/test_data/minimal.gguf` via a C tool (not Python). File is ~200 bytes: magic, version=3, 1 tensor (4-element F32 vector), 2 KV metadata entries. Tests verify open, get metadata, get tensor, dequant.

## API Surface

```c
typedef struct GGufFile GGufFile;

GGufFile *fun_gguf_open(String path);
void fun_gguf_close(GGufFile *file);

// Metadata queries
CanReturnError(String) fun_gguf_get_metadata_string(GGufFile *f, String key);
CanReturnError(int32_t) fun_gguf_get_metadata_i32(GGufFile *f, String key);
CanReturnError(uint32_t) fun_gguf_get_metadata_u32(GGufFile *f, String key);
CanReturnError(float) fun_gguf_get_metadata_f32(GGufFile *f, String key);

// Tensor queries
CanReturnError(uint64_t) fun_gguf_get_tensor_offset(GGufFile *f, String name);
CanReturnError(uint64_t) fun_gguf_get_tensor_size(GGufFile *f, String name);
CanReturnError(uint32_t) fun_gguf_get_tensor_type(GGufFile *f, String name);

// Dequantization (caller provides float32 buffer)
CanReturnError(void) fun_gguf_dequant_f32(GGufFile *f, String name, float *out);
CanReturnError(void) fun_gguf_dequant_q8_0(GGufFile *f, String name, float *out);
CanReturnError(void) fun_gguf_dequant_mxfp4(GGufFile *f, String name, float *out);
```

## Risks / Trade-offs

- **Risk**: GGUF v3 alignment between sections may differ from v2 → **Mitigation**: Test against actual GPT-OSS GGUF file.
- **Risk**: MXFP4 dequant precision vs llama.cpp's implementation → **Mitigation**: Reference llama.cpp's dequant code; this is a well-defined arithmetic transform.
- **Trade-off**: Linear KV scan vs hash map → Acceptable for 35 entries; hash map adds complexity for zero benefit here.
