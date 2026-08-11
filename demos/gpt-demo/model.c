#include "model.h"
#include "fundamental/compute/compute.h"
#include "fundamental/math/math.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include <immintrin.h>
#include <stdint.h>

#define MAX_SEQ 256

typedef struct {
	int token_id;
	int pos;
} SubmitCtx;

typedef struct {
	float *x;
	const float *weight;
	float *out;
	size_t n;
	float eps;
	LayerWeights *layer;
	GGufFile *gguf;
	int layer_idx;
} CtxRmsNorm;

typedef struct {
	const float **w_ptr;
	const float **bias_ptr;
	float *x;
	float *out;
	size_t rows;
	size_t cols;
	LayerWeights *layer;
	GGufFile *gguf;
} CtxMatvecF32;

typedef struct {
	float *x;
	const float *cos;
	const float *sin;
	size_t n_heads;
	size_t half;
} CtxRotary;

typedef struct {
	float *kbuf;
	float *vbuf;
	float *kcache;
	float *vcache;
	size_t kv_dim;
	int pos;
} CtxKvStore;

typedef struct {
	float *qbuf;
	float *attn_out;
	float *kcache;
	float *vcache;
	const float *sinks;
	int pos;
	int n_heads;
	int n_kv;
	int hd;
	float inv_sqrt_hd;
	float *scores;
} CtxAttention;

typedef struct {
	float *a;
	float *b;
	float *out;
	size_t n;
} CtxAdd;

typedef struct {
	const float *router_w;
	const float *router_b;
	float *x;
	int *topk;
	float *rlog;
	float *rweights;
	int n_exp;
	int topk_n;
	int hs;
} CtxRouter;

typedef struct {
	const uint8_t *gate_w;
	const uint8_t *up_w;
	const uint8_t *down_w;
	const float *gate_b;
	const float *up_b;
	const float *down_b;
	float *hidden_in;
	float *gv;
	float *uv;
	float *mid;
	float *eg;
	float *dv;
	size_t ffn;
	size_t hs;
	int *topk;
	int expert_idx;
} CtxExpert;

typedef struct {
	float *dv0;
	float *dv1;
	float *dv2;
	float *dv3;
	float *rweights;
	int *topk;
	float *hidden;
	float *expert_out;
	float *attn_res;
	size_t hs;
} CtxExpertAccum;

typedef struct {
	const uint8_t *embeddings;
	size_t emb_stride;
	float *hidden;
	size_t hs;
	int token_id;
} CtxEmbed;

typedef struct {
	float *theta;
	float *cos;
	float *sin;
	const float *rope_pre;
	size_t half;
	float mscale;
	int pos;
} CtxRopePre;

typedef struct {
	const uint8_t *w;
	float *x;
	float *out;
	size_t vocab;
	size_t hs;
} CtxOutputProj;

typedef struct {
	float *hidden;
	float *residual;
	float *attn_res;
	float *hidden_normed;
	float *expert_out;
	float *qbuf;
	float *kbuf;
	float *vbuf;
	float *attn;
	float *proj;
	float *scores;
	float *gv[4];
	float *uv[4];
	float *mid[4];
	float *eg[4];
	float *dv[4];
	float *rlog;
	int *topk;
	float *rweights;
	float *theta;
	float *cos;
	float *sin;
} Scratch;

static void dequant_layer(LayerWeights *w, GGufFile *gguf, int layer_idx)
{
	char buf[256];
	int pre_len;
	fun_string_copy("blk.", buf, 256);
	pre_len = (int)fun_string_length(buf);
	fun_string_from_int(layer_idx, 10, buf + pre_len, 256 - pre_len);
	pre_len = (int)fun_string_length(buf);
	fun_string_copy(buf, w->name_prefix, 64);

	fun_string_copy(".attn_q.weight", buf + pre_len, 256 - pre_len);
	w->q_weight = (float *)fun_memory_allocate(
		64 * 64 * 2880 * sizeof(float)).value;
	fun_gguf_dequant_q8_0(gguf, buf, w->q_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_q.bias", buf + pre_len, 256 - pre_len);
	w->q_bias = (float *)fun_memory_allocate(64 * 64 * sizeof(float)).value;
	fun_gguf_dequant_f32(gguf, buf, w->q_bias);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_k.weight", buf + pre_len, 256 - pre_len);
	w->k_weight = (float *)fun_memory_allocate(
		8 * 64 * 2880 * sizeof(float)).value;
	fun_gguf_dequant_q8_0(gguf, buf, w->k_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_k.bias", buf + pre_len, 256 - pre_len);
	w->k_bias = (float *)fun_memory_allocate(8 * 64 * sizeof(float)).value;
	fun_gguf_dequant_f32(gguf, buf, w->k_bias);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_v.weight", buf + pre_len, 256 - pre_len);
	w->v_weight = (float *)fun_memory_allocate(
		8 * 64 * 2880 * sizeof(float)).value;
	fun_gguf_dequant_q8_0(gguf, buf, w->v_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_v.bias", buf + pre_len, 256 - pre_len);
	w->v_bias = (float *)fun_memory_allocate(8 * 64 * sizeof(float)).value;
	fun_gguf_dequant_f32(gguf, buf, w->v_bias);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_output.weight", buf + pre_len, 256 - pre_len);
	w->o_weight = (float *)fun_memory_allocate(
		2880 * 64 * 64 * sizeof(float)).value;
	fun_gguf_dequant_q8_0(gguf, buf, w->o_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_output.bias", buf + pre_len, 256 - pre_len);
	w->o_bias = (float *)fun_memory_allocate(2880 * sizeof(float)).value;
	fun_gguf_dequant_f32(gguf, buf, w->o_bias);
}

static void _exec_rms_norm(void *ctx)
{
	CtxRmsNorm *c = (CtxRmsNorm *)ctx;
	if (c->layer && !c->layer->q_weight)
		dequant_layer(c->layer, c->gguf, c->layer_idx);
	fun_math_rms_norm_f32(c->x, c->weight, c->out, c->n, c->eps);
}

static void _exec_matvec_f32(void *ctx)
{
	CtxMatvecF32 *c = (CtxMatvecF32 *)ctx;
	fun_math_matrix_vector_f32(*c->w_ptr, c->x, *c->bias_ptr, c->out,
				   c->rows, c->cols);
}

static void _exec_rotary(void *ctx)
{
	CtxRotary *c = (CtxRotary *)ctx;
	fun_math_rotary_f32(c->x, c->cos, c->sin, c->x, c->n_heads, c->half);
}

static void _exec_kvstore(void *ctx)
{
	CtxKvStore *c = (CtxKvStore *)ctx;
	size_t kv_dim = c->kv_dim;
	int pos = c->pos;
	for (size_t i = 0; i < kv_dim; i++) {
		c->kcache[pos * kv_dim + i] = c->kbuf[i];
		c->vcache[pos * kv_dim + i] = c->vbuf[i];
	}
}

static void _exec_attention(void *ctx)
{
	CtxAttention *c = (CtxAttention *)ctx;
	int pos = c->pos;
	int n_h = c->n_heads;
	int n_kv = c->n_kv;
	int hd = c->hd;
	int kv_dim = n_kv * hd;
	int swa = 0;
	int swa_len = 128;
	float *scores = c->scores;
	for (int h = 0; h < n_h; h++) {
		int kvh = h * n_kv / n_h;
		float *qh = c->qbuf + (size_t)h * (size_t)hd;
		fun_math_rows_dot_f32(qh, c->kcache + kvh * hd, scores,
				      (size_t)(pos + 1), (size_t)hd,
				      (size_t)kv_dim, c->inv_sqrt_hd);
		if (swa && pos >= swa_len)
			for (int t = 0; t <= pos - swa_len; t++)
				scores[t] = -1.0f / 0.0f;
		scores[pos + 1] = c->sinks[h];
		fun_math_softmax_f32(scores, (size_t)(pos + 2));
		fun_math_weighted_sum_f32(scores, c->vcache + kvh * hd,
					  c->attn_out + h * hd,
					  (size_t)(pos + 1), (size_t)hd,
					  (size_t)kv_dim);
	}
}

static void _exec_add(void *ctx)
{
	CtxAdd *c = (CtxAdd *)ctx;
	for (size_t i = 0; i < c->n; i++)
		c->out[i] = c->a[i] + c->b[i];
}

static void _exec_copy(void *ctx)
{
	CtxAdd *c = (CtxAdd *)ctx;
	for (size_t i = 0; i < c->n; i++)
		c->out[i] = c->a[i];
}

static void _exec_router(void *ctx)
{
	CtxRouter *c = (CtxRouter *)ctx;
	int n_exp = c->n_exp;
	int hs = c->hs;
	for (int e = 0; e < n_exp; e++) {
		float s = 0.0f;
		for (int i = 0; i < hs; i++)
			s += c->router_w[e * hs + i] * c->x[i];
		c->rlog[e] = s + c->router_b[e];
	}
	int topk = c->topk_n;
	int top[4] = { 0, 0, 0, 0 };
	float topv[4] = { -1e30f, -1e30f, -1e30f, -1e30f };
	for (int e = 0; e < n_exp; e++) {
		float v = c->rlog[e];
		if (v > topv[0]) {
			topv[3] = topv[2]; top[3] = top[2];
			topv[2] = topv[1]; top[2] = top[1];
			topv[1] = topv[0]; top[1] = top[0];
			topv[0] = v; top[0] = e;
		} else if (v > topv[1]) {
			topv[3] = topv[2]; top[3] = top[2];
			topv[2] = topv[1]; top[2] = top[1];
			topv[1] = v; top[1] = e;
		} else if (v > topv[2]) {
			topv[3] = topv[2]; top[3] = top[2];
			topv[2] = v; top[2] = e;
		} else if (v > topv[3]) {
			topv[3] = v; top[3] = e;
		}
	}
	for (int i = 0; i < topk; i++)
		c->topk[i] = top[i];
	float rmax = topv[0];
	float rsum = 0.0f;
	for (int i = 0; i < topk; i++) {
		c->rweights[i] = fun_math_exp(topv[i] - rmax);
		rsum += c->rweights[i];
	}
	for (int i = 0; i < topk; i++)
		c->rweights[i] /= rsum;
}

static void _exec_expert(void *ctx)
{
	CtxExpert *c = (CtxExpert *)ctx;
	size_t ffn = c->ffn;
	size_t hs = c->hs;
	int ex = c->topk[c->expert_idx];
	size_t exp_w_bytes = (size_t)ffn * (size_t)hs *
			     FUN_MATH_MXFP4_BLOCK_BYTES /
			     FUN_MATH_MXFP4_BLOCK_ELEMS;
	const uint8_t *gw = c->gate_w + (size_t)ex * exp_w_bytes;
	const float *gb = c->gate_b + (size_t)ex * ffn;
	const uint8_t *uw = c->up_w + (size_t)ex * exp_w_bytes;
	const float *ub = c->up_b + (size_t)ex * ffn;
	const uint8_t *dw = c->down_w + (size_t)ex * exp_w_bytes;
	const float *db = c->down_b + (size_t)ex * hs;
	fun_math_matrix_vector_mxfp4_f32(gw, c->hidden_in, gb, c->gv, ffn, hs);
	fun_math_matrix_vector_mxfp4_f32(uw, c->hidden_in, ub, c->uv, ffn, hs);
	for (size_t i = 0; i < ffn; i++) {
		float g = c->gv[i];
		if (g > 7.0f) g = 7.0f;
		c->eg[i] = -1.702f * g;
	}
	fun_math_exp_f32(c->eg, c->eg, ffn);
	for (size_t i = 0; i < ffn; i++) {
		float g = c->gv[i];
		if (g > 7.0f) g = 7.0f;
		float u = c->uv[i];
		if (u > 7.0f) u = 7.0f;
		if (u < -7.0f) u = -7.0f;
		c->mid[i] = g / (1.0f + c->eg[i]) * (u + 1.0f);
	}
	fun_math_matrix_vector_mxfp4_f32(dw, c->mid, db, c->dv, hs, ffn);
}

static void _exec_expert_accum(void *ctx)
{
	CtxExpertAccum *c = (CtxExpertAccum *)ctx;
	float *dv[4] = { c->dv0, c->dv1, c->dv2, c->dv3 };
	for (size_t i = 0; i < c->hs; i++)
		c->expert_out[i] = 0.0f;
	for (int ex = 0; ex < 4; ex++) {
		float w = c->rweights[ex];
		for (size_t i = 0; i < c->hs; i++)
			c->expert_out[i] += w * dv[ex][i];
	}
	for (size_t i = 0; i < c->hs; i++)
		c->hidden[i] = c->attn_res[i] + c->expert_out[i];
}

static void _exec_embed(void *ctx)
{
	CtxEmbed *c = (CtxEmbed *)ctx;
	fun_math_q8_dequant_row_f32(
		c->embeddings + (size_t)(c->token_id) * c->emb_stride,
		c->hidden, c->hs);
}

static void _exec_rope_pre(void *ctx)
{
	CtxRopePre *c = (CtxRopePre *)ctx;
	size_t half = c->half;
	int pos = c->pos;
	for (size_t j = 0; j < half; j++)
		c->theta[j] = (float)pos * c->rope_pre[j];
	fun_math_cos_f32(c->theta, c->cos, half);
	fun_math_sin_f32(c->theta, c->sin, half);
	for (size_t j = 0; j < half; j++) {
		c->cos[j] *= c->mscale;
		c->sin[j] *= c->mscale;
	}
}

static void _exec_output_proj(void *ctx)
{
	CtxOutputProj *c = (CtxOutputProj *)ctx;
	fun_math_q8_matrix_vector_f32(c->w, c->x, c->out, c->vocab, c->hs);
}

static void _bind_embed(void *task_ctx, void *submit_ctx)
{
	CtxEmbed *c = (CtxEmbed *)task_ctx;
	SubmitCtx *s = (SubmitCtx *)submit_ctx;
	c->token_id = s->token_id;
}

static void _bind_rope_pre(void *task_ctx, void *submit_ctx)
{
	CtxRopePre *c = (CtxRopePre *)task_ctx;
	SubmitCtx *s = (SubmitCtx *)submit_ctx;
	c->pos = s->pos;
}

static void _bind_kvstore(void *task_ctx, void *submit_ctx)
{
	CtxKvStore *c = (CtxKvStore *)task_ctx;
	SubmitCtx *s = (SubmitCtx *)submit_ctx;
	c->pos = s->pos;
}

static void _bind_attention(void *task_ctx, void *submit_ctx)
{
	CtxAttention *c = (CtxAttention *)task_ctx;
	SubmitCtx *s = (SubmitCtx *)submit_ctx;
	c->pos = s->pos;
}

static void _bind_residual(void *task_ctx, void *submit_ctx)
{
	CtxAdd *c = (CtxAdd *)task_ctx;
	(void)submit_ctx;
	for (size_t i = 0; i < c->n; i++)
		c->b[i] = c->a[i];
}

static void scratch_alloc(Scratch *sc, ModelConfig *cfg)
{
	int hs = (int)cfg->hidden_size;
	int ffn = (int)cfg->ffn_size;
	int half = (int)cfg->head_dim / 2;
	int q_dim = (int)cfg->n_heads * (int)cfg->head_dim;
	int kv_dim = (int)cfg->n_kv_heads * (int)cfg->head_dim;
	MemoryResult mr;
	mr = fun_memory_allocate((size_t)hs * sizeof(float)); sc->hidden = (float *)mr.value;
	mr = fun_memory_allocate((size_t)hs * sizeof(float)); sc->residual = (float *)mr.value;
	mr = fun_memory_allocate((size_t)hs * sizeof(float)); sc->attn_res = (float *)mr.value;
	mr = fun_memory_allocate((size_t)hs * sizeof(float)); sc->hidden_normed = (float *)mr.value;
	mr = fun_memory_allocate((size_t)hs * sizeof(float)); sc->expert_out = (float *)mr.value;
	mr = fun_memory_allocate((size_t)q_dim * sizeof(float)); sc->qbuf = (float *)mr.value;
	mr = fun_memory_allocate((size_t)kv_dim * sizeof(float)); sc->kbuf = (float *)mr.value;
	mr = fun_memory_allocate((size_t)kv_dim * sizeof(float)); sc->vbuf = (float *)mr.value;
	mr = fun_memory_allocate((size_t)q_dim * sizeof(float)); sc->attn = (float *)mr.value;
	mr = fun_memory_allocate((size_t)hs * sizeof(float)); sc->proj = (float *)mr.value;
	mr = fun_memory_allocate((size_t)(MAX_SEQ + 2) * sizeof(float)); sc->scores = (float *)mr.value;
	for (int e = 0; e < 4; e++) {
		mr = fun_memory_allocate((size_t)ffn * sizeof(float)); sc->gv[e] = (float *)mr.value;
		mr = fun_memory_allocate((size_t)ffn * sizeof(float)); sc->uv[e] = (float *)mr.value;
		mr = fun_memory_allocate((size_t)ffn * sizeof(float)); sc->mid[e] = (float *)mr.value;
		mr = fun_memory_allocate((size_t)ffn * sizeof(float)); sc->eg[e] = (float *)mr.value;
		mr = fun_memory_allocate((size_t)hs * sizeof(float)); sc->dv[e] = (float *)mr.value;
	}
	mr = fun_memory_allocate(32 * sizeof(float)); sc->rlog = (float *)mr.value;
	mr = fun_memory_allocate(4 * sizeof(int)); sc->topk = (int *)mr.value;
	mr = fun_memory_allocate(4 * sizeof(float)); sc->rweights = (float *)mr.value;
	mr = fun_memory_allocate((size_t)half * sizeof(float)); sc->theta = (float *)mr.value;
	mr = fun_memory_allocate((size_t)half * sizeof(float)); sc->cos = (float *)mr.value;
	mr = fun_memory_allocate((size_t)half * sizeof(float)); sc->sin = (float *)mr.value;
}

static void scratch_free(Scratch *sc)
{
	fun_memory_free((Memory *)&sc->hidden);
	fun_memory_free((Memory *)&sc->residual);
	fun_memory_free((Memory *)&sc->attn_res);
	fun_memory_free((Memory *)&sc->hidden_normed);
	fun_memory_free((Memory *)&sc->expert_out);
	fun_memory_free((Memory *)&sc->qbuf);
	fun_memory_free((Memory *)&sc->kbuf);
	fun_memory_free((Memory *)&sc->vbuf);
	fun_memory_free((Memory *)&sc->attn);
	fun_memory_free((Memory *)&sc->proj);
	fun_memory_free((Memory *)&sc->scores);
	for (int e = 0; e < 4; e++) {
		fun_memory_free((Memory *)&sc->gv[e]);
		fun_memory_free((Memory *)&sc->uv[e]);
		fun_memory_free((Memory *)&sc->mid[e]);
		fun_memory_free((Memory *)&sc->eg[e]);
		fun_memory_free((Memory *)&sc->dv[e]);
	}
	fun_memory_free((Memory *)&sc->rlog);
	fun_memory_free((Memory *)&sc->topk);
	fun_memory_free((Memory *)&sc->rweights);
	fun_memory_free((Memory *)&sc->theta);
	fun_memory_free((Memory *)&sc->cos);
	fun_memory_free((Memory *)&sc->sin);
}

void model_load(Model *m, GGufFile *gguf, const char *model_path)
{
	m->gguf = gguf;
	fun_string_copy(model_path, m->model_path, sizeof(m->model_path));
	m->config.hidden_size = 2880;
	m->config.n_layers = 24;
	m->config.n_heads = 64;
	m->config.n_kv_heads = 8;
	m->config.head_dim = 64;
	m->config.n_experts = 32;
	m->config.n_active_experts = 4;
	m->config.ffn_size = 2880;
	m->config.vocab_size = 201088;
	m->config.max_seq_len = MAX_SEQ;
	m->config.rope_theta = 150000.0f;
	m->config.rms_norm_eps = 1e-5f;
	m->rope_mscale = 1.0f + 0.1f * fun_math_log(32.0f);

	uint64_t te_off =
		fun_gguf_get_tensor_offset(gguf, "token_embd.weight").value;
	m->tok_embeddings = fun_gguf_get_raw_data(gguf) + te_off;

	uint64_t ow_off = fun_gguf_get_tensor_offset(gguf, "output.weight").value;
	m->output_weight = fun_gguf_get_raw_data(gguf) + ow_off;

	m->output_norm_weight =
		(float *)fun_memory_allocate(m->config.hidden_size * sizeof(float))
			.value;
	fun_gguf_dequant_f32(gguf, "output_norm.weight", m->output_norm_weight);

	m->layers = (LayerWeights *)fun_memory_allocate(
		m->config.n_layers * sizeof(LayerWeights)).value;
	for (int i = 0; i < m->config.n_layers; i++) {
		m->layers[i].q_weight = NULL;
		m->layers[i].q_bias = NULL;
		m->layers[i].k_weight = NULL;
		m->layers[i].k_bias = NULL;
		m->layers[i].v_weight = NULL;
		m->layers[i].v_bias = NULL;
		m->layers[i].o_weight = NULL;
		m->layers[i].o_bias = NULL;
		m->layers[i].attn_norm_weight = NULL;
		m->layers[i].post_attn_norm_weight = NULL;
		m->layers[i].router_weight = NULL;
		m->layers[i].router_bias = NULL;
		m->layers[i].sinks = NULL;
		m->layers[i].gguf = NULL;
	}

	int kv_dim = m->config.n_kv_heads * m->config.head_dim;
	int max_seq = m->config.max_seq_len;
	m->k_cache = (float **)fun_memory_allocate(
		m->config.n_layers * sizeof(float *)).value;
	m->v_cache = (float **)fun_memory_allocate(
		m->config.n_layers * sizeof(float *)).value;
	for (int i = 0; i < m->config.n_layers; i++) {
		m->k_cache[i] = (float *)fun_memory_allocate(
			(size_t)max_seq * kv_dim * sizeof(float)).value;
		m->v_cache[i] = (float *)fun_memory_allocate(
			(size_t)max_seq * kv_dim * sizeof(float)).value;
	}
	m->cached_len = 0;

	int half = m->config.head_dim / 2;
	m->rope_pre = (float *)fun_memory_allocate(
		(size_t)half * sizeof(float)).value;
	float theta_scale = fun_math_exp(fun_math_log(m->config.rope_theta) *
					 (-2.0f / (float)m->config.head_dim));
	int corr0 = 8, corr1 = 18;
	float power = 1.0f;
	for (int j = 0; j < half; j++) {
		float y = (float)(j - corr0) / (float)(corr1 - corr0);
		if (y < 0.0f) y = 0.0f;
		if (y > 1.0f) y = 1.0f;
		float ramp = 1.0f - y;
		float factor = ramp + (1.0f - ramp) / 32.0f;
		m->rope_pre[j] = power * factor;
		power *= theta_scale;
	}

	/* Pre-load expert tensor offsets and non-attention weights only.
	 * Attention weights (Q/K/V/O) are lazily dequantized on first use. */
	const uint8_t *rd = fun_gguf_get_raw_data(m->gguf);
	for (int l = 0; l < m->config.n_layers; l++) {
		LayerWeights *w = &m->layers[l];
		char buf[256];
		int pre_len;
		fun_string_copy("blk.", buf, 256);
		pre_len = (int)fun_string_length(buf);
		fun_string_from_int(l, 10, buf + pre_len, 256 - pre_len);
		pre_len = (int)fun_string_length(buf);
		fun_string_copy(buf, w->name_prefix, 64);

		buf[pre_len] = '\0';
		fun_string_copy(".attn_norm.weight", buf + pre_len, 256 - pre_len);
		w->attn_norm_weight = (float *)fun_memory_allocate(
			m->config.hidden_size * sizeof(float)).value;
		fun_gguf_dequant_f32(m->gguf, buf, w->attn_norm_weight);

		buf[pre_len] = '\0';
		fun_string_copy(".post_attention_norm.weight", buf + pre_len,
				256 - pre_len);
		w->post_attn_norm_weight = (float *)fun_memory_allocate(
			m->config.hidden_size * sizeof(float)).value;
		fun_gguf_dequant_f32(m->gguf, buf, w->post_attn_norm_weight);

		buf[pre_len] = '\0';
		fun_string_copy(".ffn_gate_inp.weight", buf + pre_len, 256 - pre_len);
		w->router_weight = (float *)fun_memory_allocate(
			m->config.n_experts * m->config.hidden_size * sizeof(float)).value;
		fun_gguf_dequant_f32(m->gguf, buf, w->router_weight);

		buf[pre_len] = '\0';
		fun_string_copy(".ffn_gate_inp.bias", buf + pre_len, 256 - pre_len);
		w->router_bias = (float *)fun_memory_allocate(
			m->config.n_experts * sizeof(float)).value;
		fun_gguf_dequant_f32(m->gguf, buf, w->router_bias);

		buf[pre_len] = '\0';
		fun_string_copy(".attn_sinks.weight", buf + pre_len, 256 - pre_len);
		w->sinks = (float *)fun_memory_allocate(
			m->config.n_heads * sizeof(float)).value;
		fun_gguf_dequant_f32(m->gguf, buf, w->sinks);

		buf[pre_len] = '\0';
		fun_string_copy(".ffn_gate_exps.weight", buf + pre_len, 256 - pre_len);
		w->experts.gate_w = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;
		buf[pre_len] = '\0';
		fun_string_copy(".ffn_gate_exps.bias", buf + pre_len, 256 - pre_len);
		w->experts.gate_b = (const float *)(rd + fun_gguf_get_tensor_offset(m->gguf, buf).value);
		buf[pre_len] = '\0';
		fun_string_copy(".ffn_up_exps.weight", buf + pre_len, 256 - pre_len);
		w->experts.up_w = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;
		buf[pre_len] = '\0';
		fun_string_copy(".ffn_up_exps.bias", buf + pre_len, 256 - pre_len);
		w->experts.up_b = (const float *)(rd + fun_gguf_get_tensor_offset(m->gguf, buf).value);
		buf[pre_len] = '\0';
		fun_string_copy(".ffn_down_exps.weight", buf + pre_len, 256 - pre_len);
		w->experts.down_w = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;
		buf[pre_len] = '\0';
		fun_string_copy(".ffn_down_exps.bias", buf + pre_len, 256 - pre_len);
		w->experts.down_b = (const float *)(rd + fun_gguf_get_tensor_offset(m->gguf, buf).value);
	}

	m->scratch_mem = fun_memory_allocate(sizeof(Scratch)).value;
	Scratch *s = (Scratch *)m->scratch_mem;
	scratch_alloc(s, &m->config);

	m->logits = (float *)fun_memory_allocate(
		m->config.vocab_size * sizeof(float)).value;

	int n_layers = m->config.n_layers;
	int max_tasks = 6 + n_layers * 22;
	int max_edges = 8 + n_layers * 30;
	size_t graph_bytes = fun_compute_graph_memory_required(max_tasks, max_edges, 0);
	m->graph_mem = fun_memory_allocate(graph_bytes).value;
	m->graph = fun_compute_graph_init(m->graph_mem, graph_bytes,
					   max_tasks, max_edges, 4);

	m->n_tasks = max_tasks;
	m->tasks = (FunComputeTask *)fun_memory_allocate(
		(size_t)max_tasks * sizeof(FunComputeTask)).value;

	int ti = 0;
	float inv_sqrt_hd = 1.0f / fun_math_sqrt((float)m->config.head_dim);

	m->ctx_embed = fun_memory_allocate(sizeof(CtxEmbed)).value;
	*(CtxEmbed *)m->ctx_embed = (CtxEmbed){
		.embeddings = m->tok_embeddings,
		.emb_stride = (size_t)m->config.hidden_size / FUN_MATH_Q8_BLOCK_ELEMS
			      * FUN_MATH_Q8_BLOCK_BYTES,
		.hidden = s->hidden, .hs = m->config.hidden_size,
	};
	FunComputeTask *t_embed = &m->tasks[ti++];
	fun_compute_graph_add_task(m->graph, t_embed, _exec_embed,
				   m->ctx_embed, _bind_embed, NULL);

	m->ctx_rope = fun_memory_allocate(sizeof(CtxRopePre)).value;
	*(CtxRopePre *)m->ctx_rope = (CtxRopePre){
		.theta = s->theta, .cos = s->cos, .sin = s->sin,
		.rope_pre = m->rope_pre, .half = m->config.head_dim / 2,
		.mscale = m->rope_mscale,
	};
	FunComputeTask *t_rope = &m->tasks[ti++];
	fun_compute_graph_add_task(m->graph, t_rope, _exec_rope_pre,
				   m->ctx_rope, _bind_rope_pre, NULL);

	m->ctx_embed_residual = fun_memory_allocate(sizeof(CtxAdd)).value;
	*(CtxAdd *)m->ctx_embed_residual = (CtxAdd){
		.a = s->hidden, .b = s->residual, .out = s->residual, .n = m->config.hidden_size,
	};
	FunComputeTask *t_res_embed = &m->tasks[ti++];
	fun_compute_graph_add_task(m->graph, t_res_embed, _exec_copy,
				   m->ctx_embed_residual, _bind_residual, NULL);
	fun_compute_task_depends_on(m->graph, t_res_embed, t_embed);

	m->ctx_norms = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_q = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_k = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_v = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_rq = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_rk = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_kv = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_attn = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_o = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_res = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_copy_attn = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_norms2 = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_router = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_acc = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_copy_res = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_expert = fun_memory_allocate((size_t)n_layers * 4 * sizeof(void *)).value;

	FunComputeTask *prev_layer_tail = t_res_embed;

	for (int l = 0; l < n_layers; l++) {
		LayerWeights *w = &m->layers[l];

		FunComputeTask *t_norm = &m->tasks[ti++];
		m->ctx_norms[l] = fun_memory_allocate(sizeof(CtxRmsNorm)).value;
		*(CtxRmsNorm *)m->ctx_norms[l] = (CtxRmsNorm){
			.x = s->hidden, .weight = w->attn_norm_weight,
			.out = s->hidden_normed, .n = m->config.hidden_size,
			.eps = m->config.rms_norm_eps,
			.layer = w, .gguf = m->gguf, .layer_idx = l,
		};

		// (t_norm added, depends_on below)
		fun_compute_graph_add_task(m->graph, t_norm, _exec_rms_norm,
					   m->ctx_norms[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_norm, prev_layer_tail);

		FunComputeTask *t_q = &m->tasks[ti++];
		m->ctx_q[l] = fun_memory_allocate(sizeof(CtxMatvecF32)).value;
		*(CtxMatvecF32 *)m->ctx_q[l] = (CtxMatvecF32){
			.w_ptr = (const float **)&w->q_weight,
			.bias_ptr = (const float **)&w->q_bias,
			.x = s->hidden_normed, .out = s->qbuf,
			.rows = m->config.n_heads * m->config.head_dim,
			.cols = m->config.hidden_size,
		};
		fun_compute_graph_add_task(m->graph, t_q, _exec_matvec_f32,
					   m->ctx_q[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_q, t_norm);

		FunComputeTask *t_k = &m->tasks[ti++];
		m->ctx_k[l] = fun_memory_allocate(sizeof(CtxMatvecF32)).value;
		*(CtxMatvecF32 *)m->ctx_k[l] = (CtxMatvecF32){ .w_ptr = (const float **)&w->k_weight, .bias_ptr = (const float **)&w->k_bias, .x = s->hidden_normed, .out = s->kbuf, .rows = m->config.n_kv_heads * m->config.head_dim, .cols = m->config.hidden_size,
		};
		fun_compute_graph_add_task(m->graph, t_k, _exec_matvec_f32,
					   m->ctx_k[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_k, t_norm);

		FunComputeTask *t_v = &m->tasks[ti++];
		m->ctx_v[l] = fun_memory_allocate(sizeof(CtxMatvecF32)).value;
		*(CtxMatvecF32 *)m->ctx_v[l] = (CtxMatvecF32){ .w_ptr = (const float **)&w->v_weight, .bias_ptr = (const float **)&w->v_bias, .x = s->hidden_normed, .out = s->vbuf, .rows = m->config.n_kv_heads * m->config.head_dim, .cols = m->config.hidden_size,
		};
		fun_compute_graph_add_task(m->graph, t_v, _exec_matvec_f32,
					   m->ctx_v[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_v, t_norm);

		FunComputeTask *t_rq = &m->tasks[ti++];
		m->ctx_rq[l] = fun_memory_allocate(sizeof(CtxRotary)).value;
		*(CtxRotary *)m->ctx_rq[l] = (CtxRotary){ .x = s->qbuf, .cos = s->cos, .sin = s->sin, .n_heads = m->config.n_heads, .half = m->config.head_dim / 2,
		};
		fun_compute_graph_add_task(m->graph, t_rq, _exec_rotary,
					   m->ctx_rq[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_rq, t_q);

		FunComputeTask *t_rk = &m->tasks[ti++];
		m->ctx_rk[l] = fun_memory_allocate(sizeof(CtxRotary)).value;
		*(CtxRotary *)m->ctx_rk[l] = (CtxRotary){ .x = s->kbuf, .cos = s->cos, .sin = s->sin, .n_heads = m->config.n_kv_heads, .half = m->config.head_dim / 2,
		};
		fun_compute_graph_add_task(m->graph, t_rk, _exec_rotary,
					   m->ctx_rk[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_rk, t_k);

		FunComputeTask *t_kv = &m->tasks[ti++];
		m->ctx_kv[l] = fun_memory_allocate(sizeof(CtxKvStore)).value;
		*(CtxKvStore *)m->ctx_kv[l] = (CtxKvStore){ .kbuf = s->kbuf, .vbuf = s->vbuf, .kcache = m->k_cache[l], .vcache = m->v_cache[l], .kv_dim = m->config.n_kv_heads * m->config.head_dim,
		};
		fun_compute_graph_add_task(m->graph, t_kv, _exec_kvstore,
					   m->ctx_kv[l], _bind_kvstore, NULL);
		fun_compute_task_depends_on(m->graph, t_kv, t_rk);
		fun_compute_task_depends_on(m->graph, t_kv, t_v);

		FunComputeTask *t_attn = &m->tasks[ti++];
		m->ctx_attn[l] = fun_memory_allocate(sizeof(CtxAttention)).value;
		*(CtxAttention *)m->ctx_attn[l] = (CtxAttention){ .qbuf = s->qbuf, .attn_out = s->attn, .kcache = m->k_cache[l], .vcache = m->v_cache[l], .sinks = w->sinks, .n_heads = m->config.n_heads, .n_kv = m->config.n_kv_heads, .hd = m->config.head_dim, .inv_sqrt_hd = inv_sqrt_hd, .scores = s->scores,
		};
		fun_compute_graph_add_task(m->graph, t_attn, _exec_attention,
					   m->ctx_attn[l], _bind_attention, NULL);
		fun_compute_task_depends_on(m->graph, t_attn, t_rq);
		fun_compute_task_depends_on(m->graph, t_attn, t_kv);

		FunComputeTask *t_o = &m->tasks[ti++];
		m->ctx_o[l] = fun_memory_allocate(sizeof(CtxMatvecF32)).value;
		*(CtxMatvecF32 *)m->ctx_o[l] = (CtxMatvecF32){ .w_ptr = (const float **)&w->o_weight, .bias_ptr = (const float **)&w->o_bias, .x = s->attn, .out = s->proj, .rows = m->config.hidden_size, .cols = m->config.n_heads * m->config.head_dim,
		};
		fun_compute_graph_add_task(m->graph, t_o, _exec_matvec_f32,
					   m->ctx_o[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_o, t_attn);

		FunComputeTask *t_res = &m->tasks[ti++];
		m->ctx_res[l] = fun_memory_allocate(sizeof(CtxAdd)).value;
		*(CtxAdd *)m->ctx_res[l] = (CtxAdd){ .a = s->residual, .b = s->proj, .out = s->hidden, .n = m->config.hidden_size,
		};
		fun_compute_graph_add_task(m->graph, t_res, _exec_add,
					   m->ctx_res[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_res, t_o);

		FunComputeTask *t_copy_attn = &m->tasks[ti++];
		m->ctx_copy_attn[l] = fun_memory_allocate(sizeof(CtxAdd)).value;
		*(CtxAdd *)m->ctx_copy_attn[l] = (CtxAdd){ .a = s->hidden, .out = s->attn_res, .n = m->config.hidden_size,
		};
		fun_compute_graph_add_task(m->graph, t_copy_attn, _exec_copy,
					   m->ctx_copy_attn[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_copy_attn, t_res);

		FunComputeTask *t_norm2 = &m->tasks[ti++];
		m->ctx_norms2[l] = fun_memory_allocate(sizeof(CtxRmsNorm)).value;
		*(CtxRmsNorm *)m->ctx_norms2[l] = (CtxRmsNorm){ .x = s->hidden, .weight = w->post_attn_norm_weight, .out = s->hidden_normed, .n = m->config.hidden_size, .eps = m->config.rms_norm_eps,
		};
		fun_compute_graph_add_task(m->graph, t_norm2, _exec_rms_norm,
					   m->ctx_norms2[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_norm2, t_copy_attn);

		FunComputeTask *t_router = &m->tasks[ti++];
		m->ctx_router[l] = fun_memory_allocate(sizeof(CtxRouter)).value;
		*(CtxRouter *)m->ctx_router[l] = (CtxRouter){ .router_w = w->router_weight, .router_b = w->router_bias, .x = s->hidden_normed, .topk = s->topk, .rlog = s->rlog, .rweights = s->rweights, .n_exp = m->config.n_experts, .topk_n = m->config.n_active_experts, .hs = m->config.hidden_size,
		};
		fun_compute_graph_add_task(m->graph, t_router, _exec_router,
					   m->ctx_router[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_router, t_norm2);

		FunComputeTask *t_e[4];
		for (int e = 0; e < 4; e++) {
			t_e[e] = &m->tasks[ti++];
			m->ctx_expert[l * 4 + e] = fun_memory_allocate(sizeof(CtxExpert)).value;
			*(CtxExpert *)m->ctx_expert[l * 4 + e] = (CtxExpert){ .gate_w = w->experts.gate_w, .up_w = w->experts.up_w, .down_w = w->experts.down_w, .gate_b = w->experts.gate_b, .up_b = w->experts.up_b, .down_b = w->experts.down_b, .hidden_in = s->hidden_normed, .gv = s->gv[e], .uv = s->uv[e], .mid = s->mid[e], .eg = s->eg[e], .dv = s->dv[e], .ffn = m->config.ffn_size, .hs = m->config.hidden_size, .topk = s->topk, .expert_idx = e,
			};
			fun_compute_graph_add_task(m->graph, t_e[e], _exec_expert,
						   m->ctx_expert[l * 4 + e], NULL, NULL);
			fun_compute_task_depends_on(m->graph, t_e[e], t_router);
		}

		FunComputeTask *t_acc = &m->tasks[ti++];
		m->ctx_acc[l] = fun_memory_allocate(sizeof(CtxExpertAccum)).value;
		*(CtxExpertAccum *)m->ctx_acc[l] = (CtxExpertAccum){ .dv0 = s->dv[0], .dv1 = s->dv[1], .dv2 = s->dv[2], .dv3 = s->dv[3], .rweights = s->rweights, .topk = s->topk, .hidden = s->hidden, .expert_out = s->expert_out, .attn_res = s->attn_res, .hs = m->config.hidden_size,
		};
		fun_compute_graph_add_task(m->graph, t_acc, _exec_expert_accum,
					   m->ctx_acc[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_acc, t_router);
		for (int e = 0; e < 4; e++)
			fun_compute_task_depends_on(m->graph, t_acc, t_e[e]);

		FunComputeTask *t_copy_res = &m->tasks[ti++];
		m->ctx_copy_res[l] = fun_memory_allocate(sizeof(CtxAdd)).value;
		*(CtxAdd *)m->ctx_copy_res[l] = (CtxAdd){ .a = s->hidden, .b = s->residual, .out = s->residual, .n = m->config.hidden_size,
		};
		fun_compute_graph_add_task(m->graph, t_copy_res, _exec_copy,
					   m->ctx_copy_res[l], _bind_residual, NULL);
		fun_compute_task_depends_on(m->graph, t_copy_res, t_acc);

		prev_layer_tail = t_copy_res;
	}

	FunComputeTask *t_out_norm = &m->tasks[ti++];
	m->ctx_out_norm = fun_memory_allocate(sizeof(CtxRmsNorm)).value;
	*(CtxRmsNorm *)m->ctx_out_norm = (CtxRmsNorm){
		.x = s->hidden, .weight = m->output_norm_weight,
		.out = s->hidden_normed, .n = m->config.hidden_size,
		.eps = m->config.rms_norm_eps,
	};
	fun_compute_graph_add_task(m->graph, t_out_norm, _exec_rms_norm,
				   m->ctx_out_norm, NULL, NULL);
	fun_compute_task_depends_on(m->graph, t_out_norm, prev_layer_tail);

	FunComputeTask *t_logits = &m->tasks[ti++];
	m->ctx_logits = fun_memory_allocate(sizeof(CtxOutputProj)).value;
	*(CtxOutputProj *)m->ctx_logits = (CtxOutputProj){
		.w = m->output_weight, .x = s->hidden_normed,
		.out = m->logits, .vocab = m->config.vocab_size,
		.hs = m->config.hidden_size,
	};
	fun_compute_graph_add_task(m->graph, t_logits, _exec_output_proj,
				   m->ctx_logits, NULL, NULL);
	fun_compute_task_depends_on(m->graph, t_logits, t_out_norm);

	m->n_tasks = ti;
}

void model_forward(Model *m, const int *tokens, int n_tokens, float *logits)
{
	int start = m->cached_len;
	if (start < 0 || start > n_tokens)
		start = 0;
	for (int pos = start; pos < n_tokens; pos++) {
		SubmitCtx sctx = { .token_id = tokens[pos], .pos = pos };
		fun_compute_graph_submit(m->graph, &sctx);
		fun_compute_graph_wait(m->graph);
		m->cached_len = n_tokens;
		for (size_t i = 0; i < m->config.vocab_size; i++)
			logits[i] = m->logits[i];
	}
}

void model_free(Model *m)
{
	if (m->output_norm_weight)
		fun_memory_free((Memory *)&m->output_norm_weight);
	if (m->rope_pre)
		fun_memory_free((Memory *)&m->rope_pre);
	if (m->graph) {
		fun_compute_graph_destroy(m->graph);
		fun_memory_free((Memory *)&m->graph_mem);
	}

	for (int i = 0; i < m->n_tasks; i++)
		if (m->tasks && m->tasks[i].destroy)
			m->tasks[i].destroy(m->tasks[i].ctx);
	if (m->tasks)
		fun_memory_free((Memory *)&m->tasks);

	if (m->ctx_embed) fun_memory_free((Memory *)&m->ctx_embed);
	if (m->ctx_rope) fun_memory_free((Memory *)&m->ctx_rope);
	if (m->ctx_embed_residual) fun_memory_free((Memory *)&m->ctx_embed_residual);
	if (m->ctx_out_norm) fun_memory_free((Memory *)&m->ctx_out_norm);
	if (m->ctx_logits) fun_memory_free((Memory *)&m->ctx_logits);
	for (int l = 0; l < m->config.n_layers; l++) {
		if (m->ctx_norms && m->ctx_norms[l]) fun_memory_free((Memory *)&m->ctx_norms[l]);
		if (m->ctx_q && m->ctx_q[l]) fun_memory_free((Memory *)&m->ctx_q[l]);
		if (m->ctx_k && m->ctx_k[l]) fun_memory_free((Memory *)&m->ctx_k[l]);
		if (m->ctx_v && m->ctx_v[l]) fun_memory_free((Memory *)&m->ctx_v[l]);
		if (m->ctx_rq && m->ctx_rq[l]) fun_memory_free((Memory *)&m->ctx_rq[l]);
		if (m->ctx_rk && m->ctx_rk[l]) fun_memory_free((Memory *)&m->ctx_rk[l]);
		if (m->ctx_kv && m->ctx_kv[l]) fun_memory_free((Memory *)&m->ctx_kv[l]);
		if (m->ctx_attn && m->ctx_attn[l]) fun_memory_free((Memory *)&m->ctx_attn[l]);
		if (m->ctx_o && m->ctx_o[l]) fun_memory_free((Memory *)&m->ctx_o[l]);
		if (m->ctx_res && m->ctx_res[l]) fun_memory_free((Memory *)&m->ctx_res[l]);
		if (m->ctx_copy_attn && m->ctx_copy_attn[l]) fun_memory_free((Memory *)&m->ctx_copy_attn[l]);
		if (m->ctx_norms2 && m->ctx_norms2[l]) fun_memory_free((Memory *)&m->ctx_norms2[l]);
		if (m->ctx_router && m->ctx_router[l]) fun_memory_free((Memory *)&m->ctx_router[l]);
		if (m->ctx_acc && m->ctx_acc[l]) fun_memory_free((Memory *)&m->ctx_acc[l]);
		if (m->ctx_copy_res && m->ctx_copy_res[l]) fun_memory_free((Memory *)&m->ctx_copy_res[l]);
		for (int e = 0; e < 4; e++)
			if (m->ctx_expert && m->ctx_expert[l * 4 + e]) fun_memory_free((Memory *)&m->ctx_expert[l * 4 + e]);
	}
	fun_memory_free((Memory *)&m->ctx_norms);
	fun_memory_free((Memory *)&m->ctx_q);
	fun_memory_free((Memory *)&m->ctx_k);
	fun_memory_free((Memory *)&m->ctx_v);
	fun_memory_free((Memory *)&m->ctx_rq);
	fun_memory_free((Memory *)&m->ctx_rk);
	fun_memory_free((Memory *)&m->ctx_kv);
	fun_memory_free((Memory *)&m->ctx_attn);
	fun_memory_free((Memory *)&m->ctx_o);
	fun_memory_free((Memory *)&m->ctx_res);
	fun_memory_free((Memory *)&m->ctx_copy_attn);
	fun_memory_free((Memory *)&m->ctx_norms2);
	fun_memory_free((Memory *)&m->ctx_router);
	fun_memory_free((Memory *)&m->ctx_acc);
	fun_memory_free((Memory *)&m->ctx_copy_res);
	if (m->ctx_expert) fun_memory_free((Memory *)&m->ctx_expert);

	if (m->scratch_mem) {
		scratch_free((Scratch *)m->scratch_mem);
		fun_memory_free((Memory *)&m->scratch_mem);
	}
	if (m->logits) fun_memory_free((Memory *)&m->logits);
	if (m->layers) fun_memory_free((Memory *)&m->layers);
	if (m->k_cache) {
		for (int i = 0; i < m->config.n_layers; i++)
			fun_memory_free((Memory *)&m->k_cache[i]);
		fun_memory_free((Memory *)&m->k_cache);
	}
	if (m->v_cache) {
		for (int i = 0; i < m->config.n_layers; i++)
			fun_memory_free((Memory *)&m->v_cache[i]);
		fun_memory_free((Memory *)&m->v_cache);
	}
}
