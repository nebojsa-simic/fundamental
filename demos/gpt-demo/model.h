#ifndef GPT_DEMO_MODEL_H
#define GPT_DEMO_MODEL_H

#include "fundamental/compute/compute.h"
#include "fundamental/gguf/gguf.h"
#include <stdint.h>

/* Hard-coded GPT-OSS-20B-MXFP4 hyperparameters. Every tensor shape in the
 * forward pass derives from these values; the compute graph is built once from
 * them at load time. */
typedef struct {
	uint32_t hidden_size;       /* 2880: width of residual/hidden vectors */
	uint32_t n_layers;          /* 24: transformer blocks */
	uint32_t n_heads;           /* 64: query heads (full) */
	uint32_t n_kv_heads;        /* 8: KV heads (GQA, each serves 8 Q heads) */
	uint32_t head_dim;          /* 64: per-head feature width */
	uint32_t n_experts;         /* 32: total MoE experts */
	uint32_t n_active_experts;  /* 4: top-k experts routed per token */
	uint32_t ffn_size;          /* 2880: hidden dim of each expert MLP */
	uint32_t vocab_size;        /* 201088: output projection rows */
	uint32_t max_seq_len;       /* KV-cache capacity in tokens */
	float rope_theta;           /* 150000: RoPE base frequency */
	float rms_norm_eps;         /* 1e-5: RMS-norm denominator floor */
} ModelConfig;

/* Raw pointers into the GGUF blob for a layer's shared expert weights. All
 * n_experts are packed contiguously, so expert e's data lives at offset
 * e * (ffn*hs*17/32) bytes into the MXFP4 weights. Nothing is dequantized
 * here; the exec kernels read the packed blocks directly. */
typedef struct {
	const uint8_t *gate_w; /* [n_experts][ffn][hs] packed MXFP4 */
	const float *gate_b;   /* [n_experts][ffn] */
	const uint8_t *up_w;   /* [n_experts][ffn][hs] packed MXFP4 */
	const float *up_b;     /* [n_experts][ffn] */
	const uint8_t *down_w; /* [n_experts][hs][ffn] packed MXFP4 */
	const float *down_b;   /* [n_experts][hs] */
} ExpertTensors;

/* Weights for one transformer block. Attention Q/K/V/O weights are raw
 * Q8_0-packed pointers into the GGUF blob (never dequantized); their biases
 * are raw f32 pointers into the same blob. Norms, router and sinks are
 * dequantized eagerly into f32. name_prefix is the block's tensor-name prefix
 * ("blk.<n>."). */
typedef struct {
	const uint8_t *q_weight;       /* [n_heads*head_dim][hidden] packed Q8_0 */
	const float *q_bias;           /* [n_heads*head_dim] */
	const uint8_t *k_weight;       /* [n_kv_heads*head_dim][hidden] packed Q8_0 */
	const float *k_bias;           /* [n_kv_heads*head_dim] */
	const uint8_t *v_weight;       /* [n_kv_heads*head_dim][hidden] packed Q8_0 */
	const float *v_bias;           /* [n_kv_heads*head_dim] */
	const uint8_t *o_weight;       /* [hidden][n_heads*head_dim] packed Q8_0 */
	const float *o_bias;           /* [hidden] */
	float *attn_norm_weight;       /* [hidden]: pre-attention RMS-norm gain */
	float *post_attn_norm_weight;  /* [hidden]: pre-MoE RMS-norm gain */
	float *router_weight;          /* [n_experts][hidden] = [32][2880] */
	float *router_bias;            /* [n_experts] */
	float *sinks;                  /* [n_heads]: attention-sink bias per head */
	ExpertTensors experts;
	char name_prefix[64];          /* "blk.<layer_idx>." used to build tensor names */
} LayerWeights;

/* The whole inference engine. Holds config, GGUF-backed weights, per-layer KV
 * caches, the precomputed compute-graph and all of its task contexts. The
 * ctx_* pointers are the graph-task context blocks; each layer owns a slice of
 * them (indexed [layer] or [layer*4 + slot]). */
typedef struct Model {
	ModelConfig config;
	GGufFile *gguf;
	const uint8_t *tok_embeddings; /* vocab x hidden packed Q8_0 rows */
	const uint8_t *output_weight;  /* vocab x hidden packed Q8_0 rows */
	float *output_norm_weight;     /* [hidden]: final RMS-norm gain */
	LayerWeights *layers;          /* [n_layers] */
	float **k_cache;               /* [n_layers][max_seq][n_kv_heads*head_dim] */
	float **v_cache;               /* [n_layers][max_seq][n_kv_heads*head_dim] */
	float *rope_pre;               /* [head_dim/2]: per-dim inverse frequencies */
	float rope_mscale;             /* RoPE length scaling (YaRN-style) */
	char model_path[256];
	int cached_len;                /* number of tokens already forwarded */

	FunComputeGraph graph;
	void *graph_mem;
	FunComputeTask *tasks;         /* flat task array backing the graph */
	int n_tasks;

	void *scratch_mem;             /* Scratch block: all per-token buffers */
	float *logits;                 /* [vocab]: last token's output logits */

	void *ctx_embed;
	void *ctx_rope;
	void *ctx_embed_residual;
	void *ctx_out_norm;
	void *ctx_logits;
	void **ctx_norms;              /* [n_layers] CtxRmsNorm (pre-attn) */
	void **ctx_q;                  /* [n_layers] CtxMatvecQ8 */
	void **ctx_k;                  /* [n_layers] CtxMatvecQ8 */
	void **ctx_v;                  /* [n_layers] CtxMatvecQ8 */
	void **ctx_rq;                 /* [n_layers] CtxRotary */
	void **ctx_rk;                 /* [n_layers] CtxRotary */
	void **ctx_kv;                 /* [n_layers] CtxKvStore */
	void **ctx_attn;               /* [n_layers] CtxAttention */
	void **ctx_attn_groups;        /* [n_layers*4] CtxAttentionGroup (head shards) */
	void **ctx_o;                  /* [n_layers] CtxMatvecQ8 */
	void **ctx_res;                /* [n_layers] CtxAdd */
	void **ctx_copy_attn;          /* [n_layers] CtxAdd */
	void **ctx_norms2;             /* [n_layers] CtxRmsNorm (post-attn) */
	void **ctx_router;             /* [n_layers] CtxRouter */
	void **ctx_expert;             /* [n_layers*4] CtxExpert */
	void **ctx_acc;                /* [n_layers] CtxExpertAccum */
	void **ctx_copy_res;           /* [n_layers] CtxAdd */
	void **ctx_output_groups;      /* [4] CtxOutputGroup (vocab shards) */
} Model;

/* Build all weights, KV caches, scratch buffers and the compute graph. */
void model_load(Model *m, GGufFile *gguf, const char *model_path,
				int n_threads);
/* Free every allocation made by model_load(). */
void model_free(Model *m);
/* Run the forward pass for tokens[0..n_tokens), one position at a time.
 * Positions already in the KV cache (cached_len) are skipped, so repeated
 * calls are incremental. logits is overwritten with the final position's
 * output distribution over the vocabulary (vocab_size floats). */
void model_forward(Model *m, const int *tokens, int n_tokens, float *logits);

#endif
