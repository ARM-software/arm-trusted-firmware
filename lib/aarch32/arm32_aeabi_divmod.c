/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Form ABI specifications:
 *      int __aeabi_idiv(int numerator, int denominator);
 *     unsigned __aeabi_uidiv(unsigned numerator, unsigned denominator);
 *
 *     typedef struct { int quot; int rem; } idiv_return;
 *     typedef struct { unsigned quot; unsigned rem; } uidiv_return;
 *
 *     __value_in_regs idiv_return __aeabi_idivmod(int numerator,
 *     int *denominator);
 *     __value_in_regs uidiv_return __aeabi_uidivmod(unsigned *numerator,
 *     unsigned denominator);
 */

/* struct qr - stores quotient/remainder to handle divmod EABI interfaces. */
struct qr {
	unsigned int q;		/* computed quotient */
	unsigned int r;		/* computed remainder */
	unsigned int q_n;	/* specifies if quotient shall be negative */
	unsigned int r_n;	/* specifies if remainder shall be negative */
};

static void uint_div_qr(unsigned int numerator, unsigned int denominator,
			struct qr *qr);

/* returns in R0 and R1 by tail calling an asm function */
unsigned int __aeabi_uidivmod(unsigned int numerator, unsigned int denominator);

unsigned int __aeabi_uidiv(unsigned int numerator, unsigned int denominator);

/* returns in R0 and R1 by tail calling an asm function */
signed int __aeabi_idivmod(signed int numerator, signed int denominator);

signed int __aeabi_idiv(signed int numerator, signed int denominator);

/*
 * __ste_idivmod_ret_t __aeabi_idivmod(signed numerator, signed denominator)
 * Numerator and Denominator are received in R0 and R1.
 * Where __ste_idivmod_ret_t is returned in R0 and R1.
 *
 * __ste_uidivmod_ret_t __aeabi_uidivmod(unsigned numerator,
 *                                       unsigned denominator)
 * Numerator and Denominator are received in R0 and R1.
 * Where __ste_uidivmod_ret_t is returned in R0 and R1.
 */
#ifdef __GNUC__
signed int ret_idivmod_values(signed int quotient, signed int remainder);
unsigned int ret_uidivmod_values(unsigned int quotient, unsigned int remainder);
#else
#error "Compiler not supported"
#endif

static void division_qr(unsigned int n, unsigned int p, struct qr *qr)
{
	unsigned int i = 1, q = 0;

	if (p == 0) {
		qr->r = 0xFFFFFFFF;	/* division by 0 */
		return;
	}

	while ((p >> 31) == 0) {
		i = i << 1;	/* count the max division steps */
		p = p << 1;     /* increase p until it has maximum size*/
	}

	while (i > 0) {
		q = q << 1;	/* write bit in q at index (size-1) */
		if (n >= p) {
			n -= p;
			q++;
		}
		p = p >> 1;	/* decrease p */
		i = i >> 1;	/* decrease remaining size in q */
	}
	qr->r = n;
	qr->q = q;
}

static void uint_div_qr(unsigned int numerator, unsigned int denominator,
			struct qr *qr)
{
	division_qr(numerator, denominator, qr);

	/* negate quotient and/or remainder according to requester */
	if (qr->q_n)
		qr->q = -qr->q;
	if (qr->r_n)
		qr->r = -qr->r;
}

unsigned int __aeabi_uidiv(unsigned int numerator, unsigned int denominator)
{
	struct qr qr = { .q_n = 0, .r_n = 0 };

	uint_div_qr(numerator, denominator, &qr);

	return qr.q;
}

unsigned int __aeabi_uidivmod(unsigned int numerator, unsigned int denominator)
{
	struct qr qr = { .q_n = 0, .r_n = 0 };

	uint_div_qr(numerator, denominator, &qr);

	return ret_uidivmod_values(qr.q, qr.r);
}

signed int __aeabi_idiv(signed int numerator, signed int denominator)
{
	struct qr qr = { .q_n = 0, .r_n = 0 };

	if (((numerator < 0) && (denominator > 0)) ||
	    ((numerator > 0) && (denominator < 0)))
		qr.q_n = 1;	/* quotient shall be negate */

	if (numerator < 0) {
		numerator = -numerator;
		qr.r_n = 1;	/* remainder shall be negate */
	}

	if (denominator < 0)
		denominator = -denominator;

	uint_div_qr(numerator, denominator, &qr);

	return qr.q;
}

signed int __aeabi_idivmod(signed int numerator, signed int denominator)
{
	struct qr qr = { .q_n = 0, .r_n = 0 };

	if (((numerator < 0) && (denominator > 0)) ||
	    ((numerator > 0) && (denominator < 0)))
		qr.q_n = 1;	/* quotient shall be negate */

	if (numerator < 0) {
		numerator = -numerator;
		qr.r_n = 1;	/* remainder shall be negate */
	}

	if (denominator < 0)
		denominator = -denominator;

	uint_div_qr(numerator, denominator, &qr);

	return ret_idivmod_values(qr.q, qr.r);
}
