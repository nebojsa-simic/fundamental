#ifndef GPT_DEMO_MODEL_H
#define GPT_DEMO_MODEL_H

#include "fundamental/compute/compute.h"
#include "fundamental/gguf/gguf.h"
#include <stdint.h>

typedef struct {
	uint32_t hidden_size;
	uint32_t n_layers;
	uint32_t n_heads;
	uint32_t n_kv_heads;
	uint32_t head_dim;
	uint32_t n_experts;
	uint32_t n_active_experts;
	uint32_t ffn_size;
	uint32_t vocab_size;
	uint32_t max_seq_len;
	float rope_theta;
	float rms_norm_eps;
} ModelConfig;

typedef struct {
	const uint8_t *gate_w;
	const float *gate_b;
	const uint8_t *up_w;
	const float *up_b;
	const uint8_t *down_w;
	const float *down_b;
} ExpertTensors;

typedef struct {
	float *q_weight;
	float *q_bias;
	float *k_weight;
	float *k_bias;
	float *v_weight;
	float *v_bias;
	float *o_weight;
	float *o_bias;
	float *attn_norm_weight;
	float *post_attn_norm_weight;
	float *router_weight;
	float *router_bias;
	float *sinks;
	ExpertTensors experts;
	GGufFile *gguf;
	char name_prefix[64];
} LayerWeights;

typedef struct Model {
	ModelConfig config;
	GGufFile *gguf;
	const uint8_t *tok_embeddings;
	const uint8_t *output_weight;
	float *output_norm_weight;
	LayerWeights *layers;
	float **k_cache;
	float **v_cache;
	float *rope_pre;
	float rope_mscale;
	char model_path[256];
	int cached_len;

	FunComputeGraph graph;
	void *graph_mem;
	FunComputeTask *tasks;
	int n_tasks;

	void *scratch_mem;
	float *logits;

	void *ctx_embed;
	void *ctx_rope;
	void *ctx_embed_residual;
	void *ctx_out_norm;
	void *ctx_logits;
	void **ctx_norms;
	void **ctx_q;
	void **ctx_k;
	void **ctx_v;
	void **ctx_rq;
	void **ctx_rk;
	void **ctx_kv;
	void **ctx_attn;
	void **ctx_o;
	void **ctx_res;
	void **ctx_copy_attn;
	void **ctx_norms2;
	void **ctx_router;
	void **ctx_expert;
	void **ctx_acc;
	void **ctx_copy_res;
} Model;

void model_load(Model *m, GGufFile *gguf, const char *model_path);
void model_free(Model *m);
void model_forward(Model *m, const int *tokens, int n_tokens, float *logits);

#endif
