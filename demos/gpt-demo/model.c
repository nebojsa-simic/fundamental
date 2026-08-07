#include "model.h"
#include "fundamental/math/math.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include <stdint.h>

#define MAX_SEQ 256

static void dequant_layer(Model *m, int layer_idx)
{
	LayerWeights *w = &m->layers[layer_idx];
	char buf[256];
	int pre_len;

	fun_string_copy("blk.", buf, 256);
	pre_len = (int)fun_string_length(buf);
	fun_string_from_int(layer_idx, 10, buf + pre_len, 256 - pre_len);
	pre_len = (int)fun_string_length(buf);
	buf[pre_len++] = '.';

	fun_string_copy(buf, w->name_prefix, 64);

	fun_string_copy(".attn_q.weight", buf + pre_len, 256 - pre_len);
	uint32_t q_el = 64 * 64 * m->config.hidden_size;
	w->q_weight =
		(float *)fun_memory_allocate(q_el * sizeof(float)).value;
	fun_gguf_dequant_q8_0(m->gguf, buf, w->q_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_q.bias", buf + pre_len, 256 - pre_len);
	w->q_bias = (float *)fun_memory_allocate(64 * 64 * sizeof(float))
			    .value;
	fun_gguf_dequant_f32(m->gguf, buf, w->q_bias);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_k.weight", buf + pre_len, 256 - pre_len);
	uint64_t k_el = 8 * 64 * m->config.hidden_size;
	w->k_weight =
		(float *)fun_memory_allocate(k_el * sizeof(float)).value;
	fun_gguf_dequant_q8_0(m->gguf, buf, w->k_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_k.bias", buf + pre_len, 256 - pre_len);
	w->k_bias = (float *)fun_memory_allocate(8 * 64 * sizeof(float))
			    .value;
	fun_gguf_dequant_f32(m->gguf, buf, w->k_bias);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_v.weight", buf + pre_len, 256 - pre_len);
	w->v_weight =
		(float *)fun_memory_allocate(k_el * sizeof(float)).value;
	fun_gguf_dequant_q8_0(m->gguf, buf, w->v_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_v.bias", buf + pre_len, 256 - pre_len);
	w->v_bias = (float *)fun_memory_allocate(8 * 64 * sizeof(float))
			    .value;
	fun_gguf_dequant_f32(m->gguf, buf, w->v_bias);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_output.weight", buf + pre_len,
			256 - pre_len);
	uint64_t o_el = m->config.hidden_size * 64 * 64;
	w->o_weight =
		(float *)fun_memory_allocate(o_el * sizeof(float)).value;
	fun_gguf_dequant_q8_0(m->gguf, buf, w->o_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_output.bias", buf + pre_len,
			256 - pre_len);
	w->o_bias = (float *)fun_memory_allocate(
			    m->config.hidden_size * sizeof(float))
			    .value;
	fun_gguf_dequant_f32(m->gguf, buf, w->o_bias);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_norm.weight", buf + pre_len,
			256 - pre_len);
	w->attn_norm_weight = (float *)fun_memory_allocate(
				      m->config.hidden_size *
				      sizeof(float))
				      .value;
	fun_gguf_dequant_f32(m->gguf, buf, w->attn_norm_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".post_attention_norm.weight", buf + pre_len,
			256 - pre_len);
	w->post_attn_norm_weight = (float *)fun_memory_allocate(
					   m->config.hidden_size *
					   sizeof(float))
					   .value;
	fun_gguf_dequant_f32(m->gguf, buf, w->post_attn_norm_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".ffn_gate_inp.weight", buf + pre_len,
			256 - pre_len);
	w->router_weight = (float *)fun_memory_allocate(
				   m->config.n_experts *
				   m->config.hidden_size *
				   sizeof(float))
				   .value;
	fun_gguf_dequant_f32(m->gguf, buf, w->router_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".ffn_gate_inp.bias", buf + pre_len,
			256 - pre_len);
	w->router_bias = (float *)fun_memory_allocate(
				 m->config.n_experts * sizeof(float))
				 .value;
	fun_gguf_dequant_f32(m->gguf, buf, w->router_bias);
}

void model_load(Model *m, GGufFile *gguf)
{
	m->gguf = gguf;
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

	m->tok_embeddings =
		(float *)fun_memory_allocate(m->config.vocab_size *
					     m->config.hidden_size *
					     sizeof(float))
			 .value;
	fun_gguf_dequant_q8_0(gguf, "token_embd.weight",
			      m->tok_embeddings);

	m->output_weight =
		(float *)fun_memory_allocate(m->config.vocab_size *
					     m->config.hidden_size *
					     sizeof(float))
			 .value;
	fun_gguf_dequant_q8_0(gguf, "output.weight", m->output_weight);

	m->layers = (LayerWeights *)fun_memory_allocate(
			    m->config.n_layers * sizeof(LayerWeights))
			    .value;
}

void model_free(Model *m)
{
	if (m->tok_embeddings)
		fun_memory_free((Memory *)&m->tok_embeddings);
	if (m->output_weight)
		fun_memory_free((Memory *)&m->output_weight);
	if (m->layers) {
		fun_memory_free((Memory *)&m->layers);
	}
}

static void rms_norm(float *x, const float *w, int n, float eps)
{
	float ss = 0.0f;
	for (int i = 0; i < n; i++)
		ss += x[i] * x[i];
	float scale = 1.0f / fun_math_sqrt(ss / (float)n + eps);
	for (int i = 0; i < n; i++)
		x[i] *= scale * w[i];
}

static void mat_vec_f32(const float *w, const float *x, float *bias,
			 float *out, int rows, int cols)
{
	for (int r = 0; r < rows; r++) {
		float s = bias ? bias[r] : 0.0f;
		for (int c = 0; c < cols; c++)
			s += w[r * cols + c] * x[c];
		out[r] = s;
	}
}

static float mat_vec_dot32(const float *a, const float *b, int n)
{
	float s = 0.0f;
	for (int i = 0; i < n; i++)
		s += a[i] * b[i];
	return s;
}

static void rope_single(float *q, float *k, int pos, float theta, int hd,
			 int n_h, int n_kv_h)
{
	for (int h = 0; h < n_h; h++) {
		float *qh = q + h * hd;
		for (int d = 0; d < hd; d += 2) {
			float freq =
				1.0f / fun_math_sqrt(6.0f * theta + 1e-6f);
			float angle = (float)pos * freq;
			float ca = fun_math_cos(angle);
			float sa = fun_math_sin(angle);
			float q0 = qh[d], q1 = qh[d + 1];
			qh[d] = q0 * ca - q1 * sa;
			qh[d + 1] = q0 * sa + q1 * ca;
		}
	}
	for (int h = 0; h < n_kv_h; h++) {
		float *kh = k + h * hd;
		for (int d = 0; d < hd; d += 2) {
			float freq =
				1.0f / fun_math_sqrt(6.0f * theta + 1e-6f);
			float angle = (float)pos * freq;
			float ca = fun_math_cos(angle);
			float sa = fun_math_sin(angle);
			float k0 = kh[d], k1 = kh[d + 1];
			kh[d] = k0 * ca - k1 * sa;
			kh[d + 1] = k0 * sa + k1 * ca;
		}
	}
}

void model_forward(Model *m, const int *tokens, int n_tokens,
		   float *logits)
{
	int hs = m->config.hidden_size;
	int n_h = m->config.n_heads;
	int n_kv = m->config.n_kv_heads;
	int hd = m->config.head_dim;
	int n_exp = m->config.n_experts;
	int topk = m->config.n_active_experts;
	int ffn = m->config.ffn_size;
	float eps = m->config.rms_norm_eps;
	float th = m->config.rope_theta;

	int kv_dim = n_kv * hd;
	int q_dim = n_h * hd;
	int max_seq = n_tokens;

	float *hidden =
		(float *)fun_memory_allocate(hs * sizeof(float)).value;
	float *qbuf =
		(float *)fun_memory_allocate(q_dim * sizeof(float)).value;
	float *kbuf =
		(float *)fun_memory_allocate(kv_dim * sizeof(float)).value;
	float *vbuf =
		(float *)fun_memory_allocate(kv_dim * sizeof(float)).value;
	float *attn =
		(float *)fun_memory_allocate(q_dim * sizeof(float)).value;
	float *proj =
		(float *)fun_memory_allocate(hs * sizeof(float)).value;
	float *expert =
		(float *)fun_memory_allocate(hs * sizeof(float)).value;

	float **k_hist = (float **)fun_memory_allocate(
				 m->config.n_layers * sizeof(float *))
				 .value;
	float **v_hist = (float **)fun_memory_allocate(
				 m->config.n_layers * sizeof(float *))
				 .value;
	for (int l = 0; l < m->config.n_layers; l++) {
		k_hist[l] = (float *)fun_memory_allocate(
				    max_seq * kv_dim * sizeof(float))
				    .value;
		v_hist[l] = (float *)fun_memory_allocate(
				    max_seq * kv_dim * sizeof(float))
				    .value;
	}

	for (int pos = 0; pos < n_tokens; pos++) {
		float *emb = m->tok_embeddings + tokens[pos] * hs;
		for (int i = 0; i < hs; i++)
			hidden[i] = emb[i];

		for (int l = 0; l < m->config.n_layers; l++) {
			LayerWeights *w = &m->layers[l];
			if (!w->q_weight)
				dequant_layer(m, l);

			rms_norm(hidden, w->attn_norm_weight, hs, eps);

			mat_vec_f32(w->q_weight, hidden, w->q_bias, qbuf,
				    q_dim, hs);
			mat_vec_f32(w->k_weight, hidden, w->k_bias, kbuf,
				    kv_dim, hs);
			mat_vec_f32(w->v_weight, hidden, w->v_bias, vbuf,
				    kv_dim, hs);

			rope_single(qbuf, kbuf, pos, th, hd, n_h, n_kv);

			for (int i = 0; i < kv_dim; i++) {
				k_hist[l][pos * kv_dim + i] = kbuf[i];
				v_hist[l][pos * kv_dim + i] = vbuf[i];
			}

			for (int i = 0; i < q_dim; i++)
				attn[i] = 0.0f;
			for (int h = 0; h < n_h; h++) {
				int kvh = h * n_kv / n_h;
				float *qh = qbuf + h * hd;
				float max_score = -1e30f;
				float sum_exp = 0.0f;
				for (int t = 0; t <= pos; t++) {
					float *kh = k_hist[l] +
						    t * kv_dim + kvh * hd;
					float s =
						mat_vec_dot32(qh, kh, hd) /
						fun_math_sqrt((float)hd);
					if (s > max_score)
						max_score = s;
					float es = fun_math_exp(
						s - max_score);
					sum_exp += es;
				}
				for (int t = 0; t <= pos; t++) {
					float *kh = k_hist[l] +
						    t * kv_dim + kvh * hd;
					float s =
						mat_vec_dot32(qh, kh, hd) /
						fun_math_sqrt((float)hd);
					float wgt = fun_math_exp(
							    s - max_score) /
						    sum_exp;
					float *vh = v_hist[l] +
						    t * kv_dim + kvh * hd;
					for (int d = 0; d < hd; d++)
						attn[h * hd + d] +=
							wgt * vh[d];
				}
			}

			mat_vec_f32(w->o_weight, attn, w->o_bias, proj, hs,
				    q_dim);
			for (int i = 0; i < hs; i++)
				hidden[i] += proj[i];

			rms_norm(hidden, w->post_attn_norm_weight, hs,
				 eps);

			float rlog[32];
			for (int e = 0; e < n_exp; e++) {
				float s = 0.0f;
				for (int i = 0; i < hs; i++)
					s += w->router_weight[e * hs + i] *
					     hidden[i];
				rlog[e] = s + w->router_bias[e];
			}

			int top[4] = { 0, 0, 0, 0 };
			float topv[4] = { -1e30f, -1e30f, -1e30f, -1e30f };
			for (int e = 0; e < n_exp; e++) {
				float v = rlog[e];
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

			float rw[4], rsum = 0.0f;
			for (int i = 0; i < topk; i++) {
				rw[i] = fun_math_exp(topv[i]);
				rsum += rw[i];
			}
			for (int i = 0; i < topk; i++)
				rw[i] /= rsum;

			for (int i = 0; i < hs; i++)
				expert[i] = 0.0f;

			for (int ex = 0; ex < topk; ex++) {
				int e = top[ex];
				uint64_t el_start = (uint64_t)e * ffn * hs;
				uint64_t el_count = (uint64_t)ffn * hs;
				char eb[256];
				int pl =
					(int)fun_string_length(
						w->name_prefix);

				for (int c = 0; c < pl; c++)
					eb[c] = w->name_prefix[c];
				fun_string_copy(".ffn_gate_exps.weight",
						eb + pl, 256 - pl);

				float *gate_buf = (float *)
					fun_memory_allocate(el_count *
							    sizeof(float))
						.value;
				fun_gguf_dequant_mxfp4_range(
					m->gguf, eb, el_start, el_count,
					gate_buf);

				eb[pl] = '\0';
				fun_string_copy(".ffn_gate_exps.bias",
						eb + pl, 256 - pl);
				float *gbias = (float *)
					fun_memory_allocate(
						ffn * sizeof(float))
						.value;
				fun_gguf_dequant_f32(m->gguf, eb,
						     gbias);

				eb[pl] = '\0';
				fun_string_copy(".ffn_up_exps.weight",
						eb + pl, 256 - pl);
				float *up_buf = (float *)
					fun_memory_allocate(el_count *
							    sizeof(float))
						.value;
				fun_gguf_dequant_mxfp4_range(
					m->gguf, eb, el_start, el_count,
					up_buf);

				eb[pl] = '\0';
				fun_string_copy(".ffn_up_exps.bias",
						eb + pl, 256 - pl);
				float *ubias = (float *)
					fun_memory_allocate(
						ffn * sizeof(float))
						.value;
				fun_gguf_dequant_f32(m->gguf, eb,
						     ubias);

				eb[pl] = '\0';
				fun_string_copy(
					".ffn_down_exps.weight",
					eb + pl, 256 - pl);
				float *down_buf = (float *)
					fun_memory_allocate(
						(uint64_t)hs * ffn *
						sizeof(float))
						.value;
				fun_gguf_dequant_mxfp4_range(
					m->gguf, eb, el_start, el_count,
					down_buf);

				eb[pl] = '\0';
				fun_string_copy(
					".ffn_down_exps.bias",
					eb + pl, 256 - pl);
				float *dbias = (float *)
					fun_memory_allocate(
						hs * sizeof(float))
						.value;
				fun_gguf_dequant_f32(m->gguf, eb,
						     dbias);

				float *mid = (float *)
					fun_memory_allocate(
						ffn * sizeof(float))
						.value;
				for (int i = 0; i < ffn; i++) {
					float s = 0.0f;
					for (int j = 0; j < hs; j++)
						s += gate_buf[i * hs + j] *
						     hidden[j];
					mid[i] = fun_math_silu(
						s + gbias[i]);
				}
				for (int i = 0; i < ffn; i++) {
					float s = 0.0f;
					for (int j = 0; j < hs; j++)
						s += up_buf[i * hs + j] *
						     hidden[j];
					mid[i] *= s + ubias[i];
				}

				for (int i = 0; i < hs; i++) {
					float s = 0.0f;
					for (int j = 0; j < ffn; j++)
						s += down_buf[i * ffn + j] *
						     mid[j];
					expert[i] +=
						rw[ex] *
						(s + dbias[i]);
				}

				fun_memory_free((Memory *)&gate_buf);
				fun_memory_free((Memory *)&gbias);
				fun_memory_free((Memory *)&up_buf);
				fun_memory_free((Memory *)&ubias);
				fun_memory_free((Memory *)&down_buf);
				fun_memory_free((Memory *)&dbias);
				fun_memory_free((Memory *)&mid);
			}

			for (int i = 0; i < hs; i++)
				hidden[i] += expert[i];
		}
	}

	for (int v = 0; v < m->config.vocab_size; v++) {
		float s = 0.0f;
		for (int i = 0; i < hs; i++)
			s += m->output_weight[v * hs + i] * hidden[i];
		logits[v] = s;
	}

	fun_memory_free((Memory *)&hidden);
	fun_memory_free((Memory *)&qbuf);
	fun_memory_free((Memory *)&kbuf);
	fun_memory_free((Memory *)&vbuf);
	fun_memory_free((Memory *)&attn);
	fun_memory_free((Memory *)&proj);
	fun_memory_free((Memory *)&expert);
	for (int l = 0; l < m->config.n_layers; l++) {
		fun_memory_free((Memory *)&k_hist[l]);
		fun_memory_free((Memory *)&v_hist[l]);
	}
	fun_memory_free((Memory *)&k_hist);
	fun_memory_free((Memory *)&v_hist);
}
