/*
 * Copyright (c) 2019-2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

#include <libfdt.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/raw_nand.h>
#include <drivers/st/stm32_fmc2_nand.h>
#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

/* Timeout for device interface reset */
#define TIMEOUT_US_1_MS			1000U

/* FMC2 Compatibility */
#define DT_FMC2_EBI_COMPAT		"st,stm32mp1-fmc2-ebi"
#define DT_FMC2_NFC_COMPAT		"st,stm32mp1-fmc2-nfc"
#define MAX_CS				2U
#define MAX_BANK			5U

/* FMC2 Controller Registers */
#define FMC2_BCR1			0x00U
#define FMC2_PCR			0x80U
#define FMC2_SR				0x84U
#define FMC2_PMEM			0x88U
#define FMC2_PATT			0x8CU
#define FMC2_HECCR			0x94U
#define FMC2_BCHISR			0x254U
#define FMC2_BCHICR			0x258U
#define FMC2_BCHDSR0			0x27CU
#define FMC2_BCHDSR1			0x280U
#define FMC2_BCHDSR2			0x284U
#define FMC2_BCHDSR3			0x288U
#define FMC2_BCHDSR4			0x28CU

/* FMC2_BCR1 register */
#define FMC2_BCR1_FMC2EN		BIT(31)
/* FMC2_PCR register */
#define FMC2_PCR_PWAITEN		BIT(1)
#define FMC2_PCR_PBKEN			BIT(2)
#define FMC2_PCR_PWID_MASK		GENMASK_32(5, 4)
#define FMC2_PCR_PWID(x)		(((x) << 4) & FMC2_PCR_PWID_MASK)
#define FMC2_PCR_PWID_8			0x0U
#define FMC2_PCR_PWID_16		0x1U
#define FMC2_PCR_ECCEN			BIT(6)
#define FMC2_PCR_ECCALG			BIT(8)
#define FMC2_PCR_TCLR_MASK		GENMASK_32(12, 9)
#define FMC2_PCR_TCLR(x)		(((x) << 9) & FMC2_PCR_TCLR_MASK)
#define FMC2_PCR_TCLR_DEFAULT		0xFU
#define FMC2_PCR_TAR_MASK		GENMASK_32(16, 13)
#define FMC2_PCR_TAR(x)			(((x) << 13) & FMC2_PCR_TAR_MASK)
#define FMC2_PCR_TAR_DEFAULT		0xFU
#define FMC2_PCR_ECCSS_MASK		GENMASK_32(19, 17)
#define FMC2_PCR_ECCSS(x)		(((x) << 17) & FMC2_PCR_ECCSS_MASK)
#define FMC2_PCR_ECCSS_512		0x1U
#define FMC2_PCR_ECCSS_2048		0x3U
#define FMC2_PCR_BCHECC			BIT(24)
#define FMC2_PCR_WEN			BIT(25)
/* FMC2_SR register */
#define FMC2_SR_NWRF			BIT(6)
/* FMC2_PMEM register*/
#define FMC2_PMEM_MEMSET(x)		(((x) & GENMASK_32(7, 0)) << 0)
#define FMC2_PMEM_MEMWAIT(x)		(((x) & GENMASK_32(7, 0)) << 8)
#define FMC2_PMEM_MEMHOLD(x)		(((x) & GENMASK_32(7, 0)) << 16)
#define FMC2_PMEM_MEMHIZ(x)		(((x) & GENMASK_32(7, 0)) << 24)
#define FMC2_PMEM_DEFAULT		0x0A0A0A0AU
/* FMC2_PATT register */
#define FMC2_PATT_ATTSET(x)		(((x) & GENMASK_32(7, 0)) << 0)
#define FMC2_PATT_ATTWAIT(x)		(((x) & GENMASK_32(7, 0)) << 8)
#define FMC2_PATT_ATTHOLD(x)		(((x) & GENMASK_32(7, 0)) << 16)
#define FMC2_PATT_ATTHIZ(x)		(((x) & GENMASK_32(7, 0)) << 24)
#define FMC2_PATT_DEFAULT		0x0A0A0A0AU
/* FMC2_BCHISR register */
#define FMC2_BCHISR_DERF		BIT(1)
/* FMC2_BCHICR register */
#define FMC2_BCHICR_CLEAR_IRQ		GENMASK_32(4, 0)
/* FMC2_BCHDSR0 register */
#define FMC2_BCHDSR0_DUE		BIT(0)
#define FMC2_BCHDSR0_DEF		BIT(1)
#define FMC2_BCHDSR0_DEN_MASK		GENMASK_32(7, 4)
#define FMC2_BCHDSR0_DEN_SHIFT		4U
/* FMC2_BCHDSR1 register */
#define FMC2_BCHDSR1_EBP1_MASK		GENMASK_32(12, 0)
#define FMC2_BCHDSR1_EBP2_MASK		GENMASK_32(28, 16)
#define FMC2_BCHDSR1_EBP2_SHIFT		16U
/* FMC2_BCHDSR2 register */
#define FMC2_BCHDSR2_EBP3_MASK		GENMASK_32(12, 0)
#define FMC2_BCHDSR2_EBP4_MASK		GENMASK_32(28, 16)
#define FMC2_BCHDSR2_EBP4_SHIFT		16U
/* FMC2_BCHDSR3 register */
#define FMC2_BCHDSR3_EBP5_MASK		GENMASK_32(12, 0)
#define FMC2_BCHDSR3_EBP6_MASK		GENMASK_32(28, 16)
#define FMC2_BCHDSR3_EBP6_SHIFT		16U
/* FMC2_BCHDSR4 register */
#define FMC2_BCHDSR4_EBP7_MASK		GENMASK_32(12, 0)
#define FMC2_BCHDSR4_EBP8_MASK		GENMASK_32(28, 16)
#define FMC2_BCHDSR4_EBP8_SHIFT		16U

/* Timings */
#define FMC2_THIZ			0x01U
#define FMC2_TIO			8000U
#define FMC2_TSYNC			3000U
#define FMC2_PCR_TIMING_MASK		GENMASK_32(3, 0)
#define FMC2_PMEM_PATT_TIMING_MASK	GENMASK_32(7, 0)

#define FMC2_BBM_LEN			2U
#define FMC2_MAX_ECC_BYTES		14U
#define TIMEOUT_US_10_MS		10000U
#define FMC2_PSEC_PER_MSEC		(1000UL * 1000UL * 1000UL)

enum stm32_fmc2_ecc {
	FMC2_ECC_HAM = 1U,
	FMC2_ECC_BCH4 = 4U,
	FMC2_ECC_BCH8 = 8U
};

struct stm32_fmc2_cs_reg {
	uintptr_t data_base;
	uintptr_t cmd_base;
	uintptr_t addr_base;
};

struct stm32_fmc2_nand_timings {
	uint8_t tclr;
	uint8_t tar;
	uint8_t thiz;
	uint8_t twait;
	uint8_t thold_mem;
	uint8_t tset_mem;
	uint8_t thold_att;
	uint8_t tset_att;
};

struct stm32_fmc2_nfc {
	uintptr_t reg_base;
	struct stm32_fmc2_cs_reg cs[MAX_CS];
	unsigned long clock_id;
	unsigned int reset_id;
	uint8_t cs_sel;
};

static struct stm32_fmc2_nfc stm32_fmc2;

static uintptr_t fmc2_base(void)
{
	return stm32_fmc2.reg_base;
}

static void stm32_fmc2_nand_setup_timing(void)
{
	struct stm32_fmc2_nand_timings tims;
	unsigned long hclk = stm32mp_clk_get_rate(stm32_fmc2.clock_id);
	unsigned long hclkp = FMC2_PSEC_PER_MSEC / (hclk / 1000U);
	unsigned long timing, tar, tclr, thiz, twait;
	unsigned long tset_mem, tset_att, thold_mem, thold_att;
	uint32_t pcr, pmem, patt;

	tar = MAX(hclkp, NAND_TAR_MIN);
	timing = div_round_up(tar, hclkp) - 1U;
	tims.tar = MIN(timing, (unsigned long)FMC2_PCR_TIMING_MASK);

	tclr = MAX(hclkp, NAND_TCLR_MIN);
	timing = div_round_up(tclr, hclkp) - 1U;
	tims.tclr = MIN(timing, (unsigned long)FMC2_PCR_TIMING_MASK);

	tims.thiz = FMC2_THIZ;
	thiz = (tims.thiz + 1U) * hclkp;

	/*
	 * tWAIT > tRP
	 * tWAIT > tWP
	 * tWAIT > tREA + tIO
	 */
	twait = MAX(hclkp, NAND_TRP_MIN);
	twait = MAX(twait, NAND_TWP_MIN);
	twait = MAX(twait, NAND_TREA_MAX + FMC2_TIO);
	timing = div_round_up(twait, hclkp);
	tims.twait = CLAMP(timing, 1UL,
			   (unsigned long)FMC2_PMEM_PATT_TIMING_MASK);

	/*
	 * tSETUP_MEM > tCS - tWAIT
	 * tSETUP_MEM > tALS - tWAIT
	 * tSETUP_MEM > tDS - (tWAIT - tHIZ)
	 */
	tset_mem = hclkp;
	if ((twait < NAND_TCS_MIN) && (tset_mem < (NAND_TCS_MIN - twait))) {
		tset_mem = NAND_TCS_MIN - twait;
	}
	if ((twait > thiz) && ((twait - thiz) < NAND_TDS_MIN) &&
	    (tset_mem < (NAND_TDS_MIN - (twait - thiz)))) {
		tset_mem = NAND_TDS_MIN - (twait - thiz);
	}
	timing = div_round_up(tset_mem, hclkp);
	tims.tset_mem = CLAMP(timing, 1UL,
			      (unsigned long)FMC2_PMEM_PATT_TIMING_MASK);

	/*
	 * tHOLD_MEM > tCH
	 * tHOLD_MEM > tREH - tSETUP_MEM
	 * tHOLD_MEM > max(tRC, tWC) - (tSETUP_MEM + tWAIT)
	 */
	thold_mem = MAX(hclkp, NAND_TCH_MIN);
	if ((tset_mem < NAND_TREH_MIN) &&
	    (thold_mem < (NAND_TREH_MIN - tset_mem))) {
		thold_mem = NAND_TREH_MIN - tset_mem;
	}
	if (((tset_mem + twait) < NAND_TRC_MIN) &&
	    (thold_mem < (NAND_TRC_MIN - (tset_mem + twait)))) {
		thold_mem = NAND_TRC_MIN  - (tset_mem + twait);
	}
	if (((tset_mem + twait) < NAND_TWC_MIN) &&
	    (thold_mem < (NAND_TWC_MIN - (tset_mem + twait)))) {
		thold_mem = NAND_TWC_MIN - (tset_mem + twait);
	}
	timing = div_round_up(thold_mem, hclkp);
	tims.thold_mem = CLAMP(timing, 1UL,
			       (unsigned long)FMC2_PMEM_PATT_TIMING_MASK);

	/*
	 * tSETUP_ATT > tCS - tWAIT
	 * tSETUP_ATT > tCLS - tWAIT
	 * tSETUP_ATT > tALS - tWAIT
	 * tSETUP_ATT > tRHW - tHOLD_MEM
	 * tSETUP_ATT > tDS - (tWAIT - tHIZ)
	 */
	tset_att = hclkp;
	if ((twait < NAND_TCS_MIN) && (tset_att < (NAND_TCS_MIN - twait))) {
		tset_att = NAND_TCS_MIN - twait;
	}
	if ((thold_mem < NAND_TRHW_MIN) &&
	    (tset_att < (NAND_TRHW_MIN - thold_mem))) {
		tset_att = NAND_TRHW_MIN - thold_mem;
	}
	if ((twait > thiz) && ((twait - thiz) < NAND_TDS_MIN) &&
	    (tset_att < (NAND_TDS_MIN - (twait - thiz)))) {
		tset_att = NAND_TDS_MIN - (twait - thiz);
	}
	timing = div_round_up(tset_att, hclkp);
	tims.tset_att = CLAMP(timing, 1UL,
			      (unsigned long)FMC2_PMEM_PATT_TIMING_MASK);

	/*
	 * tHOLD_ATT > tALH
	 * tHOLD_ATT > tCH
	 * tHOLD_ATT > tCLH
	 * tHOLD_ATT > tCOH
	 * tHOLD_ATT > tDH
	 * tHOLD_ATT > tWB + tIO + tSYNC - tSETUP_MEM
	 * tHOLD_ATT > tADL - tSETUP_MEM
	 * tHOLD_ATT > tWH - tSETUP_MEM
	 * tHOLD_ATT > tWHR - tSETUP_MEM
	 * tHOLD_ATT > tRC - (tSETUP_ATT + tWAIT)
	 * tHOLD_ATT > tWC - (tSETUP_ATT + tWAIT)
	 */
	thold_att = MAX(hclkp, NAND_TALH_MIN);
	thold_att = MAX(thold_att, NAND_TCH_MIN);
	thold_att = MAX(thold_att, NAND_TCLH_MIN);
	thold_att = MAX(thold_att, NAND_TCOH_MIN);
	thold_att = MAX(thold_att, NAND_TDH_MIN);
	if (((NAND_TWB_MAX + FMC2_TIO + FMC2_TSYNC) > tset_mem) &&
	    (thold_att < (NAND_TWB_MAX + FMC2_TIO + FMC2_TSYNC - tset_mem))) {
		thold_att = NAND_TWB_MAX + FMC2_TIO + FMC2_TSYNC - tset_mem;
	}
	if ((tset_mem < NAND_TADL_MIN) &&
	    (thold_att < (NAND_TADL_MIN - tset_mem))) {
		thold_att = NAND_TADL_MIN - tset_mem;
	}
	if ((tset_mem < NAND_TWH_MIN) &&
	    (thold_att < (NAND_TWH_MIN - tset_mem))) {
		thold_att = NAND_TWH_MIN - tset_mem;
	}
	if ((tset_mem < NAND_TWHR_MIN) &&
	    (thold_att < (NAND_TWHR_MIN - tset_mem))) {
		thold_att = NAND_TWHR_MIN - tset_mem;
	}
	if (((tset_att + twait) < NAND_TRC_MIN) &&
	    (thold_att < (NAND_TRC_MIN - (tset_att + twait)))) {
		thold_att = NAND_TRC_MIN - (tset_att + twait);
	}
	if (((tset_att + twait) < NAND_TWC_MIN) &&
	    (thold_att < (NAND_TWC_MIN - (tset_att + twait)))) {
		thold_att = NAND_TWC_MIN - (tset_att + twait);
	}
	timing = div_round_up(thold_att, hclkp);
	tims.thold_att = CLAMP(timing, 1UL,
			       (unsigned long)FMC2_PMEM_PATT_TIMING_MASK);

	VERBOSE("NAND timings: %u - %u - %u - %u - %u - %u - %u - %u\n",
		tims.tclr, tims.tar, tims.thiz, tims.twait,
		tims.thold_mem, tims.tset_mem,
		tims.thold_att, tims.tset_att);

	/* Set tclr/tar timings */
	pcr = mmio_read_32(fmc2_base() + FMC2_PCR);
	pcr &= ~FMC2_PCR_TCLR_MASK;
	pcr |= FMC2_PCR_TCLR(tims.tclr);
	pcr &= ~FMC2_PCR_TAR_MASK;
	pcr |= FMC2_PCR_TAR(tims.tar);

	/* Set tset/twait/thold/thiz timings in common bank */
	pmem = FMC2_PMEM_MEMSET(tims.tset_mem);
	pmem |= FMC2_PMEM_MEMWAIT(tims.twait);
	pmem |=	FMC2_PMEM_MEMHOLD(tims.thold_mem);
	pmem |= FMC2_PMEM_MEMHIZ(tims.thiz);

	/* Set tset/twait/thold/thiz timings in attribute bank */
	patt = FMC2_PATT_ATTSET(tims.tset_att);
	patt |= FMC2_PATT_ATTWAIT(tims.twait);
	patt |= FMC2_PATT_ATTHOLD(tims.thold_att);
	patt |= FMC2_PATT_ATTHIZ(tims.thiz);

	mmio_write_32(fmc2_base() + FMC2_PCR, pcr);
	mmio_write_32(fmc2_base() + FMC2_PMEM, pmem);
	mmio_write_32(fmc2_base() + FMC2_PATT, patt);
}

static void stm32_fmc2_set_buswidth_16(bool set)
{
	mmio_clrsetbits_32(fmc2_base() + FMC2_PCR, FMC2_PCR_PWID_MASK,
			   (set ? FMC2_PCR_PWID(FMC2_PCR_PWID_16) : 0U));
}

static void stm32_fmc2_set_ecc(bool enable)
{
	mmio_clrsetbits_32(fmc2_base() + FMC2_PCR, FMC2_PCR_ECCEN,
			   (enable ? FMC2_PCR_ECCEN : 0U));
}

static int stm32_fmc2_ham_correct(uint8_t *buffer, uint8_t *eccbuffer,
				  uint8_t *ecc)
{
	uint8_t xor_ecc_ones;
	uint16_t xor_ecc_1b, xor_ecc_2b, xor_ecc_3b;
	union {
		uint32_t val;
		uint8_t  bytes[4];
	} xor_ecc;

	/* Page size--------ECC_Code Size
	 * 256---------------22 bits LSB  (ECC_CODE & 0x003FFFFF)
	 * 512---------------24 bits      (ECC_CODE & 0x00FFFFFF)
	 * 1024--------------26 bits      (ECC_CODE & 0x03FFFFFF)
	 * 2048--------------28 bits      (ECC_CODE & 0x0FFFFFFF)
	 * 4096--------------30 bits      (ECC_CODE & 0x3FFFFFFF)
	 * 8192--------------32 bits      (ECC_CODE & 0xFFFFFFFF)
	 */

	/* For Page size 512, ECC_Code size 24 bits */
	xor_ecc_1b = ecc[0] ^ eccbuffer[0];
	xor_ecc_2b = ecc[1] ^ eccbuffer[1];
	xor_ecc_3b = ecc[2] ^ eccbuffer[2];

	xor_ecc.val = 0U;
	xor_ecc.bytes[2] = xor_ecc_3b;
	xor_ecc.bytes[1] = xor_ecc_2b;
	xor_ecc.bytes[0] = xor_ecc_1b;

	if (xor_ecc.val == 0U) {
		return 0; /* No Error */
	}

	xor_ecc_ones = __builtin_popcount(xor_ecc.val);
	if (xor_ecc_ones < 23U) {
		if (xor_ecc_ones == 12U) {
			uint16_t bit_address, byte_address;

			/* Correctable ERROR */
			bit_address = ((xor_ecc_1b >> 1) & BIT(0)) |
				      ((xor_ecc_1b >> 2) & BIT(1)) |
				      ((xor_ecc_1b >> 3) & BIT(2));

			byte_address = ((xor_ecc_1b >> 7) & BIT(0)) |
				       ((xor_ecc_2b) & BIT(1)) |
				       ((xor_ecc_2b >> 1) & BIT(2)) |
				       ((xor_ecc_2b >> 2) & BIT(3)) |
				       ((xor_ecc_2b >> 3) & BIT(4)) |
				       ((xor_ecc_3b << 4) & BIT(5)) |
				       ((xor_ecc_3b << 3) & BIT(6)) |
				       ((xor_ecc_3b << 2) & BIT(7)) |
				       ((xor_ecc_3b << 1) & BIT(8));

			/* Correct bit error in the data */
			buffer[byte_address] =
				buffer[byte_address] ^ BIT(bit_address);
			VERBOSE("Hamming: 1 ECC error corrected\n");

			return 0;
		}

		/* Non Correctable ERROR */
		ERROR("%s: Uncorrectable ECC Errors\n", __func__);
		return -1;
	}

	/* ECC ERROR */
	ERROR("%s: Hamming correction error\n", __func__);
	return -1;
}


static int stm32_fmc2_ham_calculate(uint8_t *buffer, uint8_t *ecc)
{
	uint32_t heccr;
	uint64_t timeout = timeout_init_us(TIMEOUT_US_10_MS);

	while ((mmio_read_32(fmc2_base() + FMC2_SR) & FMC2_SR_NWRF) == 0U) {
		if (timeout_elapsed(timeout)) {
			return -ETIMEDOUT;
		}
	}

	heccr = mmio_read_32(fmc2_base() + FMC2_HECCR);

	ecc[0] = heccr;
	ecc[1] = heccr >> 8;
	ecc[2] = heccr >> 16;

	/* Disable ECC */
	stm32_fmc2_set_ecc(false);

	return 0;
}

static int stm32_fmc2_bch_correct(uint8_t *buffer, unsigned int eccsize)
{
	uint32_t bchdsr0, bchdsr1, bchdsr2, bchdsr3, bchdsr4;
	uint16_t pos[8];
	int i, den;
	uint64_t timeout = timeout_init_us(TIMEOUT_US_10_MS);

	while ((mmio_read_32(fmc2_base() + FMC2_BCHISR) &
		FMC2_BCHISR_DERF) == 0U) {
		if (timeout_elapsed(timeout)) {
			return -ETIMEDOUT;
		}
	}

	bchdsr0 = mmio_read_32(fmc2_base() + FMC2_BCHDSR0);
	bchdsr1 = mmio_read_32(fmc2_base() + FMC2_BCHDSR1);
	bchdsr2 = mmio_read_32(fmc2_base() + FMC2_BCHDSR2);
	bchdsr3 = mmio_read_32(fmc2_base() + FMC2_BCHDSR3);
	bchdsr4 = mmio_read_32(fmc2_base() + FMC2_BCHDSR4);

	/* Disable ECC */
	stm32_fmc2_set_ecc(false);

	/* No error found */
	if ((bchdsr0 & FMC2_BCHDSR0_DEF) == 0U) {
		return 0;
	}

	/* Too many errors detected */
	if ((bchdsr0 & FMC2_BCHDSR0_DUE) != 0U) {
		return -EBADMSG;
	}

	pos[0] = bchdsr1 & FMC2_BCHDSR1_EBP1_MASK;
	pos[1] = (bchdsr1 & FMC2_BCHDSR1_EBP2_MASK) >> FMC2_BCHDSR1_EBP2_SHIFT;
	pos[2] = bchdsr2 & FMC2_BCHDSR2_EBP3_MASK;
	pos[3] = (bchdsr2 & FMC2_BCHDSR2_EBP4_MASK) >> FMC2_BCHDSR2_EBP4_SHIFT;
	pos[4] = bchdsr3 & FMC2_BCHDSR3_EBP5_MASK;
	pos[5] = (bchdsr3 & FMC2_BCHDSR3_EBP6_MASK) >> FMC2_BCHDSR3_EBP6_SHIFT;
	pos[6] = bchdsr4 & FMC2_BCHDSR4_EBP7_MASK;
	pos[7] = (bchdsr4 & FMC2_BCHDSR4_EBP8_MASK) >> FMC2_BCHDSR4_EBP8_SHIFT;

	den = (bchdsr0 & FMC2_BCHDSR0_DEN_MASK) >> FMC2_BCHDSR0_DEN_SHIFT;
	for (i = 0; i < den; i++) {
		if (pos[i] < (eccsize * 8U)) {
			uint8_t bitmask = BIT(pos[i] % 8U);
			uint32_t offset = pos[i] / 8U;

			*(buffer + offset) ^= bitmask;
		}
	}

	return 0;
}

static void stm32_fmc2_hwctl(struct nand_device *nand)
{
	stm32_fmc2_set_ecc(false);

	if (nand->ecc.max_bit_corr != FMC2_ECC_HAM) {
		mmio_clrbits_32(fmc2_base() + FMC2_PCR, FMC2_PCR_WEN);
		mmio_write_32(fmc2_base() + FMC2_BCHICR, FMC2_BCHICR_CLEAR_IRQ);
	}

	stm32_fmc2_set_ecc(true);
}

static int stm32_fmc2_read_page(struct nand_device *nand,
				unsigned int page, uintptr_t buffer)
{
	unsigned int eccsize = nand->ecc.size;
	unsigned int eccbytes = nand->ecc.bytes;
	unsigned int eccsteps = nand->page_size / eccsize;
	uint8_t ecc_corr[FMC2_MAX_ECC_BYTES];
	uint8_t ecc_cal[FMC2_MAX_ECC_BYTES] = {0U};
	uint8_t *p;
	unsigned int i;
	unsigned int s;
	int ret;

	VERBOSE(">%s page %i buffer %lx\n", __func__, page, buffer);

	ret = nand_read_page_cmd(page, 0U, 0U, 0U);
	if (ret != 0) {
		return ret;
	}

	for (s = 0U, i = nand->page_size + FMC2_BBM_LEN, p = (uint8_t *)buffer;
	     s < eccsteps;
	     s++, i += eccbytes, p += eccsize) {
		stm32_fmc2_hwctl(nand);

		/* Read the NAND page sector (512 bytes) */
		ret = nand_change_read_column_cmd(s * eccsize, (uintptr_t)p,
						  eccsize);
		if (ret != 0) {
			return ret;
		}

		if (nand->ecc.max_bit_corr == FMC2_ECC_HAM) {
			ret = stm32_fmc2_ham_calculate(p, ecc_cal);
			if (ret != 0) {
				return ret;
			}
		}

		/* Read the corresponding ECC bytes */
		ret = nand_change_read_column_cmd(i, (uintptr_t)ecc_corr,
						  eccbytes);
		if (ret != 0) {
			return ret;
		}

		/* Correct the data */
		if (nand->ecc.max_bit_corr == FMC2_ECC_HAM) {
			ret = stm32_fmc2_ham_correct(p, ecc_corr, ecc_cal);
		} else {
			ret = stm32_fmc2_bch_correct(p, eccsize);
		}

		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

static void stm32_fmc2_read_data(struct nand_device *nand,
				 uint8_t *buff, unsigned int length,
				 bool use_bus8)
{
	uintptr_t data_base = stm32_fmc2.cs[stm32_fmc2.cs_sel].data_base;

	if (use_bus8 && (nand->buswidth == NAND_BUS_WIDTH_16)) {
		stm32_fmc2_set_buswidth_16(false);
	}

	if ((((uintptr_t)buff & BIT(0)) != 0U) && (length != 0U)) {
		*buff = mmio_read_8(data_base);
		buff += sizeof(uint8_t);
		length -= sizeof(uint8_t);
	}

	if ((((uintptr_t)buff & GENMASK_32(1, 0)) != 0U) &&
	    (length >= sizeof(uint16_t))) {
		*(uint16_t *)buff = mmio_read_16(data_base);
		buff += sizeof(uint16_t);
		length -= sizeof(uint16_t);
	}

	/* 32bit aligned */
	while (length >= sizeof(uint32_t)) {
		*(uint32_t *)buff = mmio_read_32(data_base);
		buff += sizeof(uint32_t);
		length -= sizeof(uint32_t);
	}

	/* Read remaining bytes */
	if (length >= sizeof(uint16_t)) {
		*(uint16_t *)buff = mmio_read_16(data_base);
		buff += sizeof(uint16_t);
		length -= sizeof(uint16_t);
	}

	if (length != 0U) {
		*buff = mmio_read_8(data_base);
	}

	if (use_bus8 && (nand->buswidth == NAND_BUS_WIDTH_16)) {
		/* Reconfigure bus width to 16-bit */
		stm32_fmc2_set_buswidth_16(true);
	}
}

static void stm32_fmc2_write_data(struct nand_device *nand,
				  uint8_t *buff, unsigned int length,
				  bool use_bus8)
{
	uintptr_t data_base = stm32_fmc2.cs[stm32_fmc2.cs_sel].data_base;

	if (use_bus8 && (nand->buswidth == NAND_BUS_WIDTH_16)) {
		/* Reconfigure bus width to 8-bit */
		stm32_fmc2_set_buswidth_16(false);
	}

	if ((((uintptr_t)buff & BIT(0)) != 0U) && (length != 0U)) {
		mmio_write_8(data_base, *buff);
		buff += sizeof(uint8_t);
		length -= sizeof(uint8_t);
	}

	if ((((uintptr_t)buff & GENMASK_32(1, 0)) != 0U) &&
	    (length >= sizeof(uint16_t))) {
		mmio_write_16(data_base, *(uint16_t *)buff);
		buff += sizeof(uint16_t);
		length -= sizeof(uint16_t);
	}

	/* 32bits aligned */
	while (length >= sizeof(uint32_t)) {
		mmio_write_32(data_base, *(uint32_t *)buff);
		buff += sizeof(uint32_t);
		length -= sizeof(uint32_t);
	}

	/* Read remaining bytes */
	if (length >= sizeof(uint16_t)) {
		mmio_write_16(data_base, *(uint16_t *)buff);
		buff += sizeof(uint16_t);
		length -= sizeof(uint16_t);
	}

	if (length != 0U) {
		mmio_write_8(data_base, *buff);
	}

	if (use_bus8 && (nand->buswidth == NAND_BUS_WIDTH_16)) {
		/* Reconfigure bus width to 16-bit */
		stm32_fmc2_set_buswidth_16(true);
	}
}

static void stm32_fmc2_ctrl_init(void)
{
	uint32_t pcr = mmio_read_32(fmc2_base() + FMC2_PCR);
	uint32_t bcr1 = mmio_read_32(fmc2_base() + FMC2_BCR1);

	/* Enable wait feature and NAND flash memory bank */
	pcr |= FMC2_PCR_PWAITEN;
	pcr |= FMC2_PCR_PBKEN;

	/* Set buswidth to 8 bits mode for identification */
	pcr &= ~FMC2_PCR_PWID_MASK;

	/* ECC logic is disabled */
	pcr &= ~FMC2_PCR_ECCEN;

	/* Default mode */
	pcr &= ~FMC2_PCR_ECCALG;
	pcr &= ~FMC2_PCR_BCHECC;
	pcr &= ~FMC2_PCR_WEN;

	/* Set default ECC sector size */
	pcr &= ~FMC2_PCR_ECCSS_MASK;
	pcr |= FMC2_PCR_ECCSS(FMC2_PCR_ECCSS_2048);

	/* Set default TCLR/TAR timings */
	pcr &= ~FMC2_PCR_TCLR_MASK;
	pcr |= FMC2_PCR_TCLR(FMC2_PCR_TCLR_DEFAULT);
	pcr &= ~FMC2_PCR_TAR_MASK;
	pcr |= FMC2_PCR_TAR(FMC2_PCR_TAR_DEFAULT);

	/* Enable FMC2 controller */
	bcr1 |= FMC2_BCR1_FMC2EN;

	mmio_write_32(fmc2_base() + FMC2_BCR1, bcr1);
	mmio_write_32(fmc2_base() + FMC2_PCR, pcr);
	mmio_write_32(fmc2_base() + FMC2_PMEM, FMC2_PMEM_DEFAULT);
	mmio_write_32(fmc2_base() + FMC2_PATT, FMC2_PATT_DEFAULT);
}

static int stm32_fmc2_exec(struct nand_req *req)
{
	int ret = 0;

	switch (req->type & NAND_REQ_MASK) {
	case NAND_REQ_CMD:
		VERBOSE("Write CMD %x\n", (uint8_t)req->type);
		mmio_write_8(stm32_fmc2.cs[stm32_fmc2.cs_sel].cmd_base,
			     (uint8_t)req->type);
		break;
	case NAND_REQ_ADDR:
		VERBOSE("Write ADDR %x\n", *(req->addr));
		mmio_write_8(stm32_fmc2.cs[stm32_fmc2.cs_sel].addr_base,
			     *(req->addr));
		break;
	case NAND_REQ_DATAIN:
		VERBOSE("Read data\n");
		stm32_fmc2_read_data(req->nand, req->addr, req->length,
				     ((req->type & NAND_REQ_BUS_WIDTH_8) !=
				      0U));
		break;
	case NAND_REQ_DATAOUT:
		VERBOSE("Write data\n");
		stm32_fmc2_write_data(req->nand, req->addr, req->length,
				      ((req->type & NAND_REQ_BUS_WIDTH_8) !=
				      0U));
		break;
	case NAND_REQ_WAIT:
		VERBOSE("WAIT Ready\n");
		ret = nand_wait_ready(req->delay_ms);
		break;
	default:
		ret = -EINVAL;
		break;
	};

	return ret;
}

static void stm32_fmc2_setup(struct nand_device *nand)
{
	uint32_t pcr = mmio_read_32(fmc2_base() + FMC2_PCR);

	/* Set buswidth */
	pcr &= ~FMC2_PCR_PWID_MASK;
	if (nand->buswidth == NAND_BUS_WIDTH_16) {
		pcr |= FMC2_PCR_PWID(FMC2_PCR_PWID_16);
	}

	if (nand->ecc.mode == NAND_ECC_HW) {
		nand->mtd_read_page = stm32_fmc2_read_page;

		pcr &= ~FMC2_PCR_ECCALG;
		pcr &= ~FMC2_PCR_BCHECC;

		pcr &= ~FMC2_PCR_ECCSS_MASK;
		pcr |= FMC2_PCR_ECCSS(FMC2_PCR_ECCSS_512);

		switch (nand->ecc.max_bit_corr) {
		case FMC2_ECC_HAM:
			nand->ecc.bytes = 3;
			break;
		case FMC2_ECC_BCH8:
			pcr |= FMC2_PCR_ECCALG;
			pcr |= FMC2_PCR_BCHECC;
			nand->ecc.bytes = 13;
			break;
		default:
			/* Use FMC2 ECC BCH4 */
			pcr |= FMC2_PCR_ECCALG;
			nand->ecc.bytes = 7;
			break;
		}

		if ((nand->buswidth & NAND_BUS_WIDTH_16) != 0) {
			nand->ecc.bytes++;
		}
	}

	mmio_write_32(stm32_fmc2.reg_base + FMC2_PCR, pcr);
}

static const struct nand_ctrl_ops ctrl_ops = {
	.setup = stm32_fmc2_setup,
	.exec = stm32_fmc2_exec
};

int stm32_fmc2_init(void)
{
	int fmc_ebi_node;
	int fmc_nfc_node;
	int fmc_flash_node = 0;
	int nchips = 0;
	unsigned int i;
	void *fdt = NULL;
	const fdt32_t *cuint;
	struct dt_node_info info;
	uintptr_t bank_address[MAX_BANK] = { 0, 0, 0, 0, 0 };
	uint8_t bank_assigned = 0;
	uint8_t bank;
	int ret;

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	fmc_ebi_node = dt_get_node(&info, -1, DT_FMC2_EBI_COMPAT);
	if (fmc_ebi_node < 0) {
		return fmc_ebi_node;
	}

	if (info.status == DT_DISABLED) {
		return -FDT_ERR_NOTFOUND;
	}

	stm32_fmc2.reg_base = info.base;

	if ((info.clock < 0) || (info.reset < 0)) {
		return -FDT_ERR_BADVALUE;
	}

	stm32_fmc2.clock_id = (unsigned long)info.clock;
	stm32_fmc2.reset_id = (unsigned int)info.reset;

	cuint = fdt_getprop(fdt, fmc_ebi_node, "ranges", NULL);
	if (cuint == NULL) {
		return -FDT_ERR_BADVALUE;
	}

	for (i = 0U; i < MAX_BANK; i++) {
		bank = fdt32_to_cpu(*cuint);
		if ((bank >= MAX_BANK) || ((bank_assigned & BIT(bank)) != 0U)) {
			return -FDT_ERR_BADVALUE;
		}
		bank_assigned |= BIT(bank);
		bank_address[bank] = fdt32_to_cpu(*(cuint + 2));
		cuint += 4;
	}

	/* Pinctrl initialization */
	if (dt_set_pinctrl_config(fmc_ebi_node) != 0) {
		return -FDT_ERR_BADVALUE;
	}

	/* Parse NFC controller node */
	fmc_nfc_node = fdt_node_offset_by_compatible(fdt, fmc_ebi_node,
						     DT_FMC2_NFC_COMPAT);
	if (fmc_nfc_node < 0) {
		return fmc_nfc_node;
	}

	if (fdt_get_status(fmc_nfc_node) == DT_DISABLED) {
		return -FDT_ERR_NOTFOUND;
	}

	cuint = fdt_getprop(fdt, fmc_nfc_node, "reg", NULL);
	if (cuint == NULL) {
		return -FDT_ERR_BADVALUE;
	}

	for (i = 0U; i < MAX_CS; i++) {
		bank = fdt32_to_cpu(*cuint);
		if (bank >= MAX_BANK) {
			return -FDT_ERR_BADVALUE;
		}
		stm32_fmc2.cs[i].data_base = fdt32_to_cpu(*(cuint + 1)) +
					     bank_address[bank];

		bank = fdt32_to_cpu(*(cuint + 3));
		if (bank >= MAX_BANK) {
			return -FDT_ERR_BADVALUE;
		}
		stm32_fmc2.cs[i].cmd_base = fdt32_to_cpu(*(cuint + 4)) +
					    bank_address[bank];

		bank = fdt32_to_cpu(*(cuint + 6));
		if (bank >= MAX_BANK) {
			return -FDT_ERR_BADVALUE;
		}
		stm32_fmc2.cs[i].addr_base = fdt32_to_cpu(*(cuint + 7)) +
					     bank_address[bank];

		cuint += 9;
	}

	/* Parse flash nodes */
	fdt_for_each_subnode(fmc_flash_node, fdt, fmc_nfc_node) {
		nchips++;
	}

	if (nchips != 1) {
		WARN("Only one SLC NAND device supported\n");
		return -FDT_ERR_BADVALUE;
	}

	fdt_for_each_subnode(fmc_flash_node, fdt, fmc_nfc_node) {
		/* Get chip select */
		cuint = fdt_getprop(fdt, fmc_flash_node, "reg", NULL);
		if (cuint == NULL) {
			WARN("Chip select not well defined\n");
			return -FDT_ERR_BADVALUE;
		}

		stm32_fmc2.cs_sel = fdt32_to_cpu(*cuint);
		if (stm32_fmc2.cs_sel >= MAX_CS) {
			return -FDT_ERR_BADVALUE;
		}

		VERBOSE("NAND CS %i\n", stm32_fmc2.cs_sel);
	}

	/* Enable Clock */
	stm32mp_clk_enable(stm32_fmc2.clock_id);

	/* Reset IP */
	ret = stm32mp_reset_assert(stm32_fmc2.reset_id, TIMEOUT_US_1_MS);
	if (ret != 0) {
		panic();
	}
	ret = stm32mp_reset_deassert(stm32_fmc2.reset_id, TIMEOUT_US_1_MS);
	if (ret != 0) {
		panic();
	}

	/* Setup default IP registers */
	stm32_fmc2_ctrl_init();

	/* Setup default timings */
	stm32_fmc2_nand_setup_timing();

	/* Init NAND RAW framework */
	nand_raw_ctrl_init(&ctrl_ops);

	return 0;
}
