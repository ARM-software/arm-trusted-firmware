/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_COND_H
#define MT_SPM_COND_H

#include <lpm/mt_lp_rm.h>

enum plat_spm_cond {
	PLAT_SPM_COND_MTCMOS1 = 0,
	PLAT_SPM_COND_MTCMOS2,
	PLAT_SPM_COND_CG_INFRA_0,
	PLAT_SPM_COND_CG_INFRA_1,
	PLAT_SPM_COND_CG_INFRA_2,
	PLAT_SPM_COND_CG_INFRA_3,
	PLAT_SPM_COND_CG_INFRA_4,
	PLAT_SPM_COND_CG_PERI_0,
	PLAT_SPM_COND_CG_VPPSYS0_0,
	PLAT_SPM_COND_CG_VPPSYS0_1,
	PLAT_SPM_COND_CG_VPPSYS1_0,
	PLAT_SPM_COND_CG_VPPSYS1_1,
	PLAT_SPM_COND_CG_VDOSYS0_0,
	PLAT_SPM_COND_CG_VDOSYS0_1,
	PLAT_SPM_COND_CG_VDOSYS1_0,
	PLAT_SPM_COND_CG_VDOSYS1_1,
	PLAT_SPM_COND_CG_VDOSYS1_2,
	PLAT_SPM_COND_MAX,
};

/* For PLL id >= PLAT_SPM_COND_PLL_MAX is not checked in idle condition  */
enum plat_spm_cond_pll {
	PLAT_SPM_COND_PLL_UNIVPLL = 0,
	PLAT_SPM_COND_PLL_MFGPLL,
	PLAT_SPM_COND_PLL_MSDCPLL,
	PLAT_SPM_COND_PLL_TVDPLL1,
	PLAT_SPM_COND_PLL_TVDPLL2,
	PLAT_SPM_COND_PLL_MMPLL,
	PLAT_SPM_COND_PLL_ETHPLL,
	PLAT_SPM_COND_PLL_IMGPLL,
	PLAT_SPM_COND_PLL_APLL1,
	PLAT_SPM_COND_PLL_APLL2,
	PLAT_SPM_COND_PLL_APLL3,
	PLAT_SPM_COND_PLL_APLL4,
	PLAT_SPM_COND_PLL_APLL5,
	PLAT_SPM_COND_PLL_MAX,
};

#define PLL_BIT_MFGPLL	BIT(PLAT_SPM_COND_PLL_MFGPLL)
#define PLL_BIT_MMPLL	BIT(PLAT_SPM_COND_PLL_MMPLL)
#define PLL_BIT_UNIVPLL	BIT(PLAT_SPM_COND_PLL_UNIVPLL)
#define PLL_BIT_MSDCPLL	BIT(PLAT_SPM_COND_PLL_MSDCPLL)
#define PLL_BIT_TVDPLL1	BIT(PLAT_SPM_COND_PLL_TVDPLL1)
#define PLL_BIT_TVDPLL2	BIT(PLAT_SPM_COND_PLL_TVDPLL2)
#define PLL_BIT_ETHPLL	BIT(PLAT_SPM_COND_PLL_ETHPLL)
#define PLL_BIT_IMGPLL	BIT(PLAT_SPM_COND_PLL_IMGPLL)
#define PLL_BIT_APLL1	BIT(PLAT_SPM_COND_PLL_APLL1)
#define PLL_BIT_APLL2	BIT(PLAT_SPM_COND_PLL_APLL2)
#define PLL_BIT_APLL3   BIT(PLAT_SPM_COND_PLL_APLL3)
#define PLL_BIT_APLL4	BIT(PLAT_SPM_COND_PLL_APLL4)
#define PLL_BIT_APLL5	BIT(PLAT_SPM_COND_PLL_APLL5)

/*
 * Definition about SPM_COND_CHECK_BLOCKED
 * bit[00:16]: cg blocking index
 * bit[17:29]: pll blocking index
 * bit[30]: pll blocking information
 * bit[31]: idle condition check fail
 */
#define SPM_COND_BLOCKED_CG_IDX		(0)
#define SPM_COND_BLOCKED_PLL_IDX	(17)
#define SPM_COND_CHECK_BLOCKED_PLL	BIT(30)
#define SPM_COND_CHECK_FAIL		BIT(31)

struct mt_spm_cond_tables {
	char *name;
	unsigned int table_cg[PLAT_SPM_COND_MAX];
	unsigned int table_pll;
	unsigned int table_all_pll;
	void *priv;
};

unsigned int mt_spm_cond_check(int state_id,
			       const struct mt_spm_cond_tables *src,
			       const struct mt_spm_cond_tables *dest,
			       struct mt_spm_cond_tables *res);
unsigned int mt_spm_dump_all_pll(const struct mt_spm_cond_tables *src,
				 const struct mt_spm_cond_tables *dest,
				 struct mt_spm_cond_tables *res);
int mt_spm_cond_update(struct mt_resource_constraint **con, unsigned int num,
		       int stateid, void *priv);

#endif
