/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_COND_H
#define MT_SPM_COND_H

#include <lpm_v2/mt_lp_rm.h>

#define SPM_RC_UPDATE_COND_ID_MASK 0xffff
#define SPM_RC_UPDATE_COND_RC_ID_MASK 0xffff
#define SPM_RC_UPDATE_COND_RC_ID_SHIFT (16)

#define SPM_RC_UPDATE_COND_RC_ID_GET(val)          \
	((val >> SPM_RC_UPDATE_COND_RC_ID_SHIFT) & \
	 SPM_RC_UPDATE_COND_RC_ID_MASK)

#define SPM_RC_UPDATE_COND_ID_GET(val) (val & SPM_RC_UPDATE_COND_ID_MASK)
/*
 * Definition about SPM_COND_CHECK_BLOCKED
 * bit [00 ~ 15]: cg blocking index
 * bit [16 ~ 29]: pll blocking index
 * bit [62]	: pll blocking information
 * bit [63]	: idle condition check fail
 */
#define SPM_COND_BLOCKED_CG_IDX (0)
#define SPM_COND_BLOCKED_PLL_IDX (16)
#define SPM_COND_CHECK_BLOCKED_PLL ((uint64_t)1 << 62L)
#define SPM_COND_CHECK_FAIL ((uint64_t)1 << 63L)

enum plat_spm_cond_pll {
	PLAT_SPM_COND_PLL_UNIV = 0,
	PLAT_SPM_COND_PLL_UNIV2,
	PLAT_SPM_COND_PLL_MFG,
	PLAT_SPM_COND_PLL_MFGSC,
	PLAT_SPM_COND_PLL_SENSON,
	PLAT_SPM_COND_PLL_MSDC,
	PLAT_SPM_COND_PLL_UFS,
	PLAT_SPM_COND_PLL_TVD,
	PLAT_SPM_COND_PLL_MM,
	PLAT_SPM_COND_PLL_MM2,
	PLAT_SPM_COND_PLL_MAIN2,
	PLAT_SPM_COND_PLL_IMG,
	PLAT_SPM_COND_PLL_USB,
	PLAT_SPM_COND_PLL_ADSP,
	PLAT_SPM_COND_PLL_APLL1,
	PLAT_SPM_COND_PLL_APLL2,
	PLAT_SPM_COND_PLL_MAX
};

#define PLL_BIT_MFG (1 << PLAT_SPM_COND_PLL_MFG)
#define PLL_BIT_MFGSC (1 << PLAT_SPM_COND_PLL_MFGSC)
#define PLL_BIT_SENSON (1 << PLAT_SPM_COND_PLL_SENSON)
#define PLL_BIT_MM (1 << PLAT_SPM_COND_PLL_MM)
#define PLL_BIT_MM2 (1 << PLAT_SPM_COND_PLL_MM2)
#define PLL_BIT_MAIN2 (1 << PLAT_SPM_COND_PLL_MAIN2)
#define PLL_BIT_UNIV (1 << PLAT_SPM_COND_PLL_UNIV)
#define PLL_BIT_UNIV2 (1 << PLAT_SPM_COND_PLL_UNIV2)
#define PLL_BIT_MSDC (1 << PLAT_SPM_COND_PLL_MSDC)
#define PLL_BIT_UFS (1 << PLAT_SPM_COND_PLL_UFS)
#define PLL_BIT_TVD (1 << PLAT_SPM_COND_PLL_TVD)
#define PLL_BIT_IMG (1 << PLAT_SPM_COND_PLL_IMG)
#define PLL_BIT_USB (1 << PLAT_SPM_COND_PLL_USB)
#define PLL_BIT_ADSP (1 << PLAT_SPM_COND_PLL_ADSP)
#define PLL_BIT_APLL1 (1 << PLAT_SPM_COND_PLL_APLL1)
#define PLL_BIT_APLL2 (1 << PLAT_SPM_COND_PLL_APLL2)
#define PLL_BIT_MAX (1 << PLAT_SPM_COND_PLL_MAX)

struct mt_spm_cond_tables {
	unsigned int *table_cg;
	uint64_t table_pll;
	void *priv;
};

struct plat_idle_cond_info {
	/* check SPM_PWR_STATUS for bit definition */
	unsigned int pwr_status_mask;
	/* cg address */
	uintptr_t cg_addr;
	/* bitflip value from *addr ? */
	bool bBitflip;
	/* check clkmux */
	uintptr_t clkmux_addr;
	unsigned int clkmux_mask;
};

struct plat_pll_cond_info {
	uintptr_t pll_addr;
	unsigned int pll_mask;
	unsigned int pll_bit_set;
};

struct plat_cond_info_t {
	uintptr_t spm_pwr_status_addr;
	uintptr_t spm_pwr_status_2nd_addr;
	struct plat_idle_cond_info *idle_cond_table;
	struct plat_pll_cond_info *pll_cond_table;
	unsigned int idle_cond_num;
	struct mt_spm_cond_tables cond_table_buf;
};

uint64_t mt_spm_cond_check(const struct mt_spm_cond_tables *src,
			   const struct mt_spm_cond_tables *dest,
			   struct mt_spm_cond_tables *res);
uint64_t mt_spm_dump_all_pll(const struct mt_spm_cond_tables *src,
			     const struct mt_spm_cond_tables *dest,
			     struct mt_spm_cond_tables *res);

int mt_spm_cond_update(struct mt_resource_constraint **con, int num,
		       int stateid, void *priv);
int register_plat_cond_info(struct plat_cond_info_t *cond);

#endif
