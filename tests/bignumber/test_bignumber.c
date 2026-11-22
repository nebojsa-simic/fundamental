/* bignumber module tests */
#include "fundamental/bignumber/bignumber.h"
#include "fundamental/console/console.h"

#define NL 8
#define GREEN_CHECK "\033[0;32m\u2713\033[0m"

static int g_failures;

static void expect(int cond, const char *name)
{
	if (cond) {
		fun_console_write(GREEN_CHECK);
		fun_console_write(" ");
		fun_console_write_line(name);
	} else {
		fun_console_write("FAIL: ");
		fun_console_write_line(name);
		g_failures++;
	}
}

static BigNumber make_p256(void)
{
	static const uint8_t p[32] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
								   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
								   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
								   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
								   0xFF, 0xFF, 0xFF, 0xFF };
	return fun_bignumber_from_bytes(NL, p);
}

int main(void)
{
	BigNumber p = make_p256();
	BigNumber e8 = fun_bignumber_from_u32(NL, 8);
	BigNumber e2 = fun_bignumber_from_u32(NL, 2);
	BigNumber one = fun_bignumber_from_u32(NL, 1);
	BigNumber three = fun_bignumber_from_u32(NL, 3);
	BigNumber a = fun_bignumber_from_u32(NL, 3);
	BigNumber b = fun_bignumber_from_u32(NL, 5);

	/* basic arithmetic */
	{
		BigNumber s = fun_bignumber_add(&a, &b);
		BigNumber d = fun_bignumber_subtract(&b, &a);
		expect(fun_bignumber_compare(&s, &e8) == 0, "test_add");
		expect(fun_bignumber_compare(&d, &e2) == 0, "test_sub");
	}

	{
		BigNumber z = fun_bignumber_zero(NL);
		BigNumber two = fun_bignumber_from_u32(NL, 2);
		expect(fun_bignumber_is_zero(&z) && !fun_bignumber_is_zero(&one),
			   "test_is_zero");
		expect(fun_bignumber_is_one(&one) && !fun_bignumber_is_one(&two),
			   "test_is_one");
	}

	/* modular add */
	{
		BigNumber s = fun_bignumber_modular_add(&a, &b, &p);
		expect(fun_bignumber_compare(&s, &e8) == 0, "test_modadd_basic");
	}
	{
		BigNumber pm1 = fun_bignumber_subtract(&p, &one);
		BigNumber s = fun_bignumber_modular_add(&pm1, &e2, &p);
		expect(fun_bignumber_compare(&s, &one) == 0, "test_modadd_wrap");
	}

	/* modular sub */
	{
		BigNumber s = fun_bignumber_modular_subtract(&b, &a, &p);
		expect(fun_bignumber_compare(&s, &e2) == 0, "test_modsub_basic");
	}
	{
		BigNumber expected = fun_bignumber_subtract(&p, &three);
		BigNumber r = fun_bignumber_modular_subtract(&e2, &b, &p);
		expect(fun_bignumber_compare(&r, &expected) == 0, "test_modsub_wrap");
	}

	/* modular multiply */
	{
		BigNumber p15 = fun_bignumber_from_u32(NL, 15);
		BigNumber prod = fun_bignumber_modular_multiply(&a, &b, &p);
		expect(fun_bignumber_compare(&prod, &p15) == 0, "test_modmul_small");
	}
	{
		BigNumber pm1 = fun_bignumber_subtract(&p, &one);
		BigNumber pm2 = fun_bignumber_subtract(&p, &e2);
		BigNumber prod = fun_bignumber_modular_multiply(&pm1, &e2, &p);
		expect(fun_bignumber_compare(&prod, &pm2) == 0,
			   "test_modmul_pm1_times_2");
	}
	{
		BigNumber pm1 = fun_bignumber_subtract(&p, &one);
		BigNumber sq = fun_bignumber_modular_multiply(&pm1, &pm1, &p);
		expect(fun_bignumber_compare(&sq, &one) == 0,
			   "test_modmul_pm1_squared");
	}

	/* modular inverse */
	{
		BigNumber inv = fun_bignumber_modular_inverse(&a, &p);
		BigNumber check = fun_bignumber_modular_multiply(&a, &inv, &p);
		expect(fun_bignumber_is_one(&check), "test_modinv_3");
	}
	{
		BigNumber pm1 = fun_bignumber_subtract(&p, &one);
		BigNumber inv = fun_bignumber_modular_inverse(&pm1, &p);
		expect(fun_bignumber_compare(&inv, &pm1) == 0, "test_modinv_pm1");
	}

	fun_console_flush();
	return g_failures ? 1 : 0;
}