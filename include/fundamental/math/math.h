#ifndef LIBRARY_MATH_H
#define LIBRARY_MATH_H

#include <stddef.h>

float fun_math_sqrt(float x);
float fun_math_exp(float x);
float fun_math_log(float x);
float fun_math_sin(float x);
float fun_math_cos(float x);
float fun_math_tanh(float x);
float fun_math_sigmoid(float x);
float fun_math_silu(float x);

void fun_math_silu_f32(const float *x, float *out, size_t n);
void fun_math_rms_norm_f32(const float *x, const float *weight, float *out,
                           size_t n, float eps);
void fun_math_swiglu_f32(const float *gate, const float *up, float *out,
                         size_t n);
void fun_math_softmax_f32(float *x, size_t n);

void fun_math_init(void);
int fun_math_has_sse2(void);
int fun_math_has_avx(void);
int fun_math_has_avx2(void);
int fun_math_has_avx512f(void);

#endif
