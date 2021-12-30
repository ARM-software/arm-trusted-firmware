/*
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <common/debug.h>
#include <ddr.h>
#include <immap.h>
#include <lib/mmio.h>

#define UL_5POW12	244140625UL
#define ULL_2E12	2000000000000ULL
#define UL_2POW13	(1UL << 13)
#define ULL_8FS		0xFFFFFFFFULL

#define do_div(n, base) ({				\
	unsigned int __base = (base);			\
	unsigned int __rem;				\
	__rem = ((unsigned long long)(n)) % __base;	\
	(n) = ((unsigned long long)(n)) / __base;	\
	__rem;						\
})

#define CCN_HN_F_SAM_NODEID_MASK	0x7f
#ifdef NXP_HAS_CCN504
#define CCN_HN_F_SAM_NODEID_DDR0	0x4
#define CCN_HN_F_SAM_NODEID_DDR1	0xe
#elif defined(NXP_HAS_CCN508)
#define CCN_HN_F_SAM_NODEID_DDR0_0	0x3
#define CCN_HN_F_SAM_NODEID_DDR0_1	0x8
#define CCN_HN_F_SAM_NODEID_DDR1_0	0x13
#define CCN_HN_F_SAM_NODEID_DDR1_1	0x18
#endif

unsigned long get_ddr_freq(struct sysinfo *sys, int ctrl_num)
{
	if (sys->freq_ddr_pll0 == 0) {
		get_clocks(sys);
	}

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
	if ((picos == 0U) || (data_rate == 0UL)) {
		return 0U;
	}

	/* First multiply the time by the data rate (32x32 => 64) */
	clks = picos * (unsigned long long)data_rate;
	/*
	 * Now divide by 5^12 and track the 32-bit remainder, then divide
	 * by 2*(2^12) using shifts (and updating the remainder).
	 */
	clks_rem = do_div(clks, UL_5POW12);
	clks_rem += (clks & (UL_2POW13-1)) * UL_5POW12;
	clks >>= 13U;

	/* If we had a remainder greater than the 1ps error, then round up */
	if (clks_rem > data_rate) {
		clks++;
	}

	/* Clamp to the maximum representable value */
	if (clks > ULL_8FS) {
		clks = ULL_8FS;
	}
	return (unsigned int) clks;
}

/* valid_spd_mask has been checked by parse_spd */
int disable_unused_ddrc(struct ddr_info *priv,
			int valid_spd_mask, uintptr_t nxp_ccn_hn_f0_addr)
{
#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
	void *hnf_sam_ctrl = (void *)(nxp_ccn_hn_f0_addr + CCN_HN_F_SAM_CTL);
	uint32_t val, nodeid;
#ifdef NXP_HAS_CCN504
	uint32_t num_hnf_nodes = 4U;
#else
	uint32_t num_hnf_nodes = 8U;
#endif
	int disable_ddrc = 0;
	int i;

	if (priv->num_ctlrs < 2) {
		debug("%s: nothing to do.\n", __func__);
	}

	switch (priv->dimm_on_ctlr) {
	case 1:
		disable_ddrc = ((valid_spd_mask &0x2) == 0) ? 2 : 0;
		disable_ddrc = ((valid_spd_mask &0x1) == 0) ? 1 : disable_ddrc;
		break;
	case 2:
		disable_ddrc = ((valid_spd_mask &0x4) == 0) ? 2 : 0;
		disable_ddrc = ((valid_spd_mask &0x1) == 0) ? 1 : disable_ddrc;
		break;
	default:
		ERROR("Invalid number of DIMMs %d\n", priv->dimm_on_ctlr);
		return -EINVAL;
	}

	if (disable_ddrc != 0) {
		debug("valid_spd_mask = 0x%x\n", valid_spd_mask);
	}

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
		/* fallthrough */
	case 0:
		break;
	default:
		ERROR("Program error.\n");
		return -EINVAL;
	}

	if (disable_ddrc == 0) {
		debug("Both controllers in use.\n");
		return 0;
	}

	for (i = 0; i < num_hnf_nodes; i++) {
		val = mmio_read_64((uintptr_t)hnf_sam_ctrl);
#ifdef NXP_HAS_CCN504
		nodeid = disable_ddrc == 1 ? CCN_HN_F_SAM_NODEID_DDR1 :
			(disable_ddrc == 2 ? CCN_HN_F_SAM_NODEID_DDR0 :
			 0x0);   /*Failure condition. never hit */
#elif defined(NXP_HAS_CCN508)
		if (disable_ddrc == 1) {
			nodeid = (i < 2 || i >= 6) ? CCN_HN_F_SAM_NODEID_DDR1_1 :
				CCN_HN_F_SAM_NODEID_DDR1_0;
		} else if (disable_ddrc == 2) {
			nodeid = (i < 2 || i >= 6) ? CCN_HN_F_SAM_NODEID_DDR0_0 :
				CCN_HN_F_SAM_NODEID_DDR0_1;
		} else {
			nodeid = 0; /* Failure condition. never hit */
		}
#endif
		if (nodeid != (val & CCN_HN_F_SAM_NODEID_MASK)) {
			debug("Setting HN-F node %d\n", i);
			debug("nodeid = 0x%x\n", nodeid);
			val &= ~CCN_HN_F_SAM_NODEID_MASK;
			val |= nodeid;
			mmio_write_64((uintptr_t)hnf_sam_ctrl, val);
		}
		hnf_sam_ctrl += CCN_HN_F_REGION_SIZE;
	}
#endif
	return 0;
}

unsigned int get_ddrc_version(const struct ccsr_ddr *ddr)
{
	unsigned int ver;

	ver = (ddr_in32(&ddr->ip_rev1) & 0xFFFF) << 8U;
	ver |= (ddr_in32(&ddr->ip_rev2) & 0xFF00) >> 8U;

	return ver;
}

void print_ddr_info(struct ccsr_ddr *ddr)
{
	unsigned int cs0_config = ddr_in32(&ddr->csn_cfg[0]);
	unsigned int sdram_cfg = ddr_in32(&ddr->sdram_cfg);
	int cas_lat;

	if ((sdram_cfg & SDRAM_CFG_MEM_EN) == 0U) {
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
	if ((cas_lat & 0x1) != 0) {
		printf(".5");
	}

	if ((sdram_cfg & SDRAM_CFG_ECC_EN) != 0) {
		printf(", ECC on");
	} else {
		printf(", ECC off");
	}

	if ((cs0_config & 0x20000000) != 0) {
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

	if (((sdram_cfg >> 8) & 0x7f) != 0) {
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
