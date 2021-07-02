/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_CONDIT_H
#define MT_SPM_CONDIT_H

#include <mt_lp_rm.h>

enum PLAT_SPM_COND {
	PLAT_SPM_COND_MTCMOS1 = 0,
	PLAT_SPM_COND_CG_INFRA_0,
	PLAT_SPM_COND_CG_INFRA_1,
	PLAT_SPM_COND_CG_INFRA_2,
	PLAT_SPM_COND_CG_INFRA_3,
	PLAT_SPM_COND_CG_INFRA_4,
	PLAT_SPM_COND_CG_PERI_SW_0,
	PLAT_SPM_COND_CG_VPPSYS0_SW_CG_0,
	PLAT_SPM_COND_CG_VPPSYS0_SW_CG_1,
	PLAT_SPM_COND_CG_VPPSYS0_SW_CG_2,
	PLAT_SPM_COND_CG_VPPSYS1_SW_CG_0,
	PLAT_SPM_COND_CG_VPPSYS1_SW_CG_1,
	PLAT_SPM_COND_CG_VDOSYS0_SW_CG_0,
	PLAT_SPM_COND_CG_VDOSYS0_SW_CG_1,
	PLAT_SPM_COND_CG_VDOSYS1_SW_CG_0,
	PLAT_SPM_COND_CG_VDOSYS1_SW_CG_1,
	PLAT_SPM_COND_CG_VDOSYS1_SW_CG_2,
	PLAT_SPM_COND_CG_I2C_SW_CG,
	PLAT_SPM_COND_MAX,
};

enum PLAT_SPM_COND_PLL {
	PLAT_SPM_COND_PLL_UNIVPLL = 0,
	PLAT_SPM_COND_PLL_MFGPLL,
	PLAT_SPM_COND_PLL_MSDCPLL,
	PLAT_SPM_COND_PLL_TVDPLL,
	PLAT_SPM_COND_PLL_MMPLL,
	PLAT_SPM_COND_PLL_MAX,
};

#define PLL_BIT_MFGPLL	BIT(PLAT_SPM_COND_PLL_MFGPLL)
#define PLL_BIT_MMPLL	BIT(PLAT_SPM_COND_PLL_MMPLL)
#define PLL_BIT_UNIVPLL	BIT(PLAT_SPM_COND_PLL_UNIVPLL)
#define PLL_BIT_MSDCPLL	BIT(PLAT_SPM_COND_PLL_MSDCPLL)
#define PLL_BIT_TVDPLL	BIT(PLAT_SPM_COND_PLL_TVDPLL)

/* Definition about SPM_COND_CHECK_BLOCKED
 * bit [00 ~ 17]: cg blocking index
 * bit [18 ~ 29]: pll blocking index
 * bit [30]     : pll blocking information
 * bit [31]	: idle condition check fail
 */
#define SPM_COND_BLOCKED_CG_IDX		U(0)
#define SPM_COND_BLOCKED_PLL_IDX	U(18)
#define SPM_COND_CHECK_BLOCKED_PLL	BIT(30)
#define SPM_COND_CHECK_FAIL		BIT(31)

struct mt_spm_cond_tables {
	char *name;
	unsigned int table_cg[PLAT_SPM_COND_MAX];
	unsigned int table_pll;
	void *priv;
};

extern unsigned int mt_spm_cond_check(int state_id,
				      const struct mt_spm_cond_tables *src,
				      const struct mt_spm_cond_tables *dest,
				      struct mt_spm_cond_tables *res);
extern int mt_spm_cond_update(struct mt_resource_constraint **con,
			      int stateid, void *priv);
#endif /* MT_SPM_CONDIT_H */
