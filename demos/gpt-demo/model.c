#include "model.h"
#include "fundamental/math/math.h"
#include "fundamental/memory/memory.h"
#include "fundamental/string/string.h"
#include <immintrin.h>
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

	/* Expert tensor raw pointers into the gguf mmap */
	const uint8_t *rd = fun_gguf_get_raw_data(m->gguf);
	buf[pre_len] = '\0';
	fun_string_copy(".ffn_gate_exps.weight", buf + pre_len, 256 - pre_len);
	w->experts.gate_w = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;

	buf[pre_len] = '\0';
	fun_string_copy(".ffn_gate_exps.bias", buf + pre_len, 256 - pre_len);
	w->experts.gate_b =
		(const float *)(rd + fun_gguf_get_tensor_offset(m->gguf, buf).value);

	buf[pre_len] = '\0';
	fun_string_copy(".ffn_up_exps.weight", buf + pre_len, 256 - pre_len);
	w->experts.up_w = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;

	buf[pre_len] = '\0';
	fun_string_copy(".ffn_up_exps.bias", buf + pre_len, 256 - pre_len);
	w->experts.up_b =
		(const float *)(rd + fun_gguf_get_tensor_offset(m->gguf, buf).value);

	buf[pre_len] = '\0';
	fun_string_copy(".ffn_down_exps.weight", buf + pre_len, 256 - pre_len);
	w->experts.down_w = rd + fun_gguf_get_tensor_offset(m->gguf, buf).value;

	buf[pre_len] = '\0';
	fun_string_copy(".ffn_down_exps.bias", buf + pre_len, 256 - pre_len);
	w->experts.down_b =
		(const float *)(rd + fun_gguf_get_tensor_offset(m->gguf, buf).value);
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

	int half = m->config.head_dim / 2;
	float theta_scale = fun_math_exp(fun_math_log(m->config.rope_theta) *
									 (-2.0f / (float)m->config.head_dim));
	int corr0 = 8, corr1 = 18;
	m->rope_pre =
		(float *)fun_memory_allocate((size_t)half * sizeof(float)).value;
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
}

void model_free(Model *m)
{
	if (m->output_norm_weight)
		fun_memory_free((Memory *)&m->output_norm_weight);
	if (m->rope_pre)
		fun_memory_free((Memory *)&m->rope_pre);
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

static void rms_norm(float *x, const float *w, int n, float eps)
{
	fun_math_rms_norm_f32(x, w, x, (size_t)n, eps);
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
	size_t exp_w_bytes = (size_t)ffn * (size_t)hs * FUN_MATH_MXFP4_BLOCK_BYTES /
						 FUN_MATH_MXFP4_BLOCK_ELEMS;

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

	int half = hd / 2;
	float *the_arr =
		(float *)fun_memory_allocate((size_t)half * sizeof(float)).value;
	float *cos_arr =
		(float *)fun_memory_allocate((size_t)half * sizeof(float)).value;
	float *sin_arr =
		(float *)fun_memory_allocate((size_t)half * sizeof(float)).value;
	float *mid_buf = (float *)fun_memory_allocate(ffn * sizeof(float)).value;
	float *gv_buf = (float *)fun_memory_allocate(ffn * sizeof(float)).value;
	float *uv_buf = (float *)fun_memory_allocate(ffn * sizeof(float)).value;
	float *eg_buf = (float *)fun_memory_allocate(ffn * sizeof(float)).value;
	float *dv_buf = (float *)fun_memory_allocate(hs * sizeof(float)).value;

	float inv_sqrt_hd = 1.0f / fun_math_sqrt((float)hd);
	float mscale = m->rope_mscale;

	for (int pos = start; pos < n_tokens; pos++) {
		fun_math_q8_dequant_row_f32(
			m->tok_embeddings +
				(uint64_t)tokens[pos] *
					((size_t)hs / FUN_MATH_Q8_BLOCK_ELEMS *
					 FUN_MATH_Q8_BLOCK_BYTES),
			hidden, (size_t)hs);

		for (int j = 0; j < half; j++)
			the_arr[j] = (float)pos * m->rope_pre[j];
		fun_math_cos_f32(the_arr, cos_arr, (size_t)half);
		fun_math_sin_f32(the_arr, sin_arr, (size_t)half);
		for (int j = 0; j < half; j++) {
			cos_arr[j] *= mscale;
			sin_arr[j] *= mscale;
		}

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
			fun_math_rotary_f32(qbuf, cos_arr, sin_arr, qbuf, (size_t)n_h,
								(size_t)half);
			fun_math_rotary_f32(kbuf, cos_arr, sin_arr, kbuf, (size_t)n_kv,
								(size_t)half);

			for (int i = 0; i < kv_dim; i++) {
				m->k_cache[l][pos * kv_dim + i] = kbuf[i];
				m->v_cache[l][pos * kv_dim + i] = vbuf[i];
			}

			int swa = (l % 2 == 0);
			int swa_len = 128;
			float *scores =
				(float *)fun_memory_allocate((size_t)(pos + 2) * sizeof(float))
					.value;
			for (int h = 0; h < n_h; h++) {
				int kvh = h * n_kv / n_h;
				float *qh = qbuf + h * hd;
				fun_math_rows_dot_f32(qh, m->k_cache[l] + kvh * hd, scores,
									  (size_t)pos + 1, (size_t)hd,
									  (size_t)kv_dim, inv_sqrt_hd);
				if (swa && pos >= swa_len) {
					for (int t = 0; t <= pos - swa_len; t++)
						scores[t] = -1.0f / 0.0f;
				}
				scores[pos + 1] = w->sinks[h];
				fun_math_softmax_f32(scores, (size_t)pos + 2);
				fun_math_weighted_sum_f32(scores, m->v_cache[l] + kvh * hd,
										  attn + h * hd, (size_t)pos + 1,
										  (size_t)hd, (size_t)kv_dim);
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
				for (int i2 = 0; i2 < hs; i2++)
					s += w->router_weight[e * hs + i2] * hidden[i2];
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

			ExpertTensors *et = &w->experts;
			for (int ex = 0; ex < topk; ex++) {
				int e = top[ex];
				const uint8_t *gw = et->gate_w + (size_t)e * exp_w_bytes;
				const float *gb = et->gate_b + (size_t)e * ffn;
				const uint8_t *uw = et->up_w + (size_t)e * exp_w_bytes;
				const float *ub = et->up_b + (size_t)e * ffn;
				const uint8_t *dw = et->down_w + (size_t)e * exp_w_bytes;
				const float *db = et->down_b + (size_t)e * hs;

				fun_math_matrix_vector_mxfp4_f32(gw, hidden, gb, gv_buf,
												 (size_t)ffn, (size_t)hs);
				fun_math_matrix_vector_mxfp4_f32(uw, hidden, ub, uv_buf,
												 (size_t)ffn, (size_t)hs);

				for (int i = 0; i < ffn; i++) {
					float g = gv_buf[i];
					if (g > 7.0f)
						g = 7.0f;
					eg_buf[i] = -1.702f * g;
				}
				fun_math_exp_f32(eg_buf, eg_buf, (size_t)ffn);
				for (int i = 0; i < ffn; i++) {
					float g = gv_buf[i];
					if (g > 7.0f)
						g = 7.0f;
					float u = uv_buf[i];
					if (u > 7.0f)
						u = 7.0f;
					if (u < -7.0f)
						u = -7.0f;
					mid_buf[i] = g / (1.0f + eg_buf[i]) * (u + 1.0f);
				}

				fun_math_matrix_vector_mxfp4_f32(dw, mid_buf, db, dv_buf,
												 (size_t)hs, (size_t)ffn);
				for (int i = 0; i < hs; i++)
					expert[i] += rw[ex] * dv_buf[i];
			}

			for (int i = 0; i < hs; i++)
				hidden[i] = attn_res[i] + expert[i];
		}
	}

	m->cached_len = n_tokens;

	rms_norm(hidden, m->output_norm_weight, hs, eps);

	fun_math_q8_matrix_vector_f32(m->output_weight, hidden, logits,
								  (size_t)m->config.vocab_size, (size_t)hs);

	fun_memory_free((Memory *)&hidden);
	fun_memory_free((Memory *)&residual);
	fun_memory_free((Memory *)&attn_res);
	fun_memory_free((Memory *)&qbuf);
	fun_memory_free((Memory *)&kbuf);
	fun_memory_free((Memory *)&vbuf);
	fun_memory_free((Memory *)&attn);
	fun_memory_free((Memory *)&proj);
	fun_memory_free((Memory *)&expert);
	fun_memory_free((Memory *)&the_arr);
	fun_memory_free((Memory *)&cos_arr);
	fun_memory_free((Memory *)&sin_arr);
	fun_memory_free((Memory *)&mid_buf);
	fun_memory_free((Memory *)&gv_buf);
	fun_memory_free((Memory *)&uv_buf);
	fun_memory_free((Memory *)&eg_buf);
	fun_memory_free((Memory *)&dv_buf);
}
