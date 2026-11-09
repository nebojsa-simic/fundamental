#ifndef LIBRARY_MATH_KERNELS_H
#define LIBRARY_MATH_KERNELS_H

#include <stddef.h>

#include "fundamental/math/math.h"

/* Backend dispatch layer for the math module.
 *
 * A backend is a vtable of kernel function pointers. The built-in CPU backend
 * (the default) forwards to the built-in fun_math_* kernels. A caller may
 * install a custom backend (for example a future GPU/accelerator backend) with
 * fun_math_backend_set; subsequent dispatches route through it. Passing NULL to
 * fun_math_backend_set restores the default CPU backend. Backend selection is
 * separate from, and does not affect, the link-time weak-symbol ISA dispatch of
 * the built-in CPU kernels.
 *
 * fun_math_backend_set is not thread-safe: call it during single-threaded
 * startup, before any dispatch runs. */

typedef enum {
	FUN_MATH_DTYPE_F32 = 0,
	FUN_MATH_DTYPE_F16,
	FUN_MATH_DTYPE_F64,
	FUN_MATH_DTYPE_COUNT
} FunMathDtype;

typedef enum {
	FUN_MATH_OP_SQRT = 0,
	FUN_MATH_OP_EXP,
	FUN_MATH_OP_LOG,
	FUN_MATH_OP_SIN,
	FUN_MATH_OP_COS,
	FUN_MATH_OP_TANH,
	FUN_MATH_OP_SIGMOID,
	FUN_MATH_OP_SILU,
	FUN_MATH_OP_ADD,
	FUN_MATH_OP_MUL,
	FUN_MATH_OP_SUM,
	FUN_MATH_OP_MAX,
	FUN_MATH_OP_COUNT
} FunMathOp;

typedef struct {
	void *data;
	size_t n;
	size_t stride;
} FunMathTensor;

typedef struct FunMathBackend FunMathBackend;

struct FunMathBackend {
	void (*unary)(FunMathOp op, FunMathDtype dtype, const FunMathTensor *in,
				  FunMathTensor *out);
	void (*binary)(FunMathOp op, FunMathDtype dtype, const FunMathTensor *a,
				   const FunMathTensor *b, FunMathTensor *out);
	void (*reduce)(FunMathOp op, FunMathDtype dtype, const FunMathTensor *in,
				   void *out);
	void (*matrix_vector_f32)(const float *w, const float *x, const float *bias,
							  float *out, size_t rows, size_t cols);
	float (*dot_f32)(const float *a, const float *b, size_t n);
	void (*rms_norm_f32)(const float *x, const float *weight, float *out,
						 size_t n, float eps);
	void (*softmax_f32)(float *x, size_t n);
	void (*rotary_f32)(const float *x, const float *cosv, const float *sinv,
					   float *out, size_t n_heads, size_t half);
};

void fun_math_backend_set(const FunMathBackend *backend);
const FunMathBackend *fun_math_backend_get(void);

void fun_math_unary(FunMathOp op, FunMathDtype dtype, const FunMathTensor *in,
					FunMathTensor *out);
void fun_math_binary(FunMathOp op, FunMathDtype dtype, const FunMathTensor *a,
					 const FunMathTensor *b, FunMathTensor *out);
void fun_math_reduce(FunMathOp op, FunMathDtype dtype, const FunMathTensor *in,
					 void *out);

void fun_math_matrix_vector_f32_dispatch(const float *w, const float *x,
										 const float *bias, float *out,
										 size_t rows, size_t cols);
float fun_math_dot_f32_dispatch(const float *a, const float *b, size_t n);
void fun_math_rms_norm_f32_dispatch(const float *x, const float *weight,
									float *out, size_t n, float eps);
void fun_math_softmax_f32_dispatch(float *x, size_t n);
void fun_math_rotary_f32_dispatch(const float *x, const float *cosv,
								  const float *sinv, float *out, size_t n_heads,
								  size_t half);

#endif
