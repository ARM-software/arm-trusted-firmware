/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <lib/mmio.h>
#include <lib/pm/mtk_pm.h>
#include <mt_spm_cond.h>
#include <mt_spm_conservation.h>
#include <mt_spm_constraint.h>
#include <platform_def.h>

#define TOPCKGEB_BASE			(IO_PHYS)

#define MT_LP_TZ_INFRA_REG(ofs)		(INFRACFG_AO_BASE + ofs)

#define MT_LP_TZ_SPM_REG(ofs)		(SPM_BASE + ofs)
#define MT_LP_TZ_TOPCK_REG(ofs)		(TOPCKGEB_BASE + ofs)
#define MT_LP_TZ_APMIXEDSYS(ofs)	(APMIXEDSYS + ofs)

#define MT_LP_TZ_VPPSYS0_REG(ofs)	(VPPSYS0_BASE + ofs)
#define MT_LP_TZ_VPPSYS1_REG(ofs)	(VPPSYS1_BASE + ofs)
#define MT_LP_TZ_VDOSYS0_REG(ofs)	(VDOSYS0_BASE + ofs)
#define MT_LP_TZ_VDOSYS1_REG(ofs)	(VDOSYS1_BASE + ofs)

#define MT_LP_TZ_PERI_AO_REG(ofs)	(PERICFG_AO_BASE + ofs)

#undef SPM_PWR_STATUS
#define SPM_PWR_STATUS			MT_LP_TZ_SPM_REG(0x016C)
#define SPM_PWR_STATUS_2ND		MT_LP_TZ_SPM_REG(0x0170)
#define SPM_CPU_PWR_STATUS		MT_LP_TZ_SPM_REG(0x0174)
#define	INFRA_SW_CG0			MT_LP_TZ_INFRA_REG(0x0090)
#define	INFRA_SW_CG1			MT_LP_TZ_INFRA_REG(0x0094)
#define	INFRA_SW_CG2			MT_LP_TZ_INFRA_REG(0x00AC)
#define	INFRA_SW_CG3			MT_LP_TZ_INFRA_REG(0x00C8)
#define	INFRA_SW_CG4			MT_LP_TZ_INFRA_REG(0x00E8)
#define	TOP_SW_I2C_CG			MT_LP_TZ_TOPCK_REG(0x00A4)
#define	PERI_SW_CG0			MT_LP_TZ_PERI_AO_REG(0x0018)
#define	VPPSYS0_SW_CG0			MT_LP_TZ_VPPSYS0_REG(0x0020)
#define	VPPSYS0_SW_CG1			MT_LP_TZ_VPPSYS0_REG(0x002C)
#define	VPPSYS0_SW_CG2			MT_LP_TZ_VPPSYS0_REG(0x0038)
#define	VPPSYS1_SW_CG0			MT_LP_TZ_VPPSYS1_REG(0x0100)
#define	VPPSYS1_SW_CG1			MT_LP_TZ_VPPSYS1_REG(0x0110)
#define	VDOSYS0_SW_CG0			MT_LP_TZ_VDOSYS0_REG(0x0100)
#define	VDOSYS0_SW_CG1			MT_LP_TZ_VDOSYS0_REG(0x0110)
#define	VDOSYS1_SW_CG0			MT_LP_TZ_VDOSYS1_REG(0x0100)
#define	VDOSYS1_SW_CG1			MT_LP_TZ_VDOSYS1_REG(0x0120)
#define	VDOSYS1_SW_CG2			MT_LP_TZ_VDOSYS1_REG(0x0130)

#define CLK_CFG(id)			MT_LP_TZ_TOPCK_REG(0x2c + id * 0xc)

enum {
	/* CLK_CFG_0 1000_002c */
	CLKMUX_VPP	= 0,
	NF_CLKMUX,
};

#define CLK_CHECK BIT(31)

static bool check_clkmux_pdn(unsigned int clkmux_id)
{
	unsigned int reg, val, idx;
	bool ret = false;

	if ((clkmux_id & CLK_CHECK) != 0U) {
		clkmux_id = (clkmux_id & ~CLK_CHECK);
		reg = clkmux_id / 4U;
		val = mmio_read_32(CLK_CFG(reg));
		idx = clkmux_id % 4U;
		ret = (((val >> (idx * 8U)) & 0x80) != 0U);
	}

	return ret;
}

static struct mt_spm_cond_tables spm_cond_t;

/* local definitions */
struct idle_cond_info {
	/* check SPM_PWR_STATUS for bit definition */
	unsigned int subsys_mask;
	/* cg address */
	uintptr_t addr;
	/* bitflip value from *addr ? */
	bool bBitflip;
	/* check clkmux if bit 31 = 1, id is bit[30:0] */
	unsigned int clkmux_id;
};

#define IDLE_CG(mask, addr, bitflip, clkmux)	{mask, (uintptr_t)addr, bitflip, clkmux}

static struct idle_cond_info idle_cg_info[PLAT_SPM_COND_MAX] = {
	IDLE_CG(0xffffffff, SPM_PWR_STATUS, false, 0),
	IDLE_CG(0xffffffff, SPM_CPU_PWR_STATUS, false, 0),
	IDLE_CG(0xffffffff, INFRA_SW_CG0, true, 0),
	IDLE_CG(0xffffffff, INFRA_SW_CG1, true, 0),
	IDLE_CG(0xffffffff, INFRA_SW_CG2, true, 0),
	IDLE_CG(0xffffffff, INFRA_SW_CG3, true, 0),
	IDLE_CG(0xffffffff, INFRA_SW_CG4, true, 0),
	IDLE_CG(0xffffffff, PERI_SW_CG0, true, 0),
	IDLE_CG(0x00000800, VPPSYS0_SW_CG0, true, (CLK_CHECK | CLKMUX_VPP)),
	IDLE_CG(0x00000800, VPPSYS0_SW_CG1, true, (CLK_CHECK | CLKMUX_VPP)),
	IDLE_CG(0x00001000, VPPSYS1_SW_CG0, true, (CLK_CHECK | CLKMUX_VPP)),
	IDLE_CG(0x00001000, VPPSYS1_SW_CG1, true, (CLK_CHECK | CLKMUX_VPP)),
	IDLE_CG(0x00002000, VDOSYS0_SW_CG0, true, (CLK_CHECK | CLKMUX_VPP)),
	IDLE_CG(0x00002000, VDOSYS0_SW_CG1, true, (CLK_CHECK | CLKMUX_VPP)),
	IDLE_CG(0x00004000, VDOSYS1_SW_CG0, true, (CLK_CHECK | CLKMUX_VPP)),
	IDLE_CG(0x00004000, VDOSYS1_SW_CG1, true, (CLK_CHECK | CLKMUX_VPP)),
	IDLE_CG(0x00004000, VDOSYS1_SW_CG2, true, (CLK_CHECK | CLKMUX_VPP)),
};

/* check pll idle condition */
#define PLL_MFGPLL	MT_LP_TZ_APMIXEDSYS(0x340)
#define PLL_MMPLL	MT_LP_TZ_APMIXEDSYS(0x544)
#define PLL_UNIVPLL	MT_LP_TZ_APMIXEDSYS(0x504)
#define PLL_MSDCPLL	MT_LP_TZ_APMIXEDSYS(0x514)
#define PLL_TVDPLL1	MT_LP_TZ_APMIXEDSYS(0x524)
#define PLL_TVDPLL2	MT_LP_TZ_APMIXEDSYS(0x534)
#define PLL_ETHPLL	MT_LP_TZ_APMIXEDSYS(0x44c)
#define PLL_IMGPLL	MT_LP_TZ_APMIXEDSYS(0x554)
#define PLL_APLL1	MT_LP_TZ_APMIXEDSYS(0x304)
#define PLL_APLL2	MT_LP_TZ_APMIXEDSYS(0x318)
#define PLL_APLL3	MT_LP_TZ_APMIXEDSYS(0x32c)
#define PLL_APLL4	MT_LP_TZ_APMIXEDSYS(0x404)
#define PLL_APLL5	MT_LP_TZ_APMIXEDSYS(0x418)

unsigned int mt_spm_cond_check(int state_id,
			       const struct mt_spm_cond_tables *src,
			       const struct mt_spm_cond_tables *dest,
			       struct mt_spm_cond_tables *res)
{
	unsigned int b_res = 0U;
	unsigned int i;
	bool is_system_suspend = IS_PLAT_SUSPEND_ID(state_id);

	if ((src == NULL) || (dest == NULL)) {
		return SPM_COND_CHECK_FAIL;
	}

	for (i = 0; i < PLAT_SPM_COND_MAX; i++) {
		if (res != NULL) {
			res->table_cg[i] = (src->table_cg[i] & dest->table_cg[i]);
			if (is_system_suspend && ((res->table_cg[i]) != 0U)) {
				INFO("suspend: %s block[%u](0x%lx) = 0x%08x\n",
				     dest->name, i, idle_cg_info[i].addr,
				     res->table_cg[i]);
			}

			if ((res->table_cg[i]) != 0U) {
				b_res |= BIT(i);
			}
		} else if ((src->table_cg[i] & dest->table_cg[i]) != 0U) {
			b_res |= BIT(i);
			break;
		}
	}

	if (res != NULL) {
		res->table_pll = (src->table_pll & dest->table_pll);

		if ((res->table_pll) != 0U) {
			b_res |= (res->table_pll << SPM_COND_BLOCKED_PLL_IDX) |
				 SPM_COND_CHECK_BLOCKED_PLL;
		}
	} else if ((src->table_pll & dest->table_pll) != 0U) {
		b_res |= SPM_COND_CHECK_BLOCKED_PLL;
	}

	if (is_system_suspend && ((b_res) != 0U)) {
		INFO("suspend: %s total blocked = 0x%08x\n", dest->name, b_res);
	}

	return b_res;
}

unsigned int mt_spm_dump_all_pll(const struct mt_spm_cond_tables *src,
				 const struct mt_spm_cond_tables *dest,
				 struct mt_spm_cond_tables *res)
{
	unsigned int b_res = 0U;

	if (res != NULL) {
		res->table_all_pll = src->table_all_pll;
		if ((res->table_all_pll) != 0U) {
			b_res |= (res->table_all_pll << SPM_COND_BLOCKED_PLL_IDX) |
				 SPM_COND_CHECK_BLOCKED_PLL;
		}
	} else if ((src->table_pll & dest->table_pll) != 0U) {
		b_res |= SPM_COND_CHECK_BLOCKED_PLL;
	}

	return b_res;
}

#define IS_MT_SPM_PWR_OFF(mask) \
	(!(mmio_read_32(SPM_PWR_STATUS) & mask) && \
	 !(mmio_read_32(SPM_PWR_STATUS_2ND) & mask))

int mt_spm_cond_update(struct mt_resource_constraint **con, unsigned int num,
		       int stateid, void *priv)
{
	static const struct {
		uintptr_t en_reg;
		uint32_t pll_b;
	} plls[] = {
		{ PLL_MFGPLL, PLL_BIT_MFGPLL },
		{ PLL_MMPLL, PLL_BIT_MMPLL },
		{ PLL_UNIVPLL, PLL_BIT_UNIVPLL },
		{ PLL_MSDCPLL, PLL_BIT_MSDCPLL },
		{ PLL_TVDPLL1, PLL_BIT_TVDPLL1 },
		{ PLL_TVDPLL2, PLL_BIT_TVDPLL2 },
		{ PLL_ETHPLL, PLL_BIT_ETHPLL },
		{ PLL_IMGPLL, PLL_BIT_IMGPLL },
		{ PLL_APLL1, PLL_BIT_APLL1 },
		{ PLL_APLL2, PLL_BIT_APLL2 },
		{ PLL_APLL3, PLL_BIT_APLL3 },
		{ PLL_APLL4, PLL_BIT_APLL4 },
		{ PLL_APLL5, PLL_BIT_APLL5 },
	};

	int res;
	unsigned int i;
	struct mt_resource_constraint *const *_con;

	/* read all cg state */
	for (i = 0U; i < PLAT_SPM_COND_MAX; i++) {
		spm_cond_t.table_cg[i] = 0U;

		/* check mtcmos, if off set idle_value and clk to 0 disable */
		if (IS_MT_SPM_PWR_OFF(idle_cg_info[i].subsys_mask)) {
			continue;
		}
		/* check clkmux */
		if (check_clkmux_pdn(idle_cg_info[i].clkmux_id)) {
			continue;
		}
		spm_cond_t.table_cg[i] = idle_cg_info[i].bBitflip ?
					 ~mmio_read_32(idle_cg_info[i].addr) :
					 mmio_read_32(idle_cg_info[i].addr);
	}

	spm_cond_t.table_pll = 0U;
	for (i = 0U; i < ARRAY_SIZE(plls); i++) {
		if ((mmio_read_32(plls[i].en_reg) & BIT(9)) != 0U) {
			spm_cond_t.table_pll |= plls[i].pll_b;
		}
	}

	spm_cond_t.priv = priv;
	for (i = 0U, _con = con; (*_con != NULL) && (i < num); _con++, i++) {
		if ((*_con)->update == NULL) {
			continue;
		}
		res = (*_con)->update(stateid, PLAT_RC_UPDATE_CONDITION,
				      (void const *)&spm_cond_t);
		if (res != MT_RM_STATUS_OK) {
			break;
		}
	}

	return 0;
}
