#include "model.h"
#include "fundamental/compute/compute.h"
#include "fundamental/math/math.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include "fundamental/trace/trace.h"
#include <immintrin.h>
#include <stdint.h>

#define MAX_SEQ 256

static FunSpanId SPAN_INFERENCE;
static FunSpanId SPAN_ATTENTION;
static FunSpanId SPAN_EXPERT;
static FunSpanId SPAN_ROUTER;
static FunSpanId SPAN_OUTPUT;
static FunSpanId SPAN_EMBED;
static FunSpanId SPAN_ROPE;

/* Per-submit dynamic state passed through fun_compute_graph_submit to each
 * task's bind callback. token_id and pos change every forward step; everything
 * else in the task contexts is static across steps. */
typedef struct {
	int token_id;
	int pos;
} SubmitCtx;

/* RMS-norm task context: out[i] = x[i] / sqrt(mean(x^2) + eps) * weight[i]. */
typedef struct {
	float *x;             /* in [n], also the residual input */
	const float *weight;  /* in [n] gain */
	float *out;           /* out [n] */
	size_t n;
	float eps;
} CtxRmsNorm;

/* Q8_0 matvec task context: out = W·x + bias. w is the packed Q8_0 weight
 * matrix (rows x cols, raw pointer into the GGUF blob); bias is an f32 bias
 * vector [rows], may be NULL. */
typedef struct {
	const uint8_t *w;      /* weight matrix, rows x cols packed Q8_0 */
	const float *bias;     /* bias vector [rows], may be NULL */
	float *x;              /* input [cols] */
	float *out;            /* output [rows] */
	size_t rows;
	size_t cols;
} CtxMatvecQ8;

/* Row-sharded Q8_0 matvec: out[row_start..row_end) = W[row_start..row_end)·x
 * + bias[row_start..row_end) (bias may be NULL). Used to shard the Q and O
 * attention projections (the two largest serial matvecs per layer) so they can
 * run in parallel across the thread pool. */
typedef struct {
	const uint8_t *w;      /* rows x cols packed Q8_0 */
	const float *bias;     /* [rows], may be NULL */
	float *x;              /* input [cols] */
	float *out;            /* output [rows] */
	size_t row_start;
	size_t row_end;
	size_t cols;
} CtxMatvecQ8Shard;

/* Rotary-position task context: applies per-head RoPE to x in place. cos/sin
 * are the per-half-dim rotation tables precomputed by _exec_rope_pre. */
typedef struct {
	float *x;              /* in/out [n_heads * 2*half] */
	const float *cos;      /* [half] */
	const float *sin;      /* [half] */
	size_t n_heads;
	size_t half;           /* head_dim / 2 */
} CtxRotary;

/* KV-cache store task: writes this token's rotated K and V into the layer's
 * cache at row pos. kv_dim = n_kv_heads * head_dim. */
typedef struct {
	float *kbuf;     /* in [kv_dim] */
	float *vbuf;     /* in [kv_dim] */
	float *kcache;   /* out [max_seq][kv_dim], row pos written */
	float *vcache;   /* out [max_seq][kv_dim], row pos written */
	size_t kv_dim;
	int pos;
} CtxKvStore;

/* Attention task (whole-layer variant): scores q against all cached keys up to
 * pos, applies attention-sink bias and softmax, then weights cached values. */
typedef struct {
	float *qbuf;       /* in [n_heads * hd] */
	float *attn_out;   /* out [n_heads * hd] */
	float *kcache;     /* [max_seq][n_kv*hd] */
	float *vcache;     /* [max_seq][n_kv*hd] */
	const float *sinks;/* [n_heads] */
	int pos;
	int n_heads;
	int n_kv;
	int hd;
	float inv_sqrt_hd;
	float *scores;     /* scratch [max_seq+2] */
} CtxAttention;

/* Attention task sharded over a contiguous range of query heads [head_start,
 * head_end). Each shard is an independent task so 4 heads can run in parallel. */
typedef struct {
	float *qbuf;
	float *attn_out;
	float *kcache;
	float *vcache;
	const float *sinks;
	int pos;
	int head_start;
	int head_end;
	int n_heads;
	int n_kv;
	int hd;
	float inv_sqrt_hd;
	float *scores;
} CtxAttentionGroup;

/* Elementwise add or copy task (reused for residual adds and buffer copies via
 * _exec_add / _exec_copy). */
typedef struct {
	float *a;
	float *b;    /* may alias a */
	float *out;
	size_t n;
} CtxAdd;

/* Fused post-attention block: hidden = residual + proj, snapshot attn_res =
 * hidden, then hidden_normed = rms_norm(hidden). Fuses the three serial
 * elementwise steps that follow the O projection into one task. */
typedef struct {
	float *residual;         /* in [n] */
	float *proj;             /* in [n] */
	float *hidden;           /* out [n] */
	float *attn_res;         /* out [n] */
	const float *norm_weight;/* in [n] */
	float *normed;           /* out [n] */
	size_t n;
	float eps;
} CtxPostAttn;

/* Fused post-MoE block: mix the 4 expert outputs by routing weights, add the
 * attention residual, and write the result to both hidden and residual. Fuses
 * the expert-accumulate and residual-copy steps. */
typedef struct {
	float *dv0, *dv1, *dv2, *dv3; /* in [n] per-expert outputs */
	float *rweights;              /* in [4] routing weights */
	float *attn_res;              /* in [n] */
	float *hidden;                /* out [n] */
	float *residual;              /* out [n] */
	size_t n;
} CtxPostMoe;

/* Router task: computes the per-expert logits, keeps the top-k experts, and
 * softmaxes their logits into routing weights. */
typedef struct {
	const float *router_w; /* [n_exp][hs] */
	const float *router_b; /* [n_exp] */
	float *x;              /* input [hs] */
	int *topk;             /* out [topk_n] expert indices */
	float *rlog;           /* scratch [n_exp] raw logits */
	float *rweights;       /* out [topk_n] softmax weights */
	int n_exp;
	int topk_n;
	int hs;
} CtxRouter;

/* Expert task: runs one selected expert's SwiGLU MLP. Weights are MXFP4-packed
 * (17 bytes / 32 elements). hidden_in is the post-attn RMS-normed residual. */
typedef struct {
	const uint8_t *gate_w; /* [n_experts][ffn][hs] packed MXFP4 */
	const uint8_t *up_w;   /* [n_experts][ffn][hs] packed MXFP4 */
	const uint8_t *down_w; /* [n_experts][hs][ffn] packed MXFP4 */
	const float *gate_b;   /* [n_experts][ffn] */
	const float *up_b;     /* [n_experts][ffn] */
	const float *down_b;   /* [n_experts][hs] */
	float *hidden_in;      /* input [hs] */
	float *gv;             /* scratch [ffn] gate act = gate·x+b */
	float *uv;             /* scratch [ffn] up act = up·x+b */
	float *mid;            /* scratch [ffn] SwiGLU output */
	float *eg;             /* scratch [ffn] exp(-1.702·clamp(g)) */
	float *dv;             /* out [hs] down·mid + down_b */
	size_t ffn;
	size_t hs;
	int *topk;             /* which experts were routed (indexed by expert_idx) */
	int expert_idx;        /* 0..3, which slot of topk this task serves */
} CtxExpert;

/* Token embedding task: dequantizes the Q8_0 embedding row for token_id into
 * hidden[0..hs). emb_stride is the byte stride between rows. */
typedef struct {
	const uint8_t *embeddings; /* vocab x hs packed Q8_0 */
	size_t emb_stride;
	float *hidden;             /* out [hs] */
	size_t hs;
	int token_id;
} CtxEmbed;

/* RoPE precompute task: fills cos/sin rotation tables for position pos using
 * the precomputed inverse frequencies, scaled by mscale (YaRN length factor). */
typedef struct {
	float *theta;         /* scratch [half] = pos * rope_pre */
	float *cos;           /* out [half] */
	float *sin;           /* out [half] */
	const float *rope_pre;/* [half] */
	size_t half;
	float mscale;
	int pos;
} CtxRopePre;

/* Output projection (whole-vocab variant): logits = output_weight · x. */
typedef struct {
	const uint8_t *w; /* vocab x hs packed Q8_0 */
	float *x;         /* input [hs] */
	float *out;       /* out [vocab] */
	size_t vocab;
	size_t hs;
} CtxOutputProj;

/* Output projection sharded over vocabulary rows [row_start, row_end), so the
 * full 201088-row output matvec is split into 4 parallel tasks. */
typedef struct {
	const uint8_t *w;
	float *x;
	float *out;
	size_t row_start;
	size_t row_end;
	size_t vocab;
	size_t hs;
} CtxOutputGroup;

/* All per-token working buffers, shared across the whole graph (one pass
 * reuses the same hidden/attn/expert buffers layer after layer). The [4]
 * arrays give each expert / attention shard its own slice so they can run in
 * parallel without stepping on each other. */
typedef struct {
	float *hidden;        /* [hs]: running hidden state */
	float *residual;      /* [hs]: residual stream */
	float *attn_res;      /* [hs]: attention residual saved for MoE add */
	float *hidden_normed; /* [hs]: RMS-norm output */
	float *expert_out;    /* [hs]: weighted expert sum */
	float *qbuf;          /* [q_dim] */
	float *kbuf;          /* [kv_dim] */
	float *vbuf;          /* [kv_dim] */
	float *attn;          /* [q_dim]: attention output */
	float *proj;          /* [hs]: O-projection output */
	float *scores[4];     /* [MAX_SEQ+2] per attention shard */
	float *gv[4];         /* [ffn] per expert */
	float *uv[4];         /* [ffn] per expert */
	float *mid[4];        /* [ffn] per expert */
	float *eg[4];         /* [ffn] per expert */
	float *dv[4];         /* [hs] per expert */
	float *rlog;          /* [n_experts] */
	int *topk;            /* [4] */
	float *rweights;      /* [4] */
	float *theta;         /* [half] */
	float *cos;           /* [half] */
	float *sin;           /* [half] */
} Scratch;

/* RMS-normalize the residual: out[i] = x[i] / rms(x) * weight[i]. */
static void _exec_rms_norm(void *ctx)
{
	fun_trace_span_begin(SPAN_ATTENTION);
	CtxRmsNorm *c = (CtxRmsNorm *)ctx;
	fun_math_rms_norm_f32(c->x, c->weight, c->out, c->n, c->eps);
	fun_trace_span_end(SPAN_ATTENTION);
}

/* Q8_0 matvec: out = W·x + bias.
 * fun_math_q8_matrix_vector_f32 multiplies the Q8_0-packed matrix W
 * (rows x cols) by column vector x (cols) into out (rows); the f32 bias
 * (rows, may be NULL) is added afterwards. Used for Q/K/V (4096/512 x 2880)
 * and the O projection (2880 x 4096). */
static void _exec_matvec_q8(void *ctx)
{
	fun_trace_span_begin(SPAN_ATTENTION);
	CtxMatvecQ8 *c = (CtxMatvecQ8 *)ctx;
	fun_math_q8_matrix_vector_f32(c->w, c->x, c->out, c->rows, c->cols);
	if (c->bias)
		for (size_t i = 0; i < c->rows; i++)
			c->out[i] += c->bias[i];
	fun_trace_span_end(SPAN_ATTENTION);
}

/* Row-sharded variant of _exec_matvec_q8: offsets the packed weight pointer by
 * row_start and writes into out[row_start..], adding bias[row_start..] when a
 * bias is present. */
static void _exec_matvec_q8_shard(void *ctx)
{
	fun_trace_span_begin(SPAN_ATTENTION);
	CtxMatvecQ8Shard *c = (CtxMatvecQ8Shard *)ctx;
	size_t rows = c->row_end - c->row_start;
	size_t blocks = c->cols / FUN_MATH_Q8_BLOCK_ELEMS;
	const uint8_t *w = c->w + c->row_start * blocks * FUN_MATH_Q8_BLOCK_BYTES;
	float *out = c->out + c->row_start;
	fun_math_q8_matrix_vector_f32(w, c->x, out, rows, c->cols);
	if (c->bias)
		for (size_t i = 0; i < rows; i++)
			out[i] += c->bias[c->row_start + i];
	fun_trace_span_end(SPAN_ATTENTION);
}

/* Apply RoPE to Q or K in place: for each head, pairs (x[2j], x[2j+1]) are
 * rotated by angle cos[j]/sin[j]. x doubles as both input and output. */
static void _exec_rotary(void *ctx)
{
	fun_trace_span_begin(SPAN_ATTENTION);
	CtxRotary *c = (CtxRotary *)ctx;
	fun_math_rotary_f32(c->x, c->cos, c->sin, c->x, c->n_heads, c->half);
	fun_trace_span_end(SPAN_ATTENTION);
}

/* Copy this token's rotated K and V into the layer KV cache at row pos, so
 * future tokens can attend to it. Elementwise loop over kv_dim entries. */
static void _exec_kvstore(void *ctx)
{
	fun_trace_span_begin(SPAN_ATTENTION);
	CtxKvStore *c = (CtxKvStore *)ctx;
	size_t kv_dim = c->kv_dim;
	int pos = c->pos;
	for (size_t i = 0; i < kv_dim; i++) {
		c->kcache[pos * kv_dim + i] = c->kbuf[i];
		c->vcache[pos * kv_dim + i] = c->vbuf[i];
	}
	fun_trace_span_end(SPAN_ATTENTION);
}

/* Attention for one shard of query heads. For each head h:
 *  - rows_dot: scores[t] = dot(q[h], k_cache[kvh][t]) * inv_sqrt_hd for the
 *    pos+1 cached positions (q/kv strides over the shared kv_dim rows).
 *  - append the attention-sink bias scores[pos+1] = sinks[h].
 *  - softmax over the pos+2 scores.
 *  - weighted_sum: attn_out[h] = sum_t scores[t] * v_cache[kvh][t].
 * kvh maps each query head to its GQA KV head via h * n_kv / n_heads. */
static void _exec_attention_group(void *ctx)
{
	fun_trace_span_begin(SPAN_ATTENTION);
	CtxAttentionGroup *c = (CtxAttentionGroup *)ctx;
	int pos = c->pos;
	int hd = c->hd;
	int kv_dim = c->n_kv * hd;
	float *scores = c->scores;
	for (int h = c->head_start; h < c->head_end; h++) {
		int kvh = h * c->n_kv / c->n_heads;
		float *qh = c->qbuf + (size_t)h * (size_t)hd;
		fun_math_rows_dot_f32(qh, c->kcache + kvh * hd, scores,
							  (size_t)(pos + 1), (size_t)hd, (size_t)kv_dim,
							  c->inv_sqrt_hd);
		scores[pos + 1] = c->sinks[h];
		fun_math_softmax_f32(scores, (size_t)(pos + 2));
		fun_math_weighted_sum_f32(scores, c->vcache + kvh * hd,
								  c->attn_out + h * hd, (size_t)(pos + 1),
								  (size_t)hd, (size_t)kv_dim);
	}
	fun_trace_span_end(SPAN_ATTENTION);
}

/* Elementwise copy: out[i] = a[i] (snapshot of a buffer for later use). */
static void _exec_copy(void *ctx)
{
	fun_trace_span_begin(SPAN_ATTENTION);
	CtxAdd *c = (CtxAdd *)ctx;
	for (size_t i = 0; i < c->n; i++)
		c->out[i] = c->a[i];
	fun_trace_span_end(SPAN_ATTENTION);
}

/* Fused post-attention block (see CtxPostAttn). */
static void _exec_post_attn(void *ctx)
{
	fun_trace_span_begin(SPAN_ATTENTION);
	CtxPostAttn *c = (CtxPostAttn *)ctx;
	for (size_t i = 0; i < c->n; i++) {
		float h = c->residual[i] + c->proj[i];
		c->hidden[i] = h;
		c->attn_res[i] = h;
	}
	fun_math_rms_norm_f32(c->hidden, c->norm_weight, c->normed, c->n, c->eps);
	fun_trace_span_end(SPAN_ATTENTION);
}

/* Fused post-MoE block (see CtxPostMoe). */
static void _exec_post_moe(void *ctx)
{
	fun_trace_span_begin(SPAN_ATTENTION);
	CtxPostMoe *c = (CtxPostMoe *)ctx;
	const float *dv[4] = { c->dv0, c->dv1, c->dv2, c->dv3 };
	for (size_t i = 0; i < c->n; i++) {
		float e = c->rweights[0] * dv[0][i] + c->rweights[1] * dv[1][i] +
				  c->rweights[2] * dv[2][i] + c->rweights[3] * dv[3][i];
		float h = c->attn_res[i] + e;
		c->hidden[i] = h;
		c->residual[i] = h;
	}
	fun_trace_span_end(SPAN_ATTENTION);
}

/* MoE router: rlog[e] = dot(router_w[e], x) + router_b[e] for all experts,
 * select the top-k by value (a 4-element insertion max-heap), then softmax the
 * top-k logits (numerically stable by subtracting the max) into rweights. */
static void _exec_router(void *ctx)
{
	fun_trace_span_begin(SPAN_ROUTER);
	CtxRouter *c = (CtxRouter *)ctx;
	int n_exp = c->n_exp;
	fun_math_matrix_vector_f32(c->router_w, c->x, c->router_b, c->rlog,
							   (size_t)n_exp, (size_t)c->hs);
	int topk = c->topk_n;
	int top[4] = { 0, 0, 0, 0 };
	float topv[4] = { -1e30f, -1e30f, -1e30f, -1e30f };
	for (int e = 0; e < n_exp; e++) {
		float v = c->rlog[e];
		if (v > topv[0]) {
			topv[3] = topv[2];
			top[3] = top[2];
			topv[2] = topv[1];
			top[2] = top[1];
			topv[1] = topv[0];
			top[1] = top[0];
			topv[0] = v;
			top[0] = e;
		} else if (v > topv[1]) {
			topv[3] = topv[2];
			top[3] = top[2];
			topv[2] = topv[1];
			top[2] = top[1];
			topv[1] = v;
			top[1] = e;
		} else if (v > topv[2]) {
			topv[3] = topv[2];
			top[3] = top[2];
			topv[2] = v;
			top[2] = e;
		} else if (v > topv[3]) {
			topv[3] = v;
			top[3] = e;
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
	fun_trace_span_end(SPAN_ROUTER);
}

/* Run one routed expert's SwiGLU MLP:
 *  - gv = gate_w(ex)·hidden_in + gate_b  [MXFP4 matvec ffn x hs]
 *  - uv = up_w(ex)·hidden_in  + up_b     [MXFP4 matvec ffn x hs]
 *  - SwiGLU: mid = clamp(g,7) / (1 + exp(-1.702·clamp(g))) * (clamp(u,±7)+1)
 *    (the /(1+eg) form reuses eg = exp(-1.702·g) computed once).
 *  - dv = down_w(ex)·mid + down_b        [MXFP4 matvec hs x ffn]
 * The expert's weights are located by offsetting into the packed arrays by
 * ex * (ffn*hs*17/32) bytes (MXFP4 = 17 bytes per 32 elements). */
static void _exec_expert(void *ctx)
{
	fun_trace_span_begin(SPAN_EXPERT);
	CtxExpert *c = (CtxExpert *)ctx;
	size_t ffn = c->ffn;
	size_t hs = c->hs;
	int ex = c->topk[c->expert_idx];
	size_t exp_w_bytes = (size_t)ffn * (size_t)hs * FUN_MATH_MXFP4_BLOCK_BYTES /
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
		if (g > 7.0f)
			g = 7.0f;
		c->eg[i] = -1.702f * g;
	}
	fun_math_exp_f32(c->eg, c->eg, ffn);
	for (size_t i = 0; i < ffn; i++) {
		float g = c->gv[i];
		if (g > 7.0f)
			g = 7.0f;
		float u = c->uv[i];
		if (u > 7.0f)
			u = 7.0f;
		if (u < -7.0f)
			u = -7.0f;
		c->mid[i] = g / (1.0f + c->eg[i]) * (u + 1.0f);
	}
	fun_math_matrix_vector_mxfp4_f32(dw, c->mid, db, c->dv, hs, ffn);
	fun_trace_span_attribute_i64(SPAN_EXPERT, "expert_idx", c->expert_idx);
	fun_trace_span_end(SPAN_EXPERT);
}

/* Dequantize the token embedding row for token_id from Q8_0 into hidden. */
static void _exec_embed(void *ctx)
{
	fun_trace_span_begin(SPAN_EMBED);
	CtxEmbed *c = (CtxEmbed *)ctx;
	fun_math_q8_dequant_row_f32(c->embeddings +
									(size_t)(c->token_id) * c->emb_stride,
								c->hidden, c->hs);
	fun_trace_span_end(SPAN_EMBED);
}

/* Precompute RoPE rotation tables for position pos: theta[j] = pos*rope_pre[j],
 * cos/sin of theta, then both scaled by mscale (YaRN length-scaling factor). */
static void _exec_rope_pre(void *ctx)
{
	fun_trace_span_begin(SPAN_ROPE);
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
	fun_trace_span_end(SPAN_ROPE);
}

/* Whole-vocab output projection: logits = output_weight · x.
 * fun_math_q8_matrix_vector_f32 multiplies the Q8_0-packed matrix W
 * (vocab x hs) by x (hs) into out (vocab); Q8_0 = 34 bytes / 32 elements. */
static void _exec_output_proj(void *ctx)
{
	fun_trace_span_begin(SPAN_OUTPUT);
	CtxOutputProj *c = (CtxOutputProj *)ctx;
	fun_math_q8_matrix_vector_f32(c->w, c->x, c->out, c->vocab, c->hs);
	fun_trace_span_end(SPAN_OUTPUT);
}

/* Output projection restricted to vocabulary rows [row_start, row_end).
 * Offsets the packed weight pointer by row_start * (hs/32)*34 bytes and writes
 * into out[row_start..], so 4 shards cover the full 201088-row output matvec. */
static void _exec_output_group(void *ctx)
{
	fun_trace_span_begin(SPAN_OUTPUT);
	CtxOutputGroup *c = (CtxOutputGroup *)ctx;
	size_t rows = c->row_end - c->row_start;
	size_t blocks = c->hs / FUN_MATH_Q8_BLOCK_ELEMS;
	const uint8_t *w = c->w + c->row_start * blocks * FUN_MATH_Q8_BLOCK_BYTES;
	float *out = c->out + c->row_start;
	fun_math_q8_matrix_vector_f32(w, c->x, out, rows, c->hs);
	fun_trace_span_end(SPAN_OUTPUT);
}

/* Bind callbacks copy the per-submit token_id/pos into the static task
 * contexts just before execution. They run on the submitting thread at
 * fun_compute_graph_submit time. */

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

/* Residual snapshot: before the residual buffer is overwritten by the next
 * add, copy its current value into b so the add task can use it. */
static void _bind_residual(void *task_ctx, void *submit_ctx)
{
	CtxAdd *c = (CtxAdd *)task_ctx;
	(void)submit_ctx;
	for (size_t i = 0; i < c->n; i++)
		c->b[i] = c->a[i];
}

/* Allocate every per-token scratch buffer into sc. Sizes come from config:
 * hidden, residual, norm outputs (hs); Q/K/V buffers (n_heads*head_dim and
 * n_kv_heads*head_dim); attention scores (MAX_SEQ+2); expert activations
 * (ffn or hs); router/topk/rweights; and RoPE tables (head_dim/2). */
static void scratch_alloc(Scratch *sc, ModelConfig *cfg)
{
	int hs = (int)cfg->hidden_size;
	int ffn = (int)cfg->ffn_size;
	int half = (int)cfg->head_dim / 2;
	int q_dim = (int)cfg->n_heads * (int)cfg->head_dim;
	int kv_dim = (int)cfg->n_kv_heads * (int)cfg->head_dim;
	MemoryResult mr;
	mr = fun_memory_allocate((size_t)hs * sizeof(float));
	sc->hidden = (float *)mr.value;
	mr = fun_memory_allocate((size_t)hs * sizeof(float));
	sc->residual = (float *)mr.value;
	mr = fun_memory_allocate((size_t)hs * sizeof(float));
	sc->attn_res = (float *)mr.value;
	mr = fun_memory_allocate((size_t)hs * sizeof(float));
	sc->hidden_normed = (float *)mr.value;
	mr = fun_memory_allocate((size_t)hs * sizeof(float));
	sc->expert_out = (float *)mr.value;
	mr = fun_memory_allocate((size_t)q_dim * sizeof(float));
	sc->qbuf = (float *)mr.value;
	mr = fun_memory_allocate((size_t)kv_dim * sizeof(float));
	sc->kbuf = (float *)mr.value;
	mr = fun_memory_allocate((size_t)kv_dim * sizeof(float));
	sc->vbuf = (float *)mr.value;
	mr = fun_memory_allocate((size_t)q_dim * sizeof(float));
	sc->attn = (float *)mr.value;
	mr = fun_memory_allocate((size_t)hs * sizeof(float));
	sc->proj = (float *)mr.value;
	for (int e = 0; e < 4; e++) {
		mr = fun_memory_allocate((size_t)(MAX_SEQ + 2) * sizeof(float));
		sc->scores[e] = (float *)mr.value;
	}
	for (int e = 0; e < 4; e++) {
		mr = fun_memory_allocate((size_t)ffn * sizeof(float));
		sc->gv[e] = (float *)mr.value;
		mr = fun_memory_allocate((size_t)ffn * sizeof(float));
		sc->uv[e] = (float *)mr.value;
		mr = fun_memory_allocate((size_t)ffn * sizeof(float));
		sc->mid[e] = (float *)mr.value;
		mr = fun_memory_allocate((size_t)ffn * sizeof(float));
		sc->eg[e] = (float *)mr.value;
		mr = fun_memory_allocate((size_t)hs * sizeof(float));
		sc->dv[e] = (float *)mr.value;
	}
	mr = fun_memory_allocate(cfg->n_experts * sizeof(float));
	sc->rlog = (float *)mr.value;
	mr = fun_memory_allocate(4 * sizeof(int));
	sc->topk = (int *)mr.value;
	mr = fun_memory_allocate(4 * sizeof(float));
	sc->rweights = (float *)mr.value;
	mr = fun_memory_allocate((size_t)half * sizeof(float));
	sc->theta = (float *)mr.value;
	mr = fun_memory_allocate((size_t)half * sizeof(float));
	sc->cos = (float *)mr.value;
	mr = fun_memory_allocate((size_t)half * sizeof(float));
	sc->sin = (float *)mr.value;
}

/* Inverse of scratch_alloc: frees every buffer (via Memory* casts). */
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
	for (int e = 0; e < 4; e++)
		fun_memory_free((Memory *)&sc->scores[e]);
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

/* Build the whole inference engine: load config, locate GGUF tensor offsets,
 * dequantize eager weights (norms/router/sinks/output norm), allocate KV caches
 * and scratch, and construct the once-built compute graph that will be replayed
 * every token. Attention Q/K/V/O weights and biases are raw GGUF pointers. */
void model_load(Model *m, GGufFile *gguf, const char *model_path, int n_threads)
{
	m->gguf = gguf;
	fun_string_copy(model_path, m->model_path, sizeof(m->model_path));

	SPAN_INFERENCE = fun_trace_span_register("inference");
	SPAN_ATTENTION = fun_trace_span_register("attention");
	SPAN_EXPERT = fun_trace_span_register("expert");
	SPAN_ROUTER = fun_trace_span_register("router");
	SPAN_OUTPUT = fun_trace_span_register("output");
	SPAN_EMBED = fun_trace_span_register("embed");
	SPAN_ROPE = fun_trace_span_register("rope");
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

	m->layers = (LayerWeights *)fun_memory_allocate(m->config.n_layers *
													sizeof(LayerWeights))
					.value;
	for (int i = 0; i < m->config.n_layers; i++) {
		m->layers[i].attn_norm_weight = NULL;
		m->layers[i].post_attn_norm_weight = NULL;
		m->layers[i].router_weight = NULL;
		m->layers[i].router_bias = NULL;
		m->layers[i].sinks = NULL;
	}

	int kv_dim = m->config.n_kv_heads * m->config.head_dim;
	int max_seq = m->config.max_seq_len;
	m->k_cache =
		(float **)fun_memory_allocate(m->config.n_layers * sizeof(float *))
			.value;
	m->v_cache =
		(float **)fun_memory_allocate(m->config.n_layers * sizeof(float *))
			.value;
	for (int i = 0; i < m->config.n_layers; i++) {
		m->k_cache[i] = (float *)fun_memory_allocate((size_t)max_seq * kv_dim *
													 sizeof(float))
							.value;
		m->v_cache[i] = (float *)fun_memory_allocate((size_t)max_seq * kv_dim *
													 sizeof(float))
							.value;
	}
	m->cached_len = 0;

	/* RoPE inverse frequencies. theta_scale = theta^(-2/head_dim) so successive
	 * dims decay geometrically; a YaRN-style corr ramp shrinks long-range dims
	 * by 1/32 to tame high-frequency extrapolation. */
	int half = m->config.head_dim / 2;
	m->rope_pre =
		(float *)fun_memory_allocate((size_t)half * sizeof(float)).value;
	float theta_scale = fun_math_exp(fun_math_log(m->config.rope_theta) *
									 (-2.0f / (float)m->config.head_dim));
	int corr0 = 8, corr1 = 18;
	float power = 1.0f;
	for (int j = 0; j < half; j++) {
		float y = (float)(j - corr0) / (float)(corr1 - corr0);
		if (y < 0.0f)
			y = 0.0f;
		if (y > 1.0f)
			y = 1.0f;
		float ramp = 1.0f - y;
		float factor = ramp + (1.0f - ramp) / 32.0f;
		m->rope_pre[j] = power * factor;
		power *= theta_scale;
	}

	/* Load tensor offsets and eager weights. Attention Q/K/V/O weights are
	 * raw Q8_0 pointers into the GGUF (no dequant); their biases are raw f32
	 * pointers. Norms, router and sinks are dequantized eagerly. */
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
		w->attn_norm_weight =
			(float *)fun_memory_allocate(m->config.hidden_size * sizeof(float))
				.value;
		fun_gguf_dequant_f32(m->gguf, buf, w->attn_norm_weight);

		buf[pre_len] = '\0';
		fun_string_copy(".post_attention_norm.weight", buf + pre_len,
						256 - pre_len);
		w->post_attn_norm_weight =
			(float *)fun_memory_allocate(m->config.hidden_size * sizeof(float))
				.value;
		fun_gguf_dequant_f32(m->gguf, buf, w->post_attn_norm_weight);

		buf[pre_len] = '\0';
		fun_string_copy(".ffn_gate_inp.weight", buf + pre_len, 256 - pre_len);
		w->router_weight =
			(float *)fun_memory_allocate(m->config.n_experts *
										 m->config.hidden_size * sizeof(float))
				.value;
		fun_gguf_dequant_f32(m->gguf, buf, w->router_weight);

		buf[pre_len] = '\0';
		fun_string_copy(".ffn_gate_inp.bias", buf + pre_len, 256 - pre_len);
		w->router_bias =
			(float *)fun_memory_allocate(m->config.n_experts * sizeof(float))
				.value;
		fun_gguf_dequant_f32(m->gguf, buf, w->router_bias);

		buf[pre_len] = '\0';
		fun_string_copy(".attn_sinks.weight", buf + pre_len, 256 - pre_len);
		w->sinks =
			(float *)fun_memory_allocate(m->config.n_heads * sizeof(float))
				.value;
		fun_gguf_dequant_f32(m->gguf, buf, w->sinks);

		buf[pre_len] = '\0';
		fun_string_copy(".attn_q.weight", buf + pre_len, 256 - pre_len);
		w->q_weight = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;
		buf[pre_len] = '\0';
		fun_string_copy(".attn_q.bias", buf + pre_len, 256 - pre_len);
		w->q_bias =
			(const float *)(rd +
							fun_gguf_get_tensor_offset(m->gguf, buf).value);
		buf[pre_len] = '\0';
		fun_string_copy(".attn_k.weight", buf + pre_len, 256 - pre_len);
		w->k_weight = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;
		buf[pre_len] = '\0';
		fun_string_copy(".attn_k.bias", buf + pre_len, 256 - pre_len);
		w->k_bias =
			(const float *)(rd +
							fun_gguf_get_tensor_offset(m->gguf, buf).value);
		buf[pre_len] = '\0';
		fun_string_copy(".attn_v.weight", buf + pre_len, 256 - pre_len);
		w->v_weight = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;
		buf[pre_len] = '\0';
		fun_string_copy(".attn_v.bias", buf + pre_len, 256 - pre_len);
		w->v_bias =
			(const float *)(rd +
							fun_gguf_get_tensor_offset(m->gguf, buf).value);
		buf[pre_len] = '\0';
		fun_string_copy(".attn_output.weight", buf + pre_len, 256 - pre_len);
		w->o_weight = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;
		buf[pre_len] = '\0';
		fun_string_copy(".attn_output.bias", buf + pre_len, 256 - pre_len);
		w->o_bias =
			(const float *)(rd +
							fun_gguf_get_tensor_offset(m->gguf, buf).value);

		buf[pre_len] = '\0';
		fun_string_copy(".ffn_gate_exps.weight", buf + pre_len, 256 - pre_len);
		w->experts.gate_w = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;
		buf[pre_len] = '\0';
		fun_string_copy(".ffn_gate_exps.bias", buf + pre_len, 256 - pre_len);
		w->experts.gate_b =
			(const float *)(rd +
							fun_gguf_get_tensor_offset(m->gguf, buf).value);
		buf[pre_len] = '\0';
		fun_string_copy(".ffn_up_exps.weight", buf + pre_len, 256 - pre_len);
		w->experts.up_w = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;
		buf[pre_len] = '\0';
		fun_string_copy(".ffn_up_exps.bias", buf + pre_len, 256 - pre_len);
		w->experts.up_b =
			(const float *)(rd +
							fun_gguf_get_tensor_offset(m->gguf, buf).value);
		buf[pre_len] = '\0';
		fun_string_copy(".ffn_down_exps.weight", buf + pre_len, 256 - pre_len);
		w->experts.down_w = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;
		buf[pre_len] = '\0';
		fun_string_copy(".ffn_down_exps.bias", buf + pre_len, 256 - pre_len);
		w->experts.down_b =
			(const float *)(rd +
							fun_gguf_get_tensor_offset(m->gguf, buf).value);
	}

	m->scratch_mem = fun_memory_allocate(sizeof(Scratch)).value;
	Scratch *s = (Scratch *)m->scratch_mem;
	scratch_alloc(s, &m->config);

	m->logits =
		(float *)fun_memory_allocate(m->config.vocab_size * sizeof(float)).value;

	int n_layers = m->config.n_layers;
	int max_tasks = 8 + n_layers * 40;
	int max_edges = 12 + n_layers * 80;
	size_t graph_bytes =
		fun_compute_graph_memory_required(max_tasks, max_edges, n_threads);
	m->graph_mem = fun_memory_allocate(graph_bytes).value;
	m->graph = fun_compute_graph_init(m->graph_mem, graph_bytes, max_tasks,
									  max_edges, n_threads);

	m->n_tasks = max_tasks;
	m->tasks = (FunComputeTask *)fun_memory_allocate((size_t)max_tasks *
													 sizeof(FunComputeTask))
				   .value;

	int ti = 0;
	float inv_sqrt_hd = 1.0f / fun_math_sqrt((float)m->config.head_dim);

	m->ctx_embed = fun_memory_allocate(sizeof(CtxEmbed)).value;
	*(CtxEmbed *)m->ctx_embed = (CtxEmbed){
		.embeddings = m->tok_embeddings,
		.emb_stride = (size_t)m->config.hidden_size / FUN_MATH_Q8_BLOCK_ELEMS *
					  FUN_MATH_Q8_BLOCK_BYTES,
		.hidden = s->hidden,
		.hs = m->config.hidden_size,
	};
	FunComputeTask *t_embed = &m->tasks[ti++];
	fun_compute_graph_add_task(m->graph, t_embed, _exec_embed, m->ctx_embed,
							   _bind_embed, NULL);

	m->ctx_rope = fun_memory_allocate(sizeof(CtxRopePre)).value;
	*(CtxRopePre *)m->ctx_rope = (CtxRopePre){
		.theta = s->theta,
		.cos = s->cos,
		.sin = s->sin,
		.rope_pre = m->rope_pre,
		.half = m->config.head_dim / 2,
		.mscale = m->rope_mscale,
	};
	FunComputeTask *t_rope = &m->tasks[ti++];
	fun_compute_graph_add_task(m->graph, t_rope, _exec_rope_pre, m->ctx_rope,
							   _bind_rope_pre, NULL);

	m->ctx_embed_residual = fun_memory_allocate(sizeof(CtxAdd)).value;
	*(CtxAdd *)m->ctx_embed_residual = (CtxAdd){
		.a = s->hidden,
		.b = s->residual,
		.out = s->residual,
		.n = m->config.hidden_size,
	};
	FunComputeTask *t_res_embed = &m->tasks[ti++];
	fun_compute_graph_add_task(m->graph, t_res_embed, _exec_copy,
							   m->ctx_embed_residual, _bind_residual, NULL);
	fun_compute_task_depends_on(m->graph, t_res_embed, t_embed);

	m->ctx_norms = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_q = fun_memory_allocate((size_t)n_layers * 4 * sizeof(void *)).value;
	m->ctx_k = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_v = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_rq = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_rk = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_kv = fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_o = fun_memory_allocate((size_t)n_layers * 4 * sizeof(void *)).value;
	m->ctx_post_attn =
		fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_router =
		fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_post_moe =
		fun_memory_allocate((size_t)n_layers * sizeof(void *)).value;
	m->ctx_expert =
		fun_memory_allocate((size_t)n_layers * 4 * sizeof(void *)).value;
	m->ctx_attn_groups =
		fun_memory_allocate((size_t)n_layers * 4 * sizeof(void *)).value;
	m->ctx_output_groups = fun_memory_allocate(4 * sizeof(void *)).value;
	for (int i = 0; i < n_layers * 4; i++) {
		((void **)m->ctx_attn_groups)[i] = NULL;
		((void **)m->ctx_q)[i] = NULL;
		((void **)m->ctx_o)[i] = NULL;
	}
	for (int i = 0; i < 4; i++)
		((void **)m->ctx_output_groups)[i] = NULL;

	FunComputeTask *prev_layer_tail = t_res_embed;

	for (int l = 0; l < n_layers; l++) {
		LayerWeights *w = &m->layers[l];

		FunComputeTask *t_norm = &m->tasks[ti++];
		m->ctx_norms[l] = fun_memory_allocate(sizeof(CtxRmsNorm)).value;
		*(CtxRmsNorm *)m->ctx_norms[l] = (CtxRmsNorm){
			.x = s->hidden,
			.weight = w->attn_norm_weight,
			.out = s->hidden_normed,
			.n = m->config.hidden_size,
			.eps = m->config.rms_norm_eps,
		};

		// (t_norm added, depends_on below)
		fun_compute_graph_add_task(m->graph, t_norm, _exec_rms_norm,
								   m->ctx_norms[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_norm, prev_layer_tail);

		FunComputeTask *t_q_sh[4];
		size_t q_rows = m->config.n_heads * m->config.head_dim;
		size_t q_chunk = q_rows / 4;
		for (int qs = 0; qs < 4; qs++) {
			t_q_sh[qs] = &m->tasks[ti++];
			int qi = l * 4 + qs;
			m->ctx_q[qi] = fun_memory_allocate(sizeof(CtxMatvecQ8Shard)).value;
			*(CtxMatvecQ8Shard *)m->ctx_q[qi] = (CtxMatvecQ8Shard){
				.w = w->q_weight,
				.bias = w->q_bias,
				.x = s->hidden_normed,
				.out = s->qbuf,
				.row_start = qs * q_chunk,
				.row_end = (qs == 3) ? q_rows : (size_t)(qs + 1) * q_chunk,
				.cols = m->config.hidden_size,
			};
			fun_compute_graph_add_task(m->graph, t_q_sh[qs],
									   _exec_matvec_q8_shard, m->ctx_q[qi],
									   NULL, NULL);
			fun_compute_task_depends_on(m->graph, t_q_sh[qs], t_norm);
		}

		FunComputeTask *t_k = &m->tasks[ti++];
		m->ctx_k[l] = fun_memory_allocate(sizeof(CtxMatvecQ8)).value;
		*(CtxMatvecQ8 *)m->ctx_k[l] = (CtxMatvecQ8){
			.w = w->k_weight,
			.bias = w->k_bias,
			.x = s->hidden_normed,
			.out = s->kbuf,
			.rows = m->config.n_kv_heads * m->config.head_dim,
			.cols = m->config.hidden_size,
		};
		fun_compute_graph_add_task(m->graph, t_k, _exec_matvec_q8, m->ctx_k[l],
								   NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_k, t_norm);

		FunComputeTask *t_v = &m->tasks[ti++];
		m->ctx_v[l] = fun_memory_allocate(sizeof(CtxMatvecQ8)).value;
		*(CtxMatvecQ8 *)m->ctx_v[l] = (CtxMatvecQ8){
			.w = w->v_weight,
			.bias = w->v_bias,
			.x = s->hidden_normed,
			.out = s->vbuf,
			.rows = m->config.n_kv_heads * m->config.head_dim,
			.cols = m->config.hidden_size,
		};
		fun_compute_graph_add_task(m->graph, t_v, _exec_matvec_q8, m->ctx_v[l],
								   NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_v, t_norm);

		FunComputeTask *t_rq = &m->tasks[ti++];
		m->ctx_rq[l] = fun_memory_allocate(sizeof(CtxRotary)).value;
		*(CtxRotary *)m->ctx_rq[l] = (CtxRotary){
			.x = s->qbuf,
			.cos = s->cos,
			.sin = s->sin,
			.n_heads = m->config.n_heads,
			.half = m->config.head_dim / 2,
		};
		fun_compute_graph_add_task(m->graph, t_rq, _exec_rotary, m->ctx_rq[l],
								   NULL, NULL);
		for (int qs = 0; qs < 4; qs++)
			fun_compute_task_depends_on(m->graph, t_rq, t_q_sh[qs]);

		FunComputeTask *t_rk = &m->tasks[ti++];
		m->ctx_rk[l] = fun_memory_allocate(sizeof(CtxRotary)).value;
		*(CtxRotary *)m->ctx_rk[l] = (CtxRotary){
			.x = s->kbuf,
			.cos = s->cos,
			.sin = s->sin,
			.n_heads = m->config.n_kv_heads,
			.half = m->config.head_dim / 2,
		};
		fun_compute_graph_add_task(m->graph, t_rk, _exec_rotary, m->ctx_rk[l],
								   NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_rk, t_k);

		FunComputeTask *t_kv = &m->tasks[ti++];
		m->ctx_kv[l] = fun_memory_allocate(sizeof(CtxKvStore)).value;
		*(CtxKvStore *)m->ctx_kv[l] = (CtxKvStore){
			.kbuf = s->kbuf,
			.vbuf = s->vbuf,
			.kcache = m->k_cache[l],
			.vcache = m->v_cache[l],
			.kv_dim = m->config.n_kv_heads * m->config.head_dim,
		};
		fun_compute_graph_add_task(m->graph, t_kv, _exec_kvstore, m->ctx_kv[l],
								   _bind_kvstore, NULL);
		fun_compute_task_depends_on(m->graph, t_kv, t_rk);
		fun_compute_task_depends_on(m->graph, t_kv, t_v);

		int heads_per = m->config.n_heads / 4;
		FunComputeTask *t_attn_h[4];
		for (int hg = 0; hg < 4; hg++) {
			t_attn_h[hg] = &m->tasks[ti++];
			int gi = l * 4 + hg;
			m->ctx_attn_groups[gi] =
				fun_memory_allocate(sizeof(CtxAttentionGroup)).value;
			*(CtxAttentionGroup *)m->ctx_attn_groups[gi] =
				(CtxAttentionGroup){ .qbuf = s->qbuf,
									 .attn_out = s->attn,
									 .kcache = m->k_cache[l],
									 .vcache = m->v_cache[l],
									 .sinks = w->sinks,
									 .head_start = hg * heads_per,
									 .head_end = (hg + 1) * heads_per,
									 .n_heads = m->config.n_heads,
									 .n_kv = m->config.n_kv_heads,
									 .hd = m->config.head_dim,
									 .inv_sqrt_hd = inv_sqrt_hd,
									 .scores = s->scores[hg] };
			fun_compute_graph_add_task(m->graph, t_attn_h[hg],
									   _exec_attention_group,
									   m->ctx_attn_groups[gi], _bind_attention,
									   NULL);
			fun_compute_task_depends_on(m->graph, t_attn_h[hg], t_rq);
			fun_compute_task_depends_on(m->graph, t_attn_h[hg], t_kv);
		}

		FunComputeTask *t_o_sh[4];
		size_t o_rows = m->config.hidden_size;
		size_t o_chunk = o_rows / 4;
		for (int os = 0; os < 4; os++) {
			t_o_sh[os] = &m->tasks[ti++];
			int oi = l * 4 + os;
			m->ctx_o[oi] = fun_memory_allocate(sizeof(CtxMatvecQ8Shard)).value;
			*(CtxMatvecQ8Shard *)m->ctx_o[oi] = (CtxMatvecQ8Shard){
				.w = w->o_weight,
				.bias = w->o_bias,
				.x = s->attn,
				.out = s->proj,
				.row_start = os * o_chunk,
				.row_end = (os == 3) ? o_rows : (size_t)(os + 1) * o_chunk,
				.cols = m->config.n_heads * m->config.head_dim,
			};
			fun_compute_graph_add_task(m->graph, t_o_sh[os],
									   _exec_matvec_q8_shard, m->ctx_o[oi],
									   NULL, NULL);
			for (int hg = 0; hg < 4; hg++)
				fun_compute_task_depends_on(m->graph, t_o_sh[os], t_attn_h[hg]);
		}

		FunComputeTask *t_post_attn = &m->tasks[ti++];
		m->ctx_post_attn[l] = fun_memory_allocate(sizeof(CtxPostAttn)).value;
		*(CtxPostAttn *)m->ctx_post_attn[l] = (CtxPostAttn){
			.residual = s->residual,
			.proj = s->proj,
			.hidden = s->hidden,
			.attn_res = s->attn_res,
			.norm_weight = w->post_attn_norm_weight,
			.normed = s->hidden_normed,
			.n = m->config.hidden_size,
			.eps = m->config.rms_norm_eps,
		};
		fun_compute_graph_add_task(m->graph, t_post_attn, _exec_post_attn,
								   m->ctx_post_attn[l], NULL, NULL);
		for (int os = 0; os < 4; os++)
			fun_compute_task_depends_on(m->graph, t_post_attn, t_o_sh[os]);

		FunComputeTask *t_router = &m->tasks[ti++];
		m->ctx_router[l] = fun_memory_allocate(sizeof(CtxRouter)).value;
		*(CtxRouter *)m->ctx_router[l] = (CtxRouter){
			.router_w = w->router_weight,
			.router_b = w->router_bias,
			.x = s->hidden_normed,
			.topk = s->topk,
			.rlog = s->rlog,
			.rweights = s->rweights,
			.n_exp = m->config.n_experts,
			.topk_n = m->config.n_active_experts,
			.hs = m->config.hidden_size,
		};
		fun_compute_graph_add_task(m->graph, t_router, _exec_router,
								   m->ctx_router[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_router, t_post_attn);

		FunComputeTask *t_e[4];
		for (int e = 0; e < 4; e++) {
			t_e[e] = &m->tasks[ti++];
			m->ctx_expert[l * 4 + e] =
				fun_memory_allocate(sizeof(CtxExpert)).value;
			*(CtxExpert *)m->ctx_expert[l * 4 + e] = (CtxExpert){
				.gate_w = w->experts.gate_w,
				.up_w = w->experts.up_w,
				.down_w = w->experts.down_w,
				.gate_b = w->experts.gate_b,
				.up_b = w->experts.up_b,
				.down_b = w->experts.down_b,
				.hidden_in = s->hidden_normed,
				.gv = s->gv[e],
				.uv = s->uv[e],
				.mid = s->mid[e],
				.eg = s->eg[e],
				.dv = s->dv[e],
				.ffn = m->config.ffn_size,
				.hs = m->config.hidden_size,
				.topk = s->topk,
				.expert_idx = e,
			};
			fun_compute_graph_add_task(m->graph, t_e[e], _exec_expert,
									   m->ctx_expert[l * 4 + e], NULL, NULL);
			fun_compute_task_depends_on(m->graph, t_e[e], t_router);
		}

		FunComputeTask *t_post_moe = &m->tasks[ti++];
		m->ctx_post_moe[l] = fun_memory_allocate(sizeof(CtxPostMoe)).value;
		*(CtxPostMoe *)m->ctx_post_moe[l] = (CtxPostMoe){
			.dv0 = s->dv[0],
			.dv1 = s->dv[1],
			.dv2 = s->dv[2],
			.dv3 = s->dv[3],
			.rweights = s->rweights,
			.attn_res = s->attn_res,
			.hidden = s->hidden,
			.residual = s->residual,
			.n = m->config.hidden_size,
		};
		fun_compute_graph_add_task(m->graph, t_post_moe, _exec_post_moe,
								   m->ctx_post_moe[l], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_post_moe, t_router);
		for (int e = 0; e < 4; e++)
			fun_compute_task_depends_on(m->graph, t_post_moe, t_e[e]);

		prev_layer_tail = t_post_moe;
	}

	FunComputeTask *t_out_norm = &m->tasks[ti++];
	m->ctx_out_norm = fun_memory_allocate(sizeof(CtxRmsNorm)).value;
	*(CtxRmsNorm *)m->ctx_out_norm = (CtxRmsNorm){
		.x = s->hidden,
		.weight = m->output_norm_weight,
		.out = s->hidden_normed,
		.n = m->config.hidden_size,
		.eps = m->config.rms_norm_eps,
	};
	fun_compute_graph_add_task(m->graph, t_out_norm, _exec_rms_norm,
							   m->ctx_out_norm, NULL, NULL);
	fun_compute_task_depends_on(m->graph, t_out_norm, prev_layer_tail);

	FunComputeTask *t_out_h[4];
	size_t vocab = m->config.vocab_size;
	size_t chunk = vocab / 4;
	for (int og = 0; og < 4; og++) {
		t_out_h[og] = &m->tasks[ti++];
		m->ctx_output_groups[og] =
			fun_memory_allocate(sizeof(CtxOutputGroup)).value;
		*(CtxOutputGroup *)m->ctx_output_groups[og] =
			(CtxOutputGroup){ .w = m->output_weight,
							  .x = s->hidden_normed,
							  .out = m->logits,
							  .row_start = (size_t)og * chunk,
							  .row_end = (og == 3) ? vocab :
													 (size_t)(og + 1) * chunk,
							  .vocab = vocab,
							  .hs = m->config.hidden_size };
		fun_compute_graph_add_task(m->graph, t_out_h[og], _exec_output_group,
								   m->ctx_output_groups[og], NULL, NULL);
		fun_compute_task_depends_on(m->graph, t_out_h[og], t_out_norm);
	}

	m->n_tasks = ti;
}

/* Forward one token at a time for tokens[0..n_tokens), skipping the first
 * cached_len positions that are already in the KV cache (incremental decode).
 * Each step submits the graph with {token_id, pos}, blocks until it finishes,
 * and copies that position's vocab-sized logits into the caller's buffer. */
void model_forward(Model *m, const int *tokens, int n_tokens, float *logits)
{
	int start = m->cached_len;
	if (start < 0 || start > n_tokens)
		start = 0;
	for (int pos = start; pos < n_tokens; pos++) {
		SubmitCtx sctx = { .token_id = tokens[pos], .pos = pos };
		fun_trace_span_begin(SPAN_INFERENCE);
		fun_compute_graph_submit(m->graph, &sctx);
		fun_compute_graph_wait(m->graph);
		fun_trace_span_end(SPAN_INFERENCE);
		m->cached_len = n_tokens;
		for (size_t i = 0; i < m->config.vocab_size; i++)
			logits[i] = m->logits[i];
	}
}

/* Tear down everything model_load allocated: graph, task contexts, scratch,
 * logits, layer weights, KV caches. Order matters (tasks destroyed before the
 * graph is freed; per-layer contexts before the arrays holding them). */
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

	if (m->ctx_embed)
		fun_memory_free((Memory *)&m->ctx_embed);
	if (m->ctx_rope)
		fun_memory_free((Memory *)&m->ctx_rope);
	if (m->ctx_embed_residual)
		fun_memory_free((Memory *)&m->ctx_embed_residual);
	if (m->ctx_out_norm)
		fun_memory_free((Memory *)&m->ctx_out_norm);
	if (m->ctx_logits)
		fun_memory_free((Memory *)&m->ctx_logits);
	for (int l = 0; l < m->config.n_layers; l++) {
		if (m->ctx_norms && m->ctx_norms[l])
			fun_memory_free((Memory *)&m->ctx_norms[l]);
		for (int qs = 0; qs < 4; qs++)
			if (m->ctx_q && m->ctx_q[l * 4 + qs])
				fun_memory_free((Memory *)&m->ctx_q[l * 4 + qs]);
		if (m->ctx_k && m->ctx_k[l])
			fun_memory_free((Memory *)&m->ctx_k[l]);
		if (m->ctx_v && m->ctx_v[l])
			fun_memory_free((Memory *)&m->ctx_v[l]);
		if (m->ctx_rq && m->ctx_rq[l])
			fun_memory_free((Memory *)&m->ctx_rq[l]);
		if (m->ctx_rk && m->ctx_rk[l])
			fun_memory_free((Memory *)&m->ctx_rk[l]);
		if (m->ctx_kv && m->ctx_kv[l])
			fun_memory_free((Memory *)&m->ctx_kv[l]);
		for (int os = 0; os < 4; os++)
			if (m->ctx_o && m->ctx_o[l * 4 + os])
				fun_memory_free((Memory *)&m->ctx_o[l * 4 + os]);
		if (m->ctx_post_attn && m->ctx_post_attn[l])
			fun_memory_free((Memory *)&m->ctx_post_attn[l]);
		if (m->ctx_router && m->ctx_router[l])
			fun_memory_free((Memory *)&m->ctx_router[l]);
		if (m->ctx_post_moe && m->ctx_post_moe[l])
			fun_memory_free((Memory *)&m->ctx_post_moe[l]);
		for (int e = 0; e < 4; e++)
			if (m->ctx_expert && m->ctx_expert[l * 4 + e])
				fun_memory_free((Memory *)&m->ctx_expert[l * 4 + e]);
		for (int hg = 0; hg < 4; hg++)
			if (m->ctx_attn_groups && m->ctx_attn_groups[l * 4 + hg])
				fun_memory_free((Memory *)&m->ctx_attn_groups[l * 4 + hg]);
	}
	fun_memory_free((Memory *)&m->ctx_norms);
	fun_memory_free((Memory *)&m->ctx_q);
	fun_memory_free((Memory *)&m->ctx_k);
	fun_memory_free((Memory *)&m->ctx_v);
	fun_memory_free((Memory *)&m->ctx_rq);
	fun_memory_free((Memory *)&m->ctx_rk);
	fun_memory_free((Memory *)&m->ctx_kv);
	fun_memory_free((Memory *)&m->ctx_o);
	fun_memory_free((Memory *)&m->ctx_post_attn);
	fun_memory_free((Memory *)&m->ctx_router);
	fun_memory_free((Memory *)&m->ctx_post_moe);
	if (m->ctx_expert)
		fun_memory_free((Memory *)&m->ctx_expert);
	if (m->ctx_attn_groups)
		fun_memory_free((Memory *)&m->ctx_attn_groups);
	for (int og = 0; og < 4; og++)
		if (m->ctx_output_groups && m->ctx_output_groups[og])
			fun_memory_free((Memory *)&m->ctx_output_groups[og]);
	if (m->ctx_output_groups)
		fun_memory_free((Memory *)&m->ctx_output_groups);

	if (m->scratch_mem) {
		scratch_free((Scratch *)m->scratch_mem);
		fun_memory_free((Memory *)&m->scratch_mem);
	}
	if (m->logits)
		fun_memory_free((Memory *)&m->logits);
	if (m->layers)
		fun_memory_free((Memory *)&m->layers);
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
