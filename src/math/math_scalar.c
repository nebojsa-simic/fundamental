#include "fundamental/math/math.h"

#include <stdint.h>

#define L2E 1.4426950408889634f
#define LN2 0.6931471805599453f

static float _nan(void)
{
    uint32_t u = 0x7FC00000u;
    return *(float *)&u;
}

static float _inf(void)
{
    uint32_t u = 0x7F800000u;
    return *(float *)&u;
}

static float _neginf(void)
{
    uint32_t u = 0xFF800000u;
    return *(float *)&u;
}

static int _is_nan(float x)
{
    uint32_t u = *(uint32_t *)&x;
    return (u & 0x7F800000u) == 0x7F800000u && (u & 0x007FFFFFu) != 0;
}

static int _is_inf(float x)
{
    uint32_t u = *(uint32_t *)&x;
    return (u & 0x7FFFFFFFu) == 0x7F800000u;
}

float fun_math_sqrt(float x)
{
    if (_is_nan(x))
        return x;
    if (x < 0.0f) {
        uint32_t u = *(uint32_t *)&x;
        if (u == 0x80000000u)
            return x;
        return _nan();
    }
    if (x == 0.0f)
        return x;
    if (_is_inf(x))
        return x;

    float half_x = 0.5f * x;
    int i = *(int *)&x;
    i = 0x5f3759df - (i >> 1);
    float y = *(float *)&i;
    y = y * (1.5f - half_x * y * y);
    y = y * (1.5f - half_x * y * y);
    return x * y;
}

float fun_math_exp(float x)
{
    if (_is_nan(x))
        return x;
    if (x >= 88.7228f)
        return _inf();
    if (x <= -87.3365f)
        return 0.0f;
    if (_is_inf(x)) {
        if (x > 0.0f)
            return _inf();
        return 0.0f;
    }

    float kf = x * L2E + 0.5f;
    int k = (int)kf;
    if (kf < k)
        k--;
    float r = x - (float)k * LN2;

    float c0 = 1.0f;
    float c1 = 0.5f;
    float c2 = 0.10714285714285714f;
    float c3 = 0.011904761904761904f;
    float c4 = 0.0005952380952380952f;

    float r2 = r * r;
    float num = c0 + c1 * r + c2 * r2 + c3 * r * r2 + c4 * r2 * r2;
    float den = c0 - c1 * r + c2 * r2 - c3 * r * r2 + c4 * r2 * r2;
    float er = num / den;

    uint32_t bits = (uint32_t)(k + 127) << 23;
    return er * (*(float *)&bits);
}

float fun_math_log(float x)
{
    if (_is_nan(x))
        return x;
    uint32_t u = *(uint32_t *)&x;
    if (u == 0x80000000u || u == 0x00000000u)
        return _neginf();
    if (x < 0.0f)
        return _nan();
    if (_is_inf(x))
        return x;

    uint32_t bits = *(uint32_t *)&x;
    int exp_field = (int)((bits >> 23) & 0xFFu);
    int n = exp_field - 127;
    uint32_t mantissa_bits = (bits & 0x7FFFFFu) | 0x3F800000u;
    float m = *(float *)&mantissa_bits;

    float t = (m - 1.0f) / (m + 1.0f);
    float t2 = t * t;
    float lg_m = 2.0f * t * (1.0f + t2 * (0.333333333f +
                       t2 * (0.2f + t2 * (0.142857143f + t2 * 0.111111111f))));

    return (float)n * LN2 + lg_m;
}

float fun_math_sin(float x)
{
    (void)x;
    return 0.0f;
}

float fun_math_cos(float x)
{
    (void)x;
    return 0.0f;
}

float fun_math_tanh(float x)
{
    (void)x;
    return 0.0f;
}

float fun_math_sigmoid(float x)
{
    (void)x;
    return 0.0f;
}

float fun_math_silu(float x)
{
    (void)x;
    return 0.0f;
}

void fun_math_silu_f32(const float *x, float *out, size_t n)
{
    (void)x;
    (void)out;
    (void)n;
}

void fun_math_rms_norm_f32(const float *x, const float *weight, float *out,
                           size_t n, float eps)
{
    (void)x;
    (void)weight;
    (void)out;
    (void)n;
    (void)eps;
}

void fun_math_swiglu_f32(const float *gate, const float *up, float *out,
                         size_t n)
{
    (void)gate;
    (void)up;
    (void)out;
    (void)n;
}

void fun_math_softmax_f32(float *x, size_t n)
{
    (void)x;
    (void)n;
}

void _math_bench_noop(const float *x, float *out, size_t n)
{
    for (size_t i = 0; i < n; i++)
        out[i] = x[i];
}
