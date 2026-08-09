#include <stdio.h>
#include <stdint.h>
#include "test_harness.h"
#include "test_data/q8_dequant_f32_golden.h"
int main() {
    for (int ci = 0; ci < 30 && q8_dequant_f32_cases[ci].n > 0; ci++) {
        int n = q8_dequant_f32_cases[ci].n;
        float out[256];
        fun_math_q8_dequant_row_f32(q8_dequant_f32_cases[ci].src, out, n);
        int any_fail = 0;
        for (int j = 0; j < n && !any_fail; j++) {
            if (!_math_test_check_float(out[j], q8_dequant_f32_cases[ci].expected[j], 1e-4f, 1e-3f)) {
                printf("CASE %d [%d]: got=%.9g want=%.9g\n", ci, j, (double)out[j], (double)q8_dequant_f32_cases[ci].expected[j]);
                any_fail = 1;
            }
        }
    }
    return 0;
}
