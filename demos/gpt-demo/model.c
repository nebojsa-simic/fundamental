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

	fun_string_copy(buf, w->name_prefix, 64);

	fun_string_copy(".attn_q.weight", buf + pre_len, 256 - pre_len);
	uint32_t q_el = 64 * 64 * m->config.hidden_size;
	w->q_weight = (float *)fun_memory_allocate(q_el * sizeof(float)).value;
	fun_gguf_dequant_q8_0(m->gguf, buf, w->q_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_q.bias", buf + pre_len, 256 - pre_len);
	w->q_bias = (float *)fun_memory_allocate(64 * 64 * sizeof(float)).value;
	fun_gguf_dequant_f32(m->gguf, buf, w->q_bias);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_k.weight", buf + pre_len, 256 - pre_len);
	uint64_t k_el = 8 * 64 * m->config.hidden_size;
	w->k_weight = (float *)fun_memory_allocate(k_el * sizeof(float)).value;
	fun_gguf_dequant_q8_0(m->gguf, buf, w->k_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_k.bias", buf + pre_len, 256 - pre_len);
	w->k_bias = (float *)fun_memory_allocate(8 * 64 * sizeof(float)).value;
	fun_gguf_dequant_f32(m->gguf, buf, w->k_bias);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_v.weight", buf + pre_len, 256 - pre_len);
	w->v_weight = (float *)fun_memory_allocate(k_el * sizeof(float)).value;
	fun_gguf_dequant_q8_0(m->gguf, buf, w->v_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_v.bias", buf + pre_len, 256 - pre_len);
	w->v_bias = (float *)fun_memory_allocate(8 * 64 * sizeof(float)).value;
	fun_gguf_dequant_f32(m->gguf, buf, w->v_bias);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_output.weight", buf + pre_len, 256 - pre_len);
	uint64_t o_el = m->config.hidden_size * 64 * 64;
	w->o_weight = (float *)fun_memory_allocate(o_el * sizeof(float)).value;
	fun_gguf_dequant_q8_0(m->gguf, buf, w->o_weight);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_output.bias", buf + pre_len, 256 - pre_len);
	w->o_bias =
		(float *)fun_memory_allocate(m->config.hidden_size * sizeof(float))
			.value;
	fun_gguf_dequant_f32(m->gguf, buf, w->o_bias);

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
		(float *)fun_memory_allocate(m->config.n_experts * sizeof(float)).value;
	fun_gguf_dequant_f32(m->gguf, buf, w->router_bias);

	buf[pre_len] = '\0';
	fun_string_copy(".attn_sinks.weight", buf + pre_len, 256 - pre_len);
	w->sinks =
		(float *)fun_memory_allocate(m->config.n_heads * sizeof(float)).value;
	fun_gguf_dequant_f32(m->gguf, buf, w->sinks);
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
									 m->config.hidden_size * sizeof(float))
			.value;
	fun_gguf_dequant_q8_0(gguf, "token_embd.weight", m->tok_embeddings);

	m->output_weight =
		(float *)fun_memory_allocate(m->config.vocab_size *
									 m->config.hidden_size * sizeof(float))
			.value;
	fun_gguf_dequant_q8_0(gguf, "output.weight", m->output_weight);

	m->output_norm_weight =
		(float *)fun_memory_allocate(m->config.hidden_size * sizeof(float))
			.value;
	fun_gguf_dequant_f32(gguf, "output_norm.weight", m->output_norm_weight);

	m->layers = (LayerWeights *)fun_memory_allocate(m->config.n_layers *
													sizeof(LayerWeights))
					.value;
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
}

void model_free(Model *m)
{
	if (m->tok_embeddings)
		fun_memory_free((Memory *)&m->tok_embeddings);
	if (m->output_weight)
		fun_memory_free((Memory *)&m->output_weight);
	if (m->output_norm_weight)
		fun_memory_free((Memory *)&m->output_norm_weight);
	if (m->layers) {
		fun_memory_free((Memory *)&m->layers);
	}
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

static void rms_norm(float *x, const float *w, int n, float eps)
{
	fun_math_rms_norm_f32(x, w, x, (size_t)n, eps);
}

static void rope_single(float *q, float *k, int pos, float theta, int hd,
						int n_h, int n_kv_h)
{
	float base = theta;
	float theta_scale = fun_math_exp(fun_math_log(base) * (-2.0f / (float)hd));
	float mscale = 1.0f + 0.1f * fun_math_log(32.0f);
	int corr0 = 8, corr1 = 18;
	int half = hd / 2;
	float the_arr[32];
	float ca[32];
	float sa[32];

	for (int h = 0; h < n_h; h++) {
		float *qh = q + h * hd;
		float the = (float)pos;
		for (int j = 0; j < half; j++) {
			float y = (float)(j - corr0) / (float)(corr1 - corr0);
			if (y < 0.0f)
				y = 0.0f;
			if (y > 1.0f)
				y = 1.0f;
			float ramp = 1.0f - y;
			the_arr[j] = (the / 32.0f) * (1.0f - ramp) + the * ramp;
			the *= theta_scale;
		}
		fun_math_cos_f32(the_arr, ca, (size_t)half);
		fun_math_sin_f32(the_arr, sa, (size_t)half);
		for (int j = 0; j < half; j++) {
			ca[j] *= mscale;
			sa[j] *= mscale;
			float q0 = qh[j], q1 = qh[j + half];
			qh[j] = q0 * ca[j] - q1 * sa[j];
			qh[j + half] = q0 * sa[j] + q1 * ca[j];
		}
	}
	for (int h = 0; h < n_kv_h; h++) {
		float *kh = k + h * hd;
		float the = (float)pos;
		for (int j = 0; j < half; j++) {
			float y = (float)(j - corr0) / (float)(corr1 - corr0);
			if (y < 0.0f)
				y = 0.0f;
			if (y > 1.0f)
				y = 1.0f;
			float ramp = 1.0f - y;
			the_arr[j] = (the / 32.0f) * (1.0f - ramp) + the * ramp;
			the *= theta_scale;
		}
		fun_math_cos_f32(the_arr, ca, (size_t)half);
		fun_math_sin_f32(the_arr, sa, (size_t)half);
		for (int j = 0; j < half; j++) {
			ca[j] *= mscale;
			sa[j] *= mscale;
			float k0 = kh[j], k1 = kh[j + half];
			kh[j] = k0 * ca[j] - k1 * sa[j];
			kh[j + half] = k0 * sa[j] + k1 * ca[j];
		}
	}
}

void model_forward(Model *m, const int *tokens, int n_tokens, float *logits)
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

	int start = m->cached_len;
	if (start < 0 || start > n_tokens)
		start = 0;

	float *hidden = (float *)fun_memory_allocate(hs * sizeof(float)).value;
	float *residual = (float *)fun_memory_allocate(hs * sizeof(float)).value;
	float *attn_res = (float *)fun_memory_allocate(hs * sizeof(float)).value;
	float *qbuf = (float *)fun_memory_allocate(q_dim * sizeof(float)).value;
	float *kbuf = (float *)fun_memory_allocate(kv_dim * sizeof(float)).value;
	float *vbuf = (float *)fun_memory_allocate(kv_dim * sizeof(float)).value;
	float *attn = (float *)fun_memory_allocate(q_dim * sizeof(float)).value;
	float *proj = (float *)fun_memory_allocate(hs * sizeof(float)).value;
	float *expert = (float *)fun_memory_allocate(hs * sizeof(float)).value;

	for (int pos = start; pos < n_tokens; pos++) {
		float *emb = m->tok_embeddings + tokens[pos] * hs;
		for (int i = 0; i < hs; i++)
			hidden[i] = emb[i];

		for (int l = 0; l < m->config.n_layers; l++) {
			LayerWeights *w = &m->layers[l];
			if (!w->q_weight)
				dequant_layer(m, l);

			for (int i = 0; i < hs; i++)
				residual[i] = hidden[i];

			rms_norm(hidden, w->attn_norm_weight, hs, eps);

			fun_math_matrix_vector_f32(w->q_weight, hidden, w->q_bias, qbuf,
									   (size_t)q_dim, (size_t)hs);
			fun_math_matrix_vector_f32(w->k_weight, hidden, w->k_bias, kbuf,
									   (size_t)kv_dim, (size_t)hs);
			fun_math_matrix_vector_f32(w->v_weight, hidden, w->v_bias, vbuf,
									   (size_t)kv_dim, (size_t)hs);
			rope_single(qbuf, kbuf, pos, th, hd, n_h, n_kv);

			for (int i = 0; i < kv_dim; i++) {
				m->k_cache[l][pos * kv_dim + i] = kbuf[i];
				m->v_cache[l][pos * kv_dim + i] = vbuf[i];
			}

			for (int i = 0; i < q_dim; i++)
				attn[i] = 0.0f;
			int swa = (l % 2 == 0);
			int swa_len = 128;
			float *scores =
				(float *)fun_memory_allocate((size_t)(pos + 1) * sizeof(float))
					.value;
			for (int h = 0; h < n_h; h++) {
				int kvh = h * n_kv / n_h;
				float *qh = qbuf + h * hd;
				float inv_sqrt_hd = 1.0f / fun_math_sqrt((float)hd);
				for (int t = 0; t <= pos; t++) {
					if (swa && t < pos - swa_len + 1) {
						scores[t] = -1.0f / 0.0f;
						continue;
					}
					float *kh = m->k_cache[l] + t * kv_dim + kvh * hd;
					scores[t] =
						fun_math_dot_f32(qh, kh, (size_t)hd) * inv_sqrt_hd;
				}
				float sink = w->sinks[h];
				float mx = sink;
				for (int t = 0; t <= pos; t++)
					if (scores[t] > mx)
						mx = scores[t];
				float denom = 0.0f;
				for (int t = 0; t <= pos; t++)
					scores[t] -= mx;
				fun_math_exp_f32(scores, scores, (size_t)pos + 1);
				for (int t = 0; t <= pos; t++)
					denom += scores[t];
				denom += fun_math_exp(sink - mx);
				for (int t = 0; t <= pos; t++)
					scores[t] /= denom;
				for (int t = 0; t <= pos; t++) {
					float *vh = m->v_cache[l] + t * kv_dim + kvh * hd;
					float wgt = scores[t];
					for (int d = 0; d < hd; d++)
						attn[h * hd + d] += wgt * vh[d];
				}
			}
			fun_memory_free((Memory *)&scores);

			fun_math_matrix_vector_f32(w->o_weight, attn, w->o_bias, proj,
									   (size_t)hs, (size_t)q_dim);
			for (int i = 0; i < hs; i++)
				hidden[i] = residual[i] + proj[i];

			for (int i = 0; i < hs; i++)
				attn_res[i] = hidden[i];

			rms_norm(hidden, w->post_attn_norm_weight, hs, eps);

			float rlog[32];
			for (int e = 0; e < n_exp; e++) {
				float s = 0.0f;
				for (int i = 0; i < hs; i++)
					s += w->router_weight[e * hs + i] * hidden[i];
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

			float rmax = topv[0];
			float rw[4], rsum = 0.0f;
			for (int i = 0; i < topk; i++) {
				rw[i] = fun_math_exp(topv[i] - rmax);
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
				int pl = (int)fun_string_length(w->name_prefix);

				for (int c = 0; c < pl; c++)
					eb[c] = w->name_prefix[c];
				fun_string_copy(".ffn_gate_exps.weight", eb + pl, 256 - pl);

				float *gate_buf =
					(float *)fun_memory_allocate(el_count * sizeof(float)).value;
				fun_gguf_dequant_mxfp4_range(m->gguf, eb, el_start, el_count,
											 gate_buf);

				eb[pl] = '\0';
				fun_string_copy(".ffn_gate_exps.bias", eb + pl, 256 - pl);
				float *gbias =
					(float *)fun_memory_allocate(ffn * sizeof(float)).value;
				fun_gguf_dequant_f32_range(m->gguf, eb, (uint64_t)e * ffn, ffn,
										   gbias);

				eb[pl] = '\0';
				fun_string_copy(".ffn_up_exps.weight", eb + pl, 256 - pl);
				float *up_buf =
					(float *)fun_memory_allocate(el_count * sizeof(float)).value;
				fun_gguf_dequant_mxfp4_range(m->gguf, eb, el_start, el_count,
											 up_buf);

				eb[pl] = '\0';
				fun_string_copy(".ffn_up_exps.bias", eb + pl, 256 - pl);
				float *ubias =
					(float *)fun_memory_allocate(ffn * sizeof(float)).value;
				fun_gguf_dequant_f32_range(m->gguf, eb, (uint64_t)e * ffn, ffn,
										   ubias);

				eb[pl] = '\0';
				fun_string_copy(".ffn_down_exps.weight", eb + pl, 256 - pl);
				float *down_buf = (float *)fun_memory_allocate(
									  (uint64_t)hs * ffn * sizeof(float))
									  .value;
				fun_gguf_dequant_mxfp4_range(m->gguf, eb, el_start, el_count,
											 down_buf);

				eb[pl] = '\0';
				fun_string_copy(".ffn_down_exps.bias", eb + pl, 256 - pl);
				float *dbias =
					(float *)fun_memory_allocate(hs * sizeof(float)).value;
				fun_gguf_dequant_f32_range(m->gguf, eb, (uint64_t)e * hs, hs,
										   dbias);

				float *mid =
					(float *)fun_memory_allocate(ffn * sizeof(float)).value;
				float *gv =
					(float *)fun_memory_allocate(ffn * sizeof(float)).value;
				float *uv =
					(float *)fun_memory_allocate(ffn * sizeof(float)).value;
				float *eg =
					(float *)fun_memory_allocate(ffn * sizeof(float)).value;
				fun_math_matrix_vector_f32(gate_buf, hidden, gbias, gv,
										   (size_t)ffn, (size_t)hs);
				fun_math_matrix_vector_f32(up_buf, hidden, ubias, uv,
										   (size_t)ffn, (size_t)hs);
				for (int i = 0; i < ffn; i++) {
					float g = gv[i];
					if (g > 7.0f)
						g = 7.0f;
					eg[i] = -1.702f * g;
				}
				fun_math_exp_f32(eg, eg, (size_t)ffn);
				for (int i = 0; i < ffn; i++) {
					float g = gv[i];
					if (g > 7.0f)
						g = 7.0f;
					float u = uv[i];
					if (u > 7.0f)
						u = 7.0f;
					if (u < -7.0f)
						u = -7.0f;
					mid[i] = g / (1.0f + eg[i]) * (u + 1.0f);
				}

				float *dv =
					(float *)fun_memory_allocate(hs * sizeof(float)).value;
				fun_math_matrix_vector_f32(down_buf, mid, dbias, dv, (size_t)hs,
										   (size_t)ffn);
				for (int i = 0; i < hs; i++)
					expert[i] += rw[ex] * dv[i];

				fun_memory_free((Memory *)&gate_buf);
				fun_memory_free((Memory *)&gbias);
				fun_memory_free((Memory *)&up_buf);
				fun_memory_free((Memory *)&ubias);
				fun_memory_free((Memory *)&down_buf);
				fun_memory_free((Memory *)&dbias);
				fun_memory_free((Memory *)&mid);
				fun_memory_free((Memory *)&gv);
				fun_memory_free((Memory *)&uv);
				fun_memory_free((Memory *)&eg);
				fun_memory_free((Memory *)&dv);
			}

			for (int i = 0; i < hs; i++)
				hidden[i] = attn_res[i] + expert[i];
		}
	}

	m->cached_len = n_tokens;

	rms_norm(hidden, m->output_norm_weight, hs, eps);

	for (int v = 0; v < m->config.vocab_size; v++) {
		float s = 0.0f;
		for (int i = 0; i < hs; i++)
			s += m->output_weight[v * hs + i] * hidden[i];
		logits[v] = s;
	}

	fun_memory_free((Memory *)&hidden);
	fun_memory_free((Memory *)&residual);
	fun_memory_free((Memory *)&attn_res);
	fun_memory_free((Memory *)&qbuf);
	fun_memory_free((Memory *)&kbuf);
	fun_memory_free((Memory *)&vbuf);
	fun_memory_free((Memory *)&attn);
	fun_memory_free((Memory *)&proj);
	fun_memory_free((Memory *)&expert);
}
