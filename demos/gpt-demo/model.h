#ifndef GPT_DEMO_MODEL_H
#define GPT_DEMO_MODEL_H

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
	GGufFile *gguf;
	char name_prefix[64];
} LayerWeights;

typedef struct {
	ModelConfig config;
	GGufFile *gguf;
	float *tok_embeddings;
	float *output_weight;
	float *output_norm_weight;
	LayerWeights *layers;
	float **k_cache;
	float **v_cache;
	int cached_len;
} Model;

void model_load(Model *m, GGufFile *gguf);
void model_free(Model *m);
void model_forward(Model *m, const int *tokens, int n_tokens, float *logits);

#endif
