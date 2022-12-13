/*
 * Copyright (c) 2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32_pka.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <libfdt.h>
#include <plat/common/platform.h>

#include <platform_def.h>

/*
 * For our comprehension in this file
 *  _len are in BITs
 *  _size are in BYTEs
 *  _nbw are in number of PKA_word (PKA_word = u64)
 */

#define UINT8_LEN			8U
#define UINT64_LEN			(UINT8_LEN * sizeof(uint64_t))
#define WORD_SIZE			(sizeof(uint64_t))
#define OP_NBW_FROM_LEN(len)		(DIV_ROUND_UP_2EVAL((len), UINT64_LEN) + 1)
#define OP_NBW_FROM_SIZE(s)		OP_NBW_FROM_LEN((s) * UINT8_LEN)
#define OP_SIZE_FROM_SIZE(s)		(OP_NBW_FROM_SIZE(s) * WORD_SIZE)

#define DT_PKA_COMPAT			"st,stm32-pka64"

#define MAX_ECC_SIZE_LEN		640U
#define MAX_EO_NBW			OP_NBW_FROM_LEN(MAX_ECC_SIZE_LEN)

/* PKA registers */
/* PKA control register */
#define _PKA_CR				0x0U
/* PKA status register */
#define _PKA_SR				0x4U
/* PKA clear flag register */
#define _PKA_CLRFR			0x8U
/* PKA version register */
#define _PKA_VERR			0x1FF4U
/* PKA identification register */
#define _PKA_IPIDR			0x1FF8U

/* PKA control register fields */
#define _PKA_CR_MODE_MASK		GENMASK(13, 8)
#define _PKA_CR_MODE_SHIFT		8U
#define _PKA_CR_MODE_ADD		0x9U
#define _PKA_CR_MODE_ECDSA_VERIF	0x26U
#define _PKA_CR_START			BIT(1)
#define _PKA_CR_EN			BIT(0)

/* PKA status register fields */
#define _PKA_SR_BUSY			BIT(16)
#define _PKA_SR_LMF			BIT(1)
#define _PKA_SR_INITOK			BIT(0)

/* PKA it flag fields (used in CR, SR and CLRFR) */
#define _PKA_IT_MASK			(GENMASK(21, 19) | BIT(17))
#define _PKA_IT_SHIFT			17U
#define _PKA_IT_OPERR			BIT(21)
#define _PKA_IT_ADDRERR			BIT(20)
#define _PKA_IT_RAMERR			BIT(19)
#define _PKA_IT_PROCEND			BIT(17)

/* PKA version register fields */
#define _PKA_VERR_MAJREV_MASK		GENMASK(7, 4)
#define _PKA_VERR_MAJREV_SHIFT		4U
#define _PKA_VERR_MINREV_MASK		GENMASK(3, 0)
#define _PKA_VERR_MINREV_SHIFT		0U

/* RAM magic offset */
#define _PKA_RAM_START			0x400U
#define _PKA_RAM_SIZE			5336U

/* ECDSA verification */
#define _PKA_RAM_N_LEN			0x408U /* 64 */
#define _PKA_RAM_P_LEN			0x4C8U /* 64 */
#define _PKA_RAM_A_SIGN			0x468U /* 64 */
#define _PKA_RAM_A			0x470U /* EOS */
#define _PKA_RAM_P			0x4D0U /* EOS */
#define _PKA_RAM_XG			0x678U /* EOS */
#define _PKA_RAM_YG			0x6D0U /* EOS */
#define _PKA_RAM_XQ			0x12F8U /* EOS */
#define _PKA_RAM_YQ			0x1350U /* EOS */
#define _PKA_RAM_SIGN_R			0x10E0U /* EOS */
#define _PKA_RAM_SIGN_S			0xC68U /* EOS */
#define _PKA_RAM_HASH_Z			0x13A8U /* EOS */
#define _PKA_RAM_PRIME_N		0x1088U /* EOS */
#define _PKA_RAM_ECDSA_VERIFY		0x5D0U /* 64 */
#define _PKA_RAM_ECDSA_VERIFY_VALID	0xD60DULL
#define _PKA_RAM_ECDSA_VERIFY_INVALID	0xA3B7ULL

#define PKA_TIMEOUT_US			1000000U
#define TIMEOUT_US_1MS			1000U
#define PKA_RESET_DELAY			20U

struct curve_parameters {
	uint32_t a_sign;  /* 0 positive, 1 negative */
	uint8_t *a;    /* Curve coefficient |a| */
	size_t a_size;
	uint8_t *p;    /* Curve modulus value */
	uint32_t p_len;
	uint8_t *xg;   /* Curve base point G coordinate x */
	size_t xg_size;
	uint8_t *yg;   /* Curve base point G coordinate y */
	size_t yg_size;
	uint8_t *n;    /* Curve prime order n */
	uint32_t n_len;
};

static const struct curve_parameters curve_def[] = {
#if PKA_USE_NIST_P256
	[PKA_NIST_P256] = {
		.p_len = 256U,
		.n_len = 256U,
		.p  = (uint8_t[]){0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01,
				  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				  0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
				  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
		.n  = (uint8_t[]){0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
				  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				  0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84,
				  0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51},
		.a_sign = 1U,
		.a = (uint8_t[]){0x03},
		.a_size = 1U,
		.xg = (uint8_t[]){0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47,
				  0xF8, 0xBC, 0xE6, 0xE5, 0x63, 0xA4, 0x40, 0xF2,
				  0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33, 0xA0,
				  0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96},
		.xg_size = 32U,
		.yg = (uint8_t[]){0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F, 0x9B,
				  0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E, 0x16,
				  0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE,
				  0xCB, 0xB6, 0x40, 0x68, 0x37, 0xBF, 0x51, 0xF5},
		.yg_size = 32U,
	},
#endif
#if PKA_USE_BRAINPOOL_P256R1
	[PKA_BRAINPOOL_P256R1] = {
		.p_len = 256,
		.n_len = 256,
		.p  = (uint8_t[]){0xA9, 0xFB, 0x57, 0xDB, 0xA1, 0xEE, 0xA9, 0xBC,
				  0x3E, 0x66, 0x0A, 0x90, 0x9D, 0x83, 0x8D, 0x72,
				  0x6E, 0x3B, 0xF6, 0x23, 0xD5, 0x26, 0x20, 0x28,
				  0x20, 0x13, 0x48, 0x1D, 0x1F, 0x6E, 0x53, 0x77},
		.n  = (uint8_t[]){0xA9, 0xFB, 0x57, 0xDB, 0xA1, 0xEE, 0xA9, 0xBC,
				  0x3E, 0x66, 0x0A, 0x90, 0x9D, 0x83, 0x8D, 0x71,
				  0x8C, 0x39, 0x7A, 0xA3, 0xB5, 0x61, 0xA6, 0xF7,
				  0x90, 0x1E, 0x0E, 0x82, 0x97, 0x48, 0x56, 0xA7},
		.a  = (uint8_t[]){0x7D, 0x5A, 0x09, 0x75, 0xFC, 0x2C, 0x30, 0x57,
				  0xEE, 0xF6, 0x75, 0x30, 0x41, 0x7A, 0xFF, 0xE7,
				  0xFB, 0x80, 0x55, 0xC1, 0x26, 0xDC, 0x5C, 0x6C,
				  0xE9, 0x4A, 0x4B, 0x44, 0xF3, 0x30, 0xB5, 0xD9},
		.a_size = 32U,
		.xg = (uint8_t[]){0x8B, 0xD2, 0xAE, 0xB9, 0xCB, 0x7E, 0x57, 0xCB,
				  0x2C, 0x4B, 0x48, 0x2F, 0xFC, 0x81, 0xB7, 0xAF,
				  0xB9, 0xDE, 0x27, 0xE1, 0xE3, 0xBD, 0x23, 0xC2,
				  0x3A, 0x44, 0x53, 0xBD, 0x9A, 0xCE, 0x32, 0x62},
		.xg_size = 32U,
		.yg = (uint8_t[]){0x54, 0x7E, 0xF8, 0x35, 0xC3, 0xDA, 0xC4, 0xFD,
				  0x97, 0xF8, 0x46, 0x1A, 0x14, 0x61, 0x1D, 0xC9,
				  0xC2, 0x77, 0x45, 0x13, 0x2D, 0xED, 0x8E, 0x54,
				  0x5C, 0x1D, 0x54, 0xC7, 0x2F, 0x04, 0x69, 0x97},
		.yg_size = 32U,
	},
#endif
#if PKA_USE_BRAINPOOL_P256T1
	[PKA_BRAINPOOL_P256T1] = {
		.p_len = 256,
		.n_len = 256,
		.p  = (uint8_t[]){0xA9, 0xFB, 0x57, 0xDB, 0xA1, 0xEE, 0xA9, 0xBC,
				  0x3E, 0x66, 0x0A, 0x90, 0x9D, 0x83, 0x8D, 0x72,
				  0x6E, 0x3B, 0xF6, 0x23, 0xD5, 0x26, 0x20, 0x28,
				  0x20, 0x13, 0x48, 0x1D, 0x1F, 0x6E, 0x53, 0x77},
		.n  = (uint8_t[]){0xA9, 0xFB, 0x57, 0xDB, 0xA1, 0xEE, 0xA9, 0xBC,
				  0x3E, 0x66, 0x0A, 0x90, 0x9D, 0x83, 0x8D, 0x71,
				  0x8C, 0x39, 0x7A, 0xA3, 0xB5, 0x61, 0xA6, 0xF7,
				  0x90, 0x1E, 0x0E, 0x82, 0x97, 0x48, 0x56, 0xA7},
		.a  = (uint8_t[]){0xA9, 0xFB, 0x57, 0xDB, 0xA1, 0xEE, 0xA9, 0xBC,
				  0x3E, 0x66, 0x0A, 0x90, 0x9D, 0x83, 0x8D, 0x72,
				  0x6E, 0x3B, 0xF6, 0x23, 0xD5, 0x26, 0x20, 0x28,
				  0x20, 0x13, 0x48, 0x1D, 0x1F, 0x6E, 0x53, 0x74},
		.a_size = 32U,
		.xg = (uint8_t[]){0xA3, 0xE8, 0xEB, 0x3C, 0xC1, 0xCF, 0xE7, 0xB7,
				  0x73, 0x22, 0x13, 0xB2, 0x3A, 0x65, 0x61, 0x49,
				  0xAF, 0xA1, 0x42, 0xC4, 0x7A, 0xAF, 0xBC, 0x2B,
				  0x79, 0xA1, 0x91, 0x56, 0x2E, 0x13, 0x05, 0xF4},
		.xg_size = 32U,
		.yg = (uint8_t[]){0x2D, 0x99, 0x6C, 0x82, 0x34, 0x39, 0xC5, 0x6D,
				  0x7F, 0x7B, 0x22, 0xE1, 0x46, 0x44, 0x41, 0x7E,
				  0x69, 0xBC, 0xB6, 0xDE, 0x39, 0xD0, 0x27, 0x00,
				  0x1D, 0xAB, 0xE8, 0xF3, 0x5B, 0x25, 0xC9, 0xBE},
		.yg_size = 32U,
	},
#endif
#if PKA_USE_NIST_P521
	[PKA_NIST_P521] = {
		.p_len = 521,
		.n_len = 521,
		.p  = (uint8_t[]){                                    0x01, 0xff,
				  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		.n  = (uint8_t[]){                                    0x01, 0xff,
				  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfa,
				  0x51, 0x86, 0x87, 0x83, 0xbf, 0x2f, 0x96, 0x6b,
				  0x7f, 0xcc, 0x01, 0x48, 0xf7, 0x09, 0xa5, 0xd0,
				  0x3b, 0xb5, 0xc9, 0xb8, 0x89, 0x9c, 0x47, 0xae,
				  0xbb, 0x6f, 0xb7, 0x1e, 0x91, 0x38, 0x64, 0x09},
		.a_sign = 1,
		.a  = (uint8_t[]){0x03},
		.a_size = 1U,
		.xg = (uint8_t[]){                                          0xc6,
				  0x85, 0x8e, 0x06, 0xb7, 0x04, 0x04, 0xe9, 0xcd,
				  0x9e, 0x3e, 0xcb, 0x66, 0x23, 0x95, 0xb4, 0x42,
				  0x9c, 0x64, 0x81, 0x39, 0x05, 0x3f, 0xb5, 0x21,
				  0xf8, 0x28, 0xaf, 0x60, 0x6b, 0x4d, 0x3d, 0xba,
				  0xa1, 0x4b, 0x5e, 0x77, 0xef, 0xe7, 0x59, 0x28,
				  0xfe, 0x1d, 0xc1, 0x27, 0xa2, 0xff, 0xa8, 0xde,
				  0x33, 0x48, 0xb3, 0xc1, 0x85, 0x6a, 0x42, 0x9b,
				  0xf9, 0x7e, 0x7e, 0x31, 0xc2, 0xe5, 0xbd, 0x66},
		.xg_size = 65U,
		.yg = (uint8_t[]){                                    0x01, 0x18,
				  0x39, 0x29, 0x6a, 0x78, 0x9a, 0x3b, 0xc0, 0x04,
				  0x5c, 0x8a, 0x5f, 0xb4, 0x2c, 0x7d, 0x1b, 0xd9,
				  0x98, 0xf5, 0x44, 0x49, 0x57, 0x9b, 0x44, 0x68,
				  0x17, 0xaf, 0xbd, 0x17, 0x27, 0x3e, 0x66, 0x2c,
				  0x97, 0xee, 0x72, 0x99, 0x5e, 0xf4, 0x26, 0x40,
				  0xc5, 0x50, 0xb9, 0x01, 0x3f, 0xad, 0x07, 0x61,
				  0x35, 0x3c, 0x70, 0x86, 0xa2, 0x72, 0xc2, 0x40,
				  0x88, 0xbe, 0x94, 0x76, 0x9f, 0xd1, 0x66, 0x50},
		.yg_size = 66U,
	},
#endif
};

static struct stm32_pka_platdata pka_pdata;

static int stm32_pka_parse_fdt(void)
{
	int node;
	struct dt_node_info info;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	node = dt_get_node(&info, -1, DT_PKA_COMPAT);
	if (node < 0) {
		ERROR("No PKA entry in DT\n");
		return -FDT_ERR_NOTFOUND;
	}

	if (info.status == DT_DISABLED) {
		return -FDT_ERR_NOTFOUND;
	}

	if ((info.base == 0) || (info.clock < 0) || (info.reset < 0)) {
		return -FDT_ERR_BADVALUE;
	}

	pka_pdata.base = (uintptr_t)info.base;
	pka_pdata.clock_id = (unsigned long)info.clock;
	pka_pdata.reset_id = (unsigned int)info.reset;

	return 0;
}

static int pka_wait_bit(uintptr_t base, uint32_t bit)
{
	uint64_t timeout = timeout_init_us(PKA_TIMEOUT_US);

	while ((mmio_read_32(base + _PKA_SR) & bit) != bit) {
		if (timeout_elapsed(timeout)) {
			WARN("timeout waiting %x\n", bit);
			return -ETIMEDOUT;
		}
	}

	return 0;

}

static void pka_disable(uintptr_t base)
{
	mmio_clrbits_32(base + _PKA_CR, _PKA_CR_EN);
}

static int pka_enable(uintptr_t base, uint32_t mode)
{
	/* Set mode and disable interrupts */
	mmio_clrsetbits_32(base + _PKA_CR, _PKA_IT_MASK | _PKA_CR_MODE_MASK,
			   _PKA_CR_MODE_MASK & (mode << _PKA_CR_MODE_SHIFT));

	mmio_setbits_32(base + _PKA_CR, _PKA_CR_EN);

	return pka_wait_bit(base, _PKA_SR_INITOK);
}

/*
 * Data are already loaded in PKA internal RAM
 * MODE is set
 * We start process, and wait for its end.
 */
static int stm32_pka_process(uintptr_t base)
{
	mmio_setbits_32(base + _PKA_CR, _PKA_CR_START);

	return pka_wait_bit(base, _PKA_IT_PROCEND);
}

/**
 * @brief Write ECC operand to PKA RAM.
 * @note  PKA expect to write u64 word, each u64 are: the least significant bit is
 *        bit 0; the most significant bit is bit 63.
 *        We write eo_nbw (ECC operand Size) u64, value that depends of the chosen
 *        prime modulus length in bits.
 *        First less signicant u64 is written to low address
 *        Most significant u64 to higher address.
 *        And at last address we write a u64(0x0)
 * @note  This function doesn't only manage endianness (as bswap64 do), but also
 *        complete most significant incomplete u64 with 0 (if data is not a u64
 *        multiple), and fill u64 last address with 0.
 * @param addr: PKA_RAM address to write the buffer 'data'
 * @param data: is a BYTE list with most significant bytes first
 * @param data_size: nb of byte in data
 * @param eo_nbw: is ECC Operand size in 64bits word (including the extra 0)
 *                (note it depends of the prime modulus length, not the data size)
 * @retval 0 if OK.
 *         -EINVAL if data_size and eo_nbw are inconsistent, ie data doesn't
 *         fit in defined eo_nbw, or eo_nbw bigger than hardware limit.
 */
static int write_eo_data(uintptr_t addr, uint8_t *data, unsigned int data_size,
			 unsigned int eo_nbw)
{
	uint32_t word_index;
	int data_index;

	if ((eo_nbw < OP_NBW_FROM_SIZE(data_size)) || (eo_nbw > MAX_EO_NBW)) {
		return -EINVAL;
	}

	/* Fill value */
	data_index = (int)data_size - 1;
	for (word_index = 0U; word_index < eo_nbw; word_index++) {
		uint64_t tmp = 0ULL;
		unsigned int i = 0U;  /* index in the tmp U64 word */

		/* Stop if end of tmp or end of data */
		while ((i < sizeof(tmp)) && (data_index >= 0)) {
			tmp |= (uint64_t)(data[data_index]) << (UINT8_LEN * i);
			i++; /* Move byte index in current (u64)tmp */
			data_index--; /* Move to just next most significat byte */
		}

		mmio_write_64(addr + word_index * sizeof(tmp), tmp);
	}

	return 0;
}

static unsigned int get_ecc_op_nbword(enum stm32_pka_ecdsa_curve_id cid)
{
	if (cid >= ARRAY_SIZE(curve_def)) {
		ERROR("CID %u is out of boundaries\n", cid);
		panic();
	}

	return OP_NBW_FROM_LEN(curve_def[cid].n_len);
}

static int stm32_pka_ecdsa_verif_configure_curve(uintptr_t base, enum stm32_pka_ecdsa_curve_id cid)
{
	int ret;
	unsigned int eo_nbw = get_ecc_op_nbword(cid);

	mmio_write_64(base + _PKA_RAM_N_LEN, curve_def[cid].n_len);
	mmio_write_64(base + _PKA_RAM_P_LEN, curve_def[cid].p_len);
	mmio_write_64(base + _PKA_RAM_A_SIGN, curve_def[cid].a_sign);

	ret = write_eo_data(base + _PKA_RAM_A, curve_def[cid].a, curve_def[cid].a_size, eo_nbw);
	if (ret < 0) {
		return ret;
	}

	ret = write_eo_data(base + _PKA_RAM_PRIME_N,
			    curve_def[cid].n, div_round_up(curve_def[cid].n_len, UINT8_LEN),
			    eo_nbw);
	if (ret < 0) {
		return ret;
	}

	ret = write_eo_data(base + _PKA_RAM_P, curve_def[cid].p,
			    div_round_up(curve_def[cid].p_len, UINT8_LEN), eo_nbw);
	if (ret < 0) {
		return ret;
	}

	ret = write_eo_data(base + _PKA_RAM_XG, curve_def[cid].xg, curve_def[cid].xg_size, eo_nbw);
	if (ret < 0) {
		return ret;
	}

	ret = write_eo_data(base + _PKA_RAM_YG, curve_def[cid].yg, curve_def[cid].yg_size, eo_nbw);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int stm32_pka_ecdsa_verif_check_return(uintptr_t base)
{
	uint64_t value;
	uint32_t sr;

	sr = mmio_read_32(base + _PKA_SR);
	if ((sr & (_PKA_IT_OPERR | _PKA_IT_ADDRERR | _PKA_IT_RAMERR)) != 0) {
		WARN("Detected error(s): %s%s%s\n",
		     (sr & _PKA_IT_OPERR) ? "Operation " : "",
		     (sr & _PKA_IT_ADDRERR) ? "Address " : "",
		     (sr & _PKA_IT_RAMERR) ? "RAM" : "");
		return -EINVAL;
	}

	value = mmio_read_64(base + _PKA_RAM_ECDSA_VERIFY);
	if (value == _PKA_RAM_ECDSA_VERIFY_VALID) {
		return 0;
	}

	if (value == _PKA_RAM_ECDSA_VERIFY_INVALID) {
		return -EAUTH;
	}

	return -EINVAL;
}

/**
 * @brief Check if BigInt stored in data is 0
 *
 * @param data: a BYTE array with most significant bytes first
 * @param size: data size
 *
 * @retval: true: if data represents a 0 value (ie all bytes == 0)
 *          false: if data represents a non-zero value.
 */
static bool is_zero(uint8_t *data, unsigned int size)
{
	unsigned int i;

	for (i = 0U; i < size; i++) {
		if (data[i] != 0U) {
			return false;
		}
	}

	return true;
}

/**
 * @brief Compare two BigInt:
 * @param xdata_a: a BYTE array with most significant bytes first
 * @param size_a: nb of Byte of 'a'
 * @param data_b: a BYTE array with most significant bytes first
 * @param size_b: nb of Byte of 'b'
 *
 * @retval: true if data_a < data_b
 *          false if data_a >= data_b
 */
static bool is_smaller(uint8_t *data_a, unsigned int size_a,
		       uint8_t *data_b, unsigned int size_b)
{
	unsigned int i;

	i = MAX(size_a, size_b) + 1U;
	do {
		uint8_t a, b;

		i--;
		if (size_a < i) {
			a = 0U;
		} else {
			a = data_a[size_a - i];
		}

		if (size_b < i) {
			b = 0U;
		} else {
			b = data_b[size_b - i];
		}

		if (a < b) {
			return true;
		}

		if (a > b) {
			return false;
		}
	} while (i != 0U);

	return false;
}

static int stm32_pka_ecdsa_check_param(void *sig_r_ptr, unsigned int sig_r_size,
				       void *sig_s_ptr, unsigned int sig_s_size,
				       void *pk_x_ptr, unsigned int pk_x_size,
				       void *pk_y_ptr, unsigned int pk_y_size,
				       enum stm32_pka_ecdsa_curve_id cid)
{
	/* Public Key check */
	/* Check Xq < p */
	if (!is_smaller(pk_x_ptr, pk_x_size,
			curve_def[cid].p, div_round_up(curve_def[cid].p_len, UINT8_LEN))) {
		WARN("%s Xq < p inval\n", __func__);
		return -EINVAL;
	}

	/* Check Yq < p */
	if (!is_smaller(pk_y_ptr, pk_y_size,
			curve_def[cid].p, div_round_up(curve_def[cid].p_len, UINT8_LEN))) {
		WARN("%s Yq < p inval\n", __func__);
		return -EINVAL;
	}

	/* Signature check */
	/* Check 0 < r < n */
	if (!is_smaller(sig_r_ptr, sig_r_size,
			curve_def[cid].n, div_round_up(curve_def[cid].n_len, UINT8_LEN)) &&
	    !is_zero(sig_r_ptr, sig_r_size)) {
		WARN("%s 0< r < n inval\n", __func__);
		return -EINVAL;
	}

	/* Check 0 < s < n */
	if (!is_smaller(sig_s_ptr, sig_s_size,
			curve_def[cid].n, div_round_up(curve_def[cid].n_len, UINT8_LEN)) &&
	    !is_zero(sig_s_ptr, sig_s_size)) {
		WARN("%s 0< s < n inval\n", __func__);
		return -EINVAL;
	}

	return 0;
}

/*
 * @brief  Initialize the PKA driver.
 * @param  None.
 * @retval 0 if OK, negative value else.
 */
int stm32_pka_init(void)
{
	int err;
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	uint32_t ver;
	uint32_t id;
#endif

	err = stm32_pka_parse_fdt();
	if (err != 0) {
		return err;
	}

	clk_enable(pka_pdata.clock_id);

	if (stm32mp_reset_assert((unsigned long)pka_pdata.reset_id, TIMEOUT_US_1MS) != 0) {
		panic();
	}

	udelay(PKA_RESET_DELAY);
	if (stm32mp_reset_deassert((unsigned long)pka_pdata.reset_id, TIMEOUT_US_1MS) != 0) {
		panic();
	}

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	id = mmio_read_32(pka_pdata.base + _PKA_IPIDR);
	ver = mmio_read_32(pka_pdata.base + _PKA_VERR);

	VERBOSE("STM32 PKA[%x] V%u.%u\n", id,
		(ver & _PKA_VERR_MAJREV_MASK) >> _PKA_VERR_MAJREV_SHIFT,
		(ver & _PKA_VERR_MINREV_MASK) >> _PKA_VERR_MINREV_SHIFT);
#endif
	return 0;
}

int stm32_pka_ecdsa_verif(void *hash, unsigned int hash_size,
			  void *sig_r_ptr, unsigned int sig_r_size,
			  void *sig_s_ptr, unsigned int sig_s_size,
			  void *pk_x_ptr, unsigned int pk_x_size,
			  void *pk_y_ptr, unsigned int pk_y_size,
			  enum stm32_pka_ecdsa_curve_id cid)
{
	int ret;
	uintptr_t base = pka_pdata.base;
	unsigned int eo_nbw = get_ecc_op_nbword(cid);

	if ((hash == NULL) || (sig_r_ptr == NULL) || (sig_s_ptr == NULL) ||
	    (pk_x_ptr == NULL) || (pk_y_ptr == NULL)) {
		INFO("%s invalid input param\n", __func__);
		return -EINVAL;
	}

	ret = stm32_pka_ecdsa_check_param(sig_r_ptr, sig_r_size,
					  sig_s_ptr, sig_s_size,
					  pk_x_ptr, pk_x_size,
					  pk_y_ptr, pk_y_size,
					  cid);
	if (ret < 0) {
		INFO("%s check param error %d\n", __func__, ret);
		goto out;
	}

	if ((mmio_read_32(base + _PKA_SR) & _PKA_SR_BUSY) == _PKA_SR_BUSY) {
		INFO("%s busy\n", __func__);
		ret = -EBUSY;
		goto out;
	}

	/* Fill PKA RAM */
	/* With curve id values */
	ret = stm32_pka_ecdsa_verif_configure_curve(base, cid);
	if (ret < 0) {
		goto out;
	}

	/* With pubkey */
	ret = write_eo_data(base + _PKA_RAM_XQ, pk_x_ptr, pk_x_size, eo_nbw);
	if (ret < 0) {
		goto out;
	}

	ret = write_eo_data(base + _PKA_RAM_YQ, pk_y_ptr, pk_y_size, eo_nbw);
	if (ret < 0) {
		goto out;
	}

	/* With hash */
	ret = write_eo_data(base + _PKA_RAM_HASH_Z, hash, hash_size, eo_nbw);
	if (ret < 0) {
		goto out;
	}

	/* With signature */
	ret = write_eo_data(base + _PKA_RAM_SIGN_R, sig_r_ptr, sig_r_size, eo_nbw);
	if (ret < 0) {
		goto out;
	}

	ret = write_eo_data(base + _PKA_RAM_SIGN_S, sig_s_ptr, sig_s_size, eo_nbw);
	if (ret < 0) {
		goto out;
	}

	/* Set mode to ecdsa signature verification */
	ret = pka_enable(base, _PKA_CR_MODE_ECDSA_VERIF);
	if (ret < 0) {
		WARN("%s set mode pka error %d\n", __func__, ret);
		goto out;
	}

	/* Start processing and wait end */
	ret = stm32_pka_process(base);
	if (ret < 0) {
		WARN("%s process error %d\n", __func__, ret);
		goto out;
	}

	/* Check return status */
	ret = stm32_pka_ecdsa_verif_check_return(base);

	/* Unset end proc */
	mmio_setbits_32(base + _PKA_CLRFR, _PKA_IT_PROCEND);

out:
	/* Disable PKA (will stop all pending proccess and reset RAM) */
	pka_disable(base);

	return ret;
}
