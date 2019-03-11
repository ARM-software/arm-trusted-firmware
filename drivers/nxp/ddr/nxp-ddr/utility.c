/*
 * Copyright 2016-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <platform_def.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <common/debug.h>
#include <errno.h>
#include <io.h>
#include "ddr.h"
#include "immap.h"

#define UL_5POW12	244140625UL
#define ULL_2E12	2000000000000ULL
#define UL_2POW13	(1UL << 13)
#define ULL_8FS		0xFFFFFFFFULL

#define do_div(n, base) ({			\
	unsigned int __base = (base);		\
	unsigned int __rem;				\
	__rem = ((unsigned long long)(n)) % __base;	\
	(n) = ((unsigned long long)(n)) / __base;	\
	__rem;					\
})

#define CCN_HN_F_SAM_NODEID_MASK	0x7f
#ifdef NXP_HAS_CCN504
#define CCN_HN_F_SAM_NODEID_DDR0	0x4
#define CCN_HN_F_SAM_NODEID_DDR1	0xe
#elif defined(NXP_HAS_CCN508)
#define CCN_HN_F_SAM_NODEID_DDR0	0x8
#define CCN_HN_F_SAM_NODEID_DDR1	0x18
#endif

unsigned long get_ddr_freq(struct sysinfo *sys, int ctrl_num)
{
	if (sys->freq_ddr_pll0 == 0)
		get_clocks(sys);

	switch (ctrl_num) {
	case 0:
		return sys->freq_ddr_pll0;
	case 1:
		return sys->freq_ddr_pll0;
	case 2:
		return sys->freq_ddr_pll1;
	}

	return 0;
}

unsigned int get_memory_clk_ps(const unsigned long data_rate)
{
	unsigned int result;
	/* Round to nearest 10ps, being careful about 64-bit multiply/divide */
	unsigned long long rem, mclk_ps = ULL_2E12;

	/* Now perform the big divide, the result fits in 32-bits */
	rem = do_div(mclk_ps, data_rate);
	result = (rem >= (data_rate >> 1)) ? mclk_ps + 1 : mclk_ps;

	return result;
}

unsigned int picos_to_mclk(unsigned long data_rate, unsigned int picos)
{
	unsigned long long clks, clks_rem;

	/* Short circuit for zero picos */
	if (!picos || !data_rate)
		return 0;

	/* First multiply the time by the data rate (32x32 => 64) */
	clks = picos * (unsigned long long)data_rate;
	/*
	 * Now divide by 5^12 and track the 32-bit remainder, then divide
	 * by 2*(2^12) using shifts (and updating the remainder).
	 */
	clks_rem = do_div(clks, UL_5POW12);
	clks_rem += (clks & (UL_2POW13-1)) * UL_5POW12;
	clks >>= 13;

	/* If we had a remainder greater than the 1ps error, then round up */
	if (clks_rem > data_rate)
		clks++;

	/* Clamp to the maximum representable value */
	if (clks > ULL_8FS)
		clks = ULL_8FS;
	return (unsigned int) clks;
}

/* valid_spd_mask has been checked by parse_spd */
int disable_unused_ddrc(struct ddr_info *priv,
			int valid_spd_mask)
{
#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
	void *hnf_sam_ctrl = (void *)(NXP_CCN_HN_F_0_ADDR + CCN_HN_F_SAM_CTL);
	uint32_t val, nodeid;
	int disable_ddrc = 0;
	int i;

	if (priv->num_ctlrs < 2)
		debug("%s: nothing to do.\n", __func__);

	switch (priv->dimm_on_ctlr) {
	case 1:
		disable_ddrc = !(valid_spd_mask & 0x1) ? 1 :
				!(valid_spd_mask & 0x2) ? 2 : 0;
		break;
	case 2:
		disable_ddrc = !(valid_spd_mask & 0x1) ? 1 :
				!(valid_spd_mask & 0x4) ? 2 : 0;
		break;
	default:
		ERROR("Invalid number of DIMMs %d\n", priv->dimm_on_ctlr);
		return -EINVAL;
	}

	if (disable_ddrc)
		debug("valid_spd_mask = 0x%x\n", valid_spd_mask);

	switch (disable_ddrc) {
	case 1:
		priv->num_ctlrs = 1;
		priv->spd_addr = &priv->spd_addr[priv->dimm_on_ctlr];
		priv->ddr[0] = priv->ddr[1];
		priv->ddr[1] = NULL;
		priv->phy[0] = priv->phy[0];
		priv->phy[1] = NULL;
		debug("Disable first DDR controller\n");
		break;
	case 2:
		priv->num_ctlrs = 1;
		priv->ddr[1] = NULL;
		priv->phy[1] = NULL;
		debug("Disable second DDR controller\n");
		/* fall through */
	case 0:
		break;
	default:
		ERROR("Program error.\n");
		return -EINVAL;
	}

	if (!disable_ddrc) {
		debug("Both controllers in use.\n");
		return 0;
	}

	for (i = 0; i < 8; i++) {
		val = in_le64(hnf_sam_ctrl);
		nodeid = disable_ddrc == 1 ? CCN_HN_F_SAM_NODEID_DDR1 :
			 (disable_ddrc == 2 ? CCN_HN_F_SAM_NODEID_DDR0 :
			  (i < 4 ? CCN_HN_F_SAM_NODEID_DDR0
				 : CCN_HN_F_SAM_NODEID_DDR1));
		if (nodeid != (val & CCN_HN_F_SAM_NODEID_MASK)) {
			debug("Setting HN-F node %d\n", i);
			debug("nodeid = 0x%x\n", nodeid);
			val &= ~CCN_HN_F_SAM_NODEID_MASK;
			val |= nodeid;
			out_le64(hnf_sam_ctrl, val);
		}
		hnf_sam_ctrl += (NXP_CCN_HN_F_1_ADDR - NXP_CCN_HN_F_0_ADDR);
	}
#endif
	return 0;
}

unsigned int get_ddrc_version(const struct ccsr_ddr *ddr)
{
	unsigned int ver;

	ver = (ddr_in32(&ddr->ip_rev1) & 0xFFFF) << 8;
	ver |= (ddr_in32(&ddr->ip_rev2) & 0xFF00) >> 8;

	return ver;
}

void print_ddr_info(struct ccsr_ddr *ddr)
{
	unsigned int cs0_config = ddr_in32(&ddr->csn_cfg[0]);
	unsigned int sdram_cfg = ddr_in32(&ddr->sdram_cfg);
	int cas_lat;

	if (!(sdram_cfg & SDRAM_CFG_MEM_EN)) {
		printf(" (DDR not enabled)\n");
		return;
	}

	printf("DDR");
	switch ((sdram_cfg & SDRAM_CFG_SDRAM_TYPE_MASK) >>
		SDRAM_CFG_SDRAM_TYPE_SHIFT) {
	case SDRAM_TYPE_DDR4:
		printf("4");
		break;
	default:
		printf("?");
		break;
	}

	switch (sdram_cfg & SDRAM_CFG_DBW_MASK) {
	case SDRAM_CFG_32_BW:
		printf(", 32-bit");
		break;
	case SDRAM_CFG_16_BW:
		printf(", 16-bit");
		break;
	case SDRAM_CFG_8_BW:
		printf(", 8-bit");
		break;
	default:
		printf(", 64-bit");
		break;
	}

	/* Calculate CAS latency based on timing cfg values */
	cas_lat = ((ddr_in32(&ddr->timing_cfg_1) >> 16) & 0xf);
	cas_lat += 2;	/* for DDRC newer than 4.4 */
	cas_lat += ((ddr_in32(&ddr->timing_cfg_3) >> 12) & 3) << 4;
	printf(", CL=%d", cas_lat >> 1);
	if (cas_lat & 0x1)
		printf(".5");

	if (sdram_cfg & SDRAM_CFG_ECC_EN)
		printf(", ECC on");
	else
		printf(", ECC off");

	if (cs0_config & 0x20000000) {
		printf(", ");
		switch ((cs0_config >> 24) & 0xf) {
		case DDR_256B_INTLV:
			printf("256B");
			break;
		default:
			printf("invalid");
			break;
		}
	}

	if ((sdram_cfg >> 8) & 0x7f) {
		printf(", ");
		switch (sdram_cfg >> 8 & 0x7f) {
		case DDR_BA_INTLV_CS0123:
			printf("CS0+CS1+CS2+CS3");
			break;
		case DDR_BA_INTLV_CS01:
			printf("CS0+CS1");
			break;
		default:
			printf("invalid");
			break;
		}
	}
	printf("\n");
}
