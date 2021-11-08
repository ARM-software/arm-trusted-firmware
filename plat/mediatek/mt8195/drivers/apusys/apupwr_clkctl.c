/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>

#include <apupwr_clkctl.h>
#include <apupwr_clkctl_def.h>
#include <mtk_plat_common.h>
#include <platform_def.h>

/* 8195 use PCW mode to change freq directly */
enum pll_set_rate_mode PLL_MODE = CON0_PCW;

char *buck_domain_str[APUSYS_BUCK_DOMAIN_NUM] = {
	"V_VPU0",
	"V_VPU1",
	"V_MDLA0",
	"V_MDLA1",
	"V_APU_CONN",
	"V_TOP_IOMMU",
	"V_VCORE",
};

uint32_t aacc_set[APUSYS_BUCK_DOMAIN_NUM] = {
	APU_ACC_CONFG_SET1, APU_ACC_CONFG_SET2,
	APU_ACC_CONFG_SET4, APU_ACC_CONFG_SET5,
	APU_ACC_CONFG_SET0, APU_ACC_CONFG_SET7
};

uint32_t aacc_clr[APUSYS_BUCK_DOMAIN_NUM] = {
	APU_ACC_CONFG_CLR1, APU_ACC_CONFG_CLR2,
	APU_ACC_CONFG_CLR4, APU_ACC_CONFG_CLR5,
	APU_ACC_CONFG_CLR0, APU_ACC_CONFG_CLR7
};

struct reg_seq {
	uint32_t address;
	uint32_t val;
};

static const struct reg_seq init_acc_cfg[] = {
	{ APU_ACC_CONFG_SET0, BIT(BIT_SEL_APU) },
	{ APU_ACC_CONFG_CLR0, BIT(BIT_CGEN_SOC) },
	{ APU_ACC_CONFG_SET0, BIT(BIT_SEL_APU_DIV2) },
	{ APU_ACC_CONFG_SET7, BIT(BIT_SEL_APU) },
	{ APU_ACC_CONFG_CLR7, BIT(BIT_CGEN_SOC) },
	{ APU_ACC_CONFG_SET7, BIT(BIT_SEL_APU_DIV2) },
	{ APU_ACC_CONFG_SET1, BIT(BIT_SEL_APU) },
	{ APU_ACC_CONFG_CLR1, BIT(BIT_CGEN_SOC) },
	{ APU_ACC_CONFG_SET1, BIT(BIT_SEL_APU_DIV2) },
	{ APU_ACC_CONFG_SET2, BIT(BIT_INVEN_OUT) },
	{ APU_ACC_CONFG_SET2, BIT(BIT_SEL_APU) },
	{ APU_ACC_CONFG_CLR2, BIT(BIT_CGEN_SOC) },
	{ APU_ACC_CONFG_SET2, BIT(BIT_SEL_APU_DIV2) },
	{ APU_ACC_CONFG_SET4, BIT(BIT_SEL_APU) },
	{ APU_ACC_CONFG_CLR4, BIT(BIT_CGEN_SOC) },
	{ APU_ACC_CONFG_SET4, BIT(BIT_SEL_APU_DIV2) },
	{ APU_ACC_CONFG_SET5, BIT(BIT_INVEN_OUT) },
	{ APU_ACC_CONFG_SET5, BIT(BIT_SEL_APU) },
	{ APU_ACC_CONFG_CLR5, BIT(BIT_CGEN_SOC) },
	{ APU_ACC_CONFG_SET5, BIT(BIT_SEL_APU_DIV2) },
};

int32_t apupwr_smc_acc_init_all(void)
{
	int32_t i;

	for (i = 0; i < ARRAY_SIZE(init_acc_cfg); i++) {
		apupwr_writel(init_acc_cfg[i].val,
			      init_acc_cfg[i].address);
	}

	/* Deault ACC will raise APU_DIV_2 */
	apupwr_smc_pll_set_rate(BUCK_VCONN_DOMAIN_DEFAULT_FREQ,
				true, V_APU_CONN);

	apupwr_smc_pll_set_rate(BUCK_VCONN_DOMAIN_DEFAULT_FREQ,
				true, V_TOP_IOMMU);

	apupwr_smc_pll_set_rate(BUCK_VVPU_DOMAIN_DEFAULT_FREQ,
				true, V_VPU0);

	apupwr_smc_pll_set_rate(BUCK_VMDLA_DOMAIN_DEFAULT_FREQ,
				true, V_MDLA0);

	return 0;
}

void apupwr_smc_acc_top(bool enable)
{
	if (enable) {
		apupwr_writel(BIT(BIT_CGEN_APU), aacc_set[V_APU_CONN]);
		apupwr_writel(BIT(BIT_CGEN_APU), aacc_set[V_TOP_IOMMU]);
	} else {
		apupwr_writel(BIT(BIT_CGEN_APU), aacc_clr[V_APU_CONN]);
		apupwr_writel(BIT(BIT_CGEN_APU), aacc_clr[V_TOP_IOMMU]);
	}
}

/*
 * acc_clk_set_parent:ACC MUX select
 * 0. freq parameters here, only ACC clksrc is valid
 * 1. Switch between APUPLL <=> Parking (F26M, PARK)
 * 2. Turn on/off CG_F26M, CG_PARK, CG_SOC, but no CG_APU
 * 3. Clear APU Div2 while Parking
 * 4. Only use clksrc of APUPLL while ACC CG_APU is on
 */
int32_t apupwr_smc_acc_set_parent(uint32_t freq, uint32_t domain)
{
	uint32_t acc_set = 0;
	uint32_t acc_clr = 0;
	int32_t ret = 0;

	if (freq > DVFS_FREQ_ACC_APUPLL) {
		ERROR("%s wrong clksrc: %d\n", __func__, freq);
		ret = -EIO;
		goto err;
	}

	switch (domain) {
	case V_VPU1:
	case V_VPU0:
	case V_MDLA1:
	case V_MDLA0:
	case V_APU_CONN:
	case V_TOP_IOMMU:
		acc_set = aacc_set[domain];
		acc_clr = aacc_clr[domain];
		break;
	default:
		ret = -EIO;
		break;
	}

	/* Select park source */
	switch (freq) {
	case DVFS_FREQ_ACC_PARKING:
		/* Select park source */
		apupwr_writel(BIT(BIT_SEL_PARK), acc_set);
		apupwr_writel(BIT(BIT_SEL_F26M), acc_clr);
		/* Enable park cg */
		apupwr_writel(BIT(BIT_CGEN_PARK), acc_set);
		apupwr_writel(BIT(BIT_CGEN_F26M) | BIT(BIT_CGEN_SOC), acc_clr);
		/* Select park path */
		apupwr_writel(BIT(BIT_SEL_APU), acc_clr);
		/* clear apu div 2 */
		apupwr_writel(BIT(BIT_SEL_APU_DIV2), acc_clr);
		break;

	case DVFS_FREQ_ACC_APUPLL:
		/* Select park path */
		apupwr_writel(BIT(BIT_SEL_APU), acc_set);
		/* Clear park cg */
		apupwr_writel(BIT(BIT_CGEN_PARK) | BIT(BIT_CGEN_F26M) |
			      BIT(BIT_CGEN_SOC), acc_clr);
		break;

	case DVFS_FREQ_ACC_SOC:
		/* Select park source */
		apupwr_writel(BIT(BIT_SEL_PARK), acc_clr);
		apupwr_writel(BIT(BIT_SEL_F26M), acc_clr);
		/* Enable park cg */
		apupwr_writel(BIT(BIT_CGEN_SOC), acc_set);
		apupwr_writel(BIT(BIT_CGEN_F26M) | BIT(BIT_CGEN_PARK), acc_clr);
		/* Select park path */
		apupwr_writel(BIT(BIT_SEL_APU), acc_clr);
		/* clear apu div 2 */
		apupwr_writel(BIT(BIT_SEL_APU_DIV2), acc_clr);
		break;

	case DVFS_FREQ_ACC_26M:
	case DVFS_FREQ_NOT_SUPPORT:
	default:
		/* Select park source */
		apupwr_writel(BIT(BIT_SEL_F26M), acc_set);
		apupwr_writel(BIT(BIT_SEL_PARK), acc_clr);
		/* Enable park cg */
		apupwr_writel(BIT(BIT_CGEN_F26M), acc_set);
		apupwr_writel(BIT(BIT_CGEN_PARK) | BIT(BIT_CGEN_SOC), acc_clr);
		/* Select park path */
		apupwr_writel(BIT(BIT_SEL_APU), acc_clr);
		/* clear apu div 2 */
		apupwr_writel(BIT(BIT_SEL_APU_DIV2), acc_clr);
		ERROR("[APUPWR] %s wrong ACC clksrc : %d, force assign 26M\n",
		      __func__, freq);
		break;
	}

err:
	return ret;
}

int32_t apupwr_smc_pll_set_rate(uint32_t freq, bool div2, uint32_t domain)
{
	int32_t ret = 0;
	uint32_t acc_set0 = 0, acc_set1 = 0;

	if (freq > DVFS_FREQ_MAX) {
		ERROR("%s wrong freq: %d\n", __func__, freq);
		ret = -EIO;
		goto err;
	}

	/*
	 * Switch to Parking src
	 * 1. Need to switch out all ACCs sharing the same apupll
	 */
	switch (domain) {
	case V_MDLA0:
	case V_MDLA1:
		acc_set0 = APU_ACC_CONFG_SET4;
		acc_set1 = APU_ACC_CONFG_SET5;
		ret = apupwr_smc_acc_set_parent(DVFS_FREQ_ACC_PARKING,
						V_MDLA0);
		ret = apupwr_smc_acc_set_parent(DVFS_FREQ_ACC_PARKING,
						V_MDLA1);
		break;
	case V_VPU0:
	case V_VPU1:
		acc_set0 = APU_ACC_CONFG_SET1;
		acc_set1 = APU_ACC_CONFG_SET2;
		ret = apupwr_smc_acc_set_parent(DVFS_FREQ_ACC_PARKING,
						V_VPU0);
		ret = apupwr_smc_acc_set_parent(DVFS_FREQ_ACC_PARKING,
						V_VPU1);
		break;
	case V_APU_CONN:
		acc_set0 = APU_ACC_CONFG_SET0;
		ret = apupwr_smc_acc_set_parent(DVFS_FREQ_ACC_PARKING,
						V_APU_CONN);
		break;
	case V_TOP_IOMMU:
		acc_set0 = APU_ACC_CONFG_SET7;
		ret = apupwr_smc_acc_set_parent(DVFS_FREQ_ACC_PARKING,
						V_TOP_IOMMU);
		break;
	default:
		ERROR("[APUPWR] %s %d invalid domain (%d)\n",
		      __func__, __LINE__, domain);
		ret = -EIO;
		goto err;
	}

	anpu_pll_set_rate(domain, PLL_MODE, (div2) ? (freq * 2) : freq);

	if (div2) {
		apupwr_writel(BIT(BIT_SEL_APU_DIV2), acc_set0);
		if (acc_set1) {
			apupwr_writel(BIT(BIT_SEL_APU_DIV2), acc_set1);
		}
	}

	/*
	 * Switch back to APUPLL
	 * Only switch back to APUPLL while CG_APU on
	 * And clksrc is not APUPLL
	 */
	switch (domain) {
	case V_VPU0:
	case V_VPU1:
		if ((apupwr_readl(acc_set0) & BIT(BIT_CGEN_APU)) &&
		     !(apupwr_readl(acc_set0) & BIT(BIT_SEL_APU))) {
			ret = apupwr_smc_acc_set_parent(DVFS_FREQ_ACC_APUPLL,
							V_VPU0);
		}
		if ((apupwr_readl(acc_set1) & BIT(BIT_CGEN_APU)) &&
		     !(apupwr_readl(acc_set1) & BIT(BIT_SEL_APU))) {
			ret = apupwr_smc_acc_set_parent(DVFS_FREQ_ACC_APUPLL,
							V_VPU1);
		}
		break;
	case V_MDLA0:
	case V_MDLA1:
		if ((apupwr_readl(acc_set0) & BIT(BIT_CGEN_APU)) &&
		     !(apupwr_readl(acc_set0) & BIT(BIT_SEL_APU))) {
			ret = apupwr_smc_acc_set_parent(DVFS_FREQ_ACC_APUPLL,
							V_MDLA0);
		}
		if ((apupwr_readl(acc_set1) & BIT(BIT_CGEN_APU)) &&
		     !(apupwr_readl(acc_set1) & BIT(BIT_SEL_APU))) {
			ret = apupwr_smc_acc_set_parent(DVFS_FREQ_ACC_APUPLL,
							V_MDLA1);
		}
		break;
	case V_APU_CONN:
	case V_TOP_IOMMU:
		if ((apupwr_readl(acc_set0) & BIT(BIT_CGEN_APU)) &&
		    !(apupwr_readl(acc_set0) & BIT(BIT_SEL_APU))) {
			ret = apupwr_smc_acc_set_parent(DVFS_FREQ_ACC_APUPLL,
							domain);
		}
		break;
	default:
		ERROR("[APUPWR] %s %d invalid domain (%d)\n",
		      __func__, __LINE__, domain);
		ret = -EIO;
		break;
	}
	INFO("[%s][%d] set domain %d to freq %d\n",
	     __func__, __LINE__, domain, (div2) ? (freq * 2) : freq);

err:
	return ret;
}

int32_t apupwr_smc_bulk_pll(bool enable)
{
	int32_t ret = 0;
	int32_t pll_idx;

	if (enable) {
		for (pll_idx = APUPLL; pll_idx < APUPLL_MAX; pll_idx++) {
			ret = apu_pll_enable(pll_idx, enable, false);
			if (ret != 0) {
				goto err;
			}
		}
	} else {
		for (pll_idx = APUPLL2; pll_idx >= APUPLL; pll_idx--) {
			ret = apu_pll_enable(pll_idx, enable, false);
			if (ret != 0) {
				goto err;
			}
		}
	}

err:
	return ret;
}

void apupwr_smc_bus_prot_cg_on(void)
{
	apupwr_clrbits(AO_MD32_MNOC_MASK, APU_CSR_DUMMY_0);
}
