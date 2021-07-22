/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
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
	PLAT_SPM_COND_CG_INFRA_5,
	PLAT_SPM_COND_CG_MMSYS_0,
	PLAT_SPM_COND_CG_MMSYS_1,
	PLAT_SPM_COND_CG_MMSYS_2,
	PLAT_SPM_COND_MAX,
};

#define PLL_BIT_UNIVPLL	BIT(0)
#define PLL_BIT_MFGPLL	BIT(1)
#define PLL_BIT_MSDCPLL	BIT(2)
#define PLL_BIT_TVDPLL	BIT(3)
#define PLL_BIT_MMPLL	BIT(4)

/* Definition about SPM_COND_CHECK_BLOCKED
 * bit [00 ~ 15]: cg blocking index
 * bit [16 ~ 29]: pll blocking index
 * bit [30]     : pll blocking information
 * bit [31]	: idle condition check fail
 */
#define SPM_COND_BLOCKED_CG_IDX		U(0)
#define SPM_COND_BLOCKED_PLL_IDX	U(16)
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
