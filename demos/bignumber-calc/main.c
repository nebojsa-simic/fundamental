/*
 * bignumber-calc — demo CLI calculator for big-number arithmetic.
 *
 * Demonstrates the bignumber module: add, sub, mul, modular ops over P-256.
 */
#include "fundamental/bignumber/bignumber.h"
#include "fundamental/console/console.h"

/* 256-bit precision for demo */
#define NL 8

static void print_hex(const char *label, const BigNumber *v)
{
	uint8_t bytes[BIGNUMBER_MAX_LIMBS * 4];
	uint32_t n = v->limbs * 4;
	uint32_t i;

	fun_bignumber_to_bytes(v, bytes);
	fun_console_write(label);
	for (i = 0; i < n; i++) {
		char hex[3];
		hex[0] = "0123456789abcdef"[bytes[i] >> 4];
		hex[1] = "0123456789abcdef"[bytes[i] & 0xF];
		hex[2] = 0;
		fun_console_write(hex);
	}
	fun_console_write("\n");
}

static BigNumber make_p256(void)
{
	uint8_t p[32] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01,
					  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					  0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
					  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	return fun_bignumber_from_bytes(NL, p);
}

int main(void)
{
	BigNumber p = make_p256();

	fun_console_write_line("bignumber-calc — Big Number Calculator Demo");
	fun_console_write_line("==========================================");

	/* --- Integer arithmetic (modulo 2^256) --- */
	{
		fun_console_write_line("\n[Integer arithmetic]");
		BigNumber a = fun_bignumber_from_u32(NL, 0xDEADBEEF);
		BigNumber b = fun_bignumber_from_u32(NL, 0xCAFEBABE);

		BigNumber s = fun_bignumber_add(&a, &b);
		BigNumber d = fun_bignumber_subtract(&b, &a);

		print_hex("  a     = 0x", &a);
		print_hex("  b     = 0x", &b);
		print_hex("  a+b   = 0x", &s);
		print_hex("  b-a   = 0x", &d);
	}

	/* --- Modular arithmetic over P-256 --- */
	{
		fun_console_write_line("\n[Modular arithmetic over P-256]");
		BigNumber a = fun_bignumber_from_u32(NL, 0xDEADBEEF);
		BigNumber b = fun_bignumber_from_u32(NL, 0xCAFEBABE);
		BigNumber one = fun_bignumber_from_u32(NL, 1);

		BigNumber ms = fun_bignumber_modular_add(&a, &b, &p);
		BigNumber md = fun_bignumber_modular_subtract(&b, &a, &p);
		BigNumber mm = fun_bignumber_modular_multiply(&a, &b, &p);

		print_hex("  a     = 0x", &a);
		print_hex("  b     = 0x", &b);
		print_hex("  a+b%p = 0x", &ms);
		print_hex("  b-a%p = 0x", &md);
		print_hex("  a*b%p = 0x", &mm);

		/* Edge cases: overflow and wrap */
		BigNumber pm1 = fun_bignumber_subtract(&p, &one);
		BigNumber two = fun_bignumber_from_u32(NL, 2);
		BigNumber wrap = fun_bignumber_modular_add(&pm1, &two, &p);
		fun_console_write_line("  (p-1)+2 mod p = 1 (wraps): ");
		print_hex("    = 0x", &wrap);
	}

	/* --- Modular inverse over P-256 --- */
	{
		fun_console_write_line("\n[Modular inverse over P-256]");
		BigNumber a = fun_bignumber_from_u32(NL, 3);
		BigNumber inv = fun_bignumber_modular_inverse(&a, &p);
		BigNumber check = fun_bignumber_modular_multiply(&a, &inv, &p);
		print_hex("  inv(3) = 0x", &inv);
		fun_console_write_line(fun_bignumber_is_one(&check) ?
								   "  3*inv(3) mod p = 1: OK" :
								   "  3*inv(3) mod p = 1: FAIL");
	}

	fun_console_write_line("\nDone.");
	fun_console_flush();
	return 0;
}