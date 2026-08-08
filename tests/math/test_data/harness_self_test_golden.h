typedef struct {
	float input;
	float expected;
	float abs_tol;
	int should_pass;
} harness_self_case;
static const harness_self_case harness_self_cases[] = {
	{ 4.0f, 2.0f, 1e-4f, 1 },
	{ 9.0f, 2.0f, 1e-4f, 0 },
	{ 0 },
};
