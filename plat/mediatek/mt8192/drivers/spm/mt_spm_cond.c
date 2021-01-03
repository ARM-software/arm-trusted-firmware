/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <mt_spm_cond.h>
#include <mt_spm_conservation.h>
#include <mt_spm_constraint.h>
#include <plat_mtk_lpm.h>
#include <plat_pm.h>
#include <platform_def.h>

#define MT_LP_TZ_INFRA_REG(ofs)		(INFRACFG_AO_BASE + ofs)
#define MT_LP_TZ_MM_REG(ofs)		(MMSYS_BASE + ofs)
#define MT_LP_TZ_SPM_REG(ofs)		(SPM_BASE + ofs)
#define MT_LP_TZ_TOPCK_REG(ofs)		(TOPCKGEN_BASE + ofs)
#define MT_LP_TZ_APMIXEDSYS(ofs)	(APMIXEDSYS + ofs)

#define SPM_PWR_STATUS			MT_LP_TZ_SPM_REG(0x016C)
#define SPM_PWR_STATUS_2ND		MT_LP_TZ_SPM_REG(0x0170)
#define	INFRA_SW_CG0			MT_LP_TZ_INFRA_REG(0x0094)
#define	INFRA_SW_CG1			MT_LP_TZ_INFRA_REG(0x0090)
#define	INFRA_SW_CG2			MT_LP_TZ_INFRA_REG(0x00AC)
#define	INFRA_SW_CG3			MT_LP_TZ_INFRA_REG(0x00C8)
#define INFRA_SW_CG4                    MT_LP_TZ_INFRA_REG(0x00D8)
#define INFRA_SW_CG5                    MT_LP_TZ_INFRA_REG(0x00E8)
#define MMSYS_CG_CON0			MT_LP_TZ_MM_REG(0x100)
#define MMSYS_CG_CON1			MT_LP_TZ_MM_REG(0x110)
#define MMSYS_CG_CON2                   MT_LP_TZ_MM_REG(0x1A0)

/***********************************************************
 * Check clkmux registers
 ***********************************************************/
#define CLK_CFG(id)	MT_LP_TZ_TOPCK_REG(0x20 + id * 0x10)
#define PDN_CHECK	BIT(7)
#define CLK_CHECK	BIT(31)

enum {
	CLKMUX_DISP = 0,
	CLKMUX_MDP  = 1,
	CLKMUX_IMG1 = 2,
	CLKMUX_IMG2 = 3,
	NF_CLKMUX,
};

static bool is_clkmux_pdn(unsigned int clkmux_id)
{
	unsigned int reg, val, idx;

	if ((clkmux_id & CLK_CHECK) != 0U) {
		clkmux_id = (clkmux_id & ~CLK_CHECK);
		reg = clkmux_id / 4U;
		val = mmio_read_32(CLK_CFG(reg));
		idx = clkmux_id % 4U;
		val = (val >> (idx * 8U)) & PDN_CHECK;
		return (val != 0U);
	}

	return false;
}

static struct mt_spm_cond_tables spm_cond_t;

struct idle_cond_info {
	unsigned int subsys_mask;
	uintptr_t addr;
	bool bBitflip;
	unsigned int clkmux_id;
};

#define IDLE_CG(mask, addr, bitflip, clkmux)	\
	{mask, (uintptr_t)addr, bitflip, clkmux}

static struct idle_cond_info idle_cg_info[PLAT_SPM_COND_MAX] = {
	IDLE_CG(0xffffffff, SPM_PWR_STATUS, false, 0U),
	IDLE_CG(0x00000200, INFRA_SW_CG0, true, 0U),
	IDLE_CG(0x00000200, INFRA_SW_CG1, true, 0U),
	IDLE_CG(0x00000200, INFRA_SW_CG2, true, 0U),
	IDLE_CG(0x00000200, INFRA_SW_CG3, true, 0U),
	IDLE_CG(0x00000200, INFRA_SW_CG4, true, 0U),
	IDLE_CG(0x00000200, INFRA_SW_CG5, true, 0U),
	IDLE_CG(0x00100000, MMSYS_CG_CON0, true, (CLK_CHECK | CLKMUX_DISP)),
	IDLE_CG(0x00100000, MMSYS_CG_CON1, true, (CLK_CHECK | CLKMUX_DISP)),
	IDLE_CG(0x00100000, MMSYS_CG_CON2, true, (CLK_CHECK | CLKMUX_DISP)),
};

/***********************************************************
 * Check pll idle condition
 ***********************************************************/
#define PLL_MFGPLL	MT_LP_TZ_APMIXEDSYS(0x268)
#define PLL_MMPLL	MT_LP_TZ_APMIXEDSYS(0x360)
#define PLL_UNIVPLL	MT_LP_TZ_APMIXEDSYS(0x308)
#define PLL_MSDCPLL	MT_LP_TZ_APMIXEDSYS(0x350)
#define PLL_TVDPLL	MT_LP_TZ_APMIXEDSYS(0x380)

unsigned int mt_spm_cond_check(int state_id,
			       const struct mt_spm_cond_tables *src,
			       const struct mt_spm_cond_tables *dest,
			       struct mt_spm_cond_tables *res)
{
	unsigned int blocked = 0U, i;
	bool is_system_suspend = IS_PLAT_SUSPEND_ID(state_id);

	if ((src == NULL) || (dest == NULL)) {
		return SPM_COND_CHECK_FAIL;
	}

	for (i = 0U; i < PLAT_SPM_COND_MAX; i++) {
		if (res != NULL) {
			res->table_cg[i] =
				(src->table_cg[i] & dest->table_cg[i]);

			if (is_system_suspend && (res->table_cg[i] != 0U)) {
				INFO("suspend: %s block[%u](0x%lx) = 0x%08x\n",
				     dest->name, i, idle_cg_info[i].addr,
				     res->table_cg[i]);
			}

			if (res->table_cg[i] != 0U) {
				blocked |= (1U << i);
			}
		} else if ((src->table_cg[i] & dest->table_cg[i]) != 0U) {
			blocked |= (1U << i);
			break;
		}
	}

	if (res != NULL) {
		res->table_pll = (src->table_pll & dest->table_pll);

		if (res->table_pll != 0U) {
			blocked |=
				(res->table_pll << SPM_COND_BLOCKED_PLL_IDX) |
				 SPM_COND_CHECK_BLOCKED_PLL;
		}
	} else if ((src->table_pll & dest->table_pll) != 0U) {
		blocked |= SPM_COND_CHECK_BLOCKED_PLL;
	}

	return blocked;
}

#define IS_MT_SPM_PWR_OFF(mask)					\
	(((mmio_read_32(SPM_PWR_STATUS) & mask) == 0U) &&	\
	 ((mmio_read_32(SPM_PWR_STATUS_2ND) & mask) == 0U))

int mt_spm_cond_update(struct mt_resource_constraint **con,
		       int stateid, void *priv)
{
	int res;
	uint32_t i;
	struct mt_resource_constraint *const *rc;

	/* read all cg state */
	for (i = 0U; i < PLAT_SPM_COND_MAX; i++) {
		spm_cond_t.table_cg[i] = 0U;

		/* check mtcmos, if off set idle_value and clk to 0 disable */
		if (IS_MT_SPM_PWR_OFF(idle_cg_info[i].subsys_mask)) {
			continue;
		}

		/* check clkmux */
		if (is_clkmux_pdn(idle_cg_info[i].clkmux_id)) {
			continue;
		}

		spm_cond_t.table_cg[i] = idle_cg_info[i].bBitflip ?
					 ~mmio_read_32(idle_cg_info[i].addr) :
					 mmio_read_32(idle_cg_info[i].addr);
	}

	spm_cond_t.table_pll = 0U;
	if ((mmio_read_32(PLL_MFGPLL) & 0x1) != 0U) {
		spm_cond_t.table_pll |= PLL_BIT_MFGPLL;
	}

	if ((mmio_read_32(PLL_MMPLL) & 0x1) != 0U) {
		spm_cond_t.table_pll |= PLL_BIT_MMPLL;
	}

	if ((mmio_read_32(PLL_UNIVPLL) & 0x1) != 0U) {
		spm_cond_t.table_pll |= PLL_BIT_UNIVPLL;
	}

	if ((mmio_read_32(PLL_MSDCPLL) & 0x1) != 0U) {
		spm_cond_t.table_pll |= PLL_BIT_MSDCPLL;
	}

	if ((mmio_read_32(PLL_TVDPLL) & 0x1) != 0U) {
		spm_cond_t.table_pll |= PLL_BIT_TVDPLL;
	}

	spm_cond_t.priv = priv;
	for (rc = con; *rc != NULL; rc++) {
		if (((*rc)->update) == NULL) {
			continue;
		}

		res = (*rc)->update(stateid, PLAT_RC_UPDATE_CONDITION,
				    (void const *)&spm_cond_t);
		if (res != MT_RM_STATUS_OK) {
			break;
		}
	}

	return 0;
}
