/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <drivers/pmic/pmic_set_lowpower.h>
#include <drivers/spmi/spmi_common.h>
#include <lib/mtk_init/mtk_init.h>
#include <mt_spm_constraint.h>
#include <mt_spm_pmic_lp.h>
#include <mt_spm_reg.h>

/* SPMI_M: 6363, SPMI_P: 6373/6316 */
/* SPMI_M: 6363, SPMI_P: 6373/6316 */
static struct lp_pmic_cfg {
	uint8_t slave;
	uint8_t master;
	char *name;
} pmics[] = {
	[LP_MT6363] = {MT6363_SLAVE, SPMI_MASTER_1, "MT6363"},
	[LP_MT6373] = {MT6373_SLAVE, SPMI_MASTER_P_1, "MT6373"},
	[LP_MT6316_1] = {MT6316_S8_SLAVE, SPMI_MASTER_P_1, "MT6316_S8"},
	[LP_MT6316_2] = {MT6316_S6_SLAVE, SPMI_MASTER_P_1, "MT6316_S6"},
	[LP_MT6316_3] = {MT6316_S7_SLAVE, SPMI_MASTER_P_1, "MT6316_S7"},
	[LP_MT6316_4] = {MT6316_S15_SLAVE, SPMI_MASTER_P_1, "MT6316_S15"},
};

#ifdef MTK_SPM_PMIC_GS_DUMP
static struct pmic_gs_info pmic_gs_dump_info;
static char *pmic_str[] = {
	[LP_MT6363] = "MT6363",
	[LP_MT6373] = "MT6373",
	[LP_MT6316_1] = "MT6316_1",
	[LP_MT6316_2] = "MT6316_2",
	[LP_MT6316_3] = "MT6316_3",
	[LP_MT6316_4] = "MT6316_4",
};
#endif
struct spmi_device *lp_sdev[LP_PMIC_SLAVE_NUM];

#define VSRAM_CORE_0_35V		56
#define VSRAM_CORE_0_55V		88
#define VSRAM_CORE_0_75V		120

/* VSRAM_CORE Low bound */
#ifdef MTK_AGING_FLAVOR_LOAD
#define SUSPEND_AGING_VAL_SHIFT		3
#define SUSPEND_AGING_VAL_DEFAULT	85
#define VSRAM_CORE_LOWBOUND		(74 - SUSPEND_AGING_VAL_SHIFT)
#else
#define VSRAM_CORE_LOWBOUND		74
#endif
#define MT6363_LP_REG			0x1687
#define MT6363_VIO18_SWITCH		0x53
#define MT6373_VIO18_SWITCH		0x58

static uint32_t vcore_sram_suspend_vol = VSRAM_CORE_0_55V;
static bool vcore_sram_lp_enable = true;
static bool vcore_lp_enable = true;

static uint32_t get_vcore_sram_suspend_vol(void)
{
	uint8_t value;
	/* Set vcore_sram to 0.55V by default */
	value = VSRAM_CORE_0_55V;

#ifdef MTK_AGING_FLAVOR_LOAD
	value -= SUSPEND_AGING_VAL_SHIFT;
	if (value > SUSPEND_AGING_VAL_DEFAULT)
		value = SUSPEND_AGING_VAL_DEFAULT;
	INFO("%s(%d) enable aging with value: %u\n",
	     __func__, __LINE__, value);
#endif
	return value;
}

#ifdef MTK_SPM_PMIC_GS_DUMP
static void mt_spm_dump_pmic_gs(uint32_t cmd)
{
#ifdef MTK_SPM_PMIC_GS_DUMP_SUSPEND
	if (cmd & MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND) { /* Suspend enter */
		mt_spm_pmic_gs_dump(SUSPEND, LP_MT6363);
		mt_spm_pmic_gs_dump(SUSPEND, LP_MT6373);
	}
#endif
	if (cmd & MT_RM_CONSTRAINT_ALLOW_VCORE_LP) {
		if (cmd & MT_RM_CONSTRAINT_ALLOW_BUS26M_OFF) { /* SODI3 */
#ifdef MTK_SPM_PMIC_GS_DUMP_SODI3
			mt_spm_pmic_gs_dump(SODI3, LP_MT6363);
			mt_spm_pmic_gs_dump(SODI3, LP_MT6373);
#endif
		} else { /* DPIDLE enter */
#ifdef MTK_SPM_PMIC_GS_DUMP_DPIDLE
			mt_spm_pmic_gs_dump(DPIDLE, LP_MT6363);
			mt_spm_pmic_gs_dump(DPIDLE, LP_MT6373);
#endif
		}
	}
}
#endif

int spm_lp_setting_init(void)
{
	uint8_t i, slvid, spmi_master;

	for (i = 0; i < ARRAY_SIZE(pmics); i++) {
		slvid = pmics[i].slave;
		spmi_master = pmics[i].master;
		lp_sdev[i] = get_spmi_device(spmi_master, slvid);
		if (!lp_sdev[i])
			return -ENODEV;
	}

	PMIC_SLVID_BUCK_SET_LP(MT6316, S8, VBUCK1, RC8,
			       false, OP_MODE_LP, HW_OFF);

	/* Get suspend case vcore_sram sleep voltage */
	vcore_sram_suspend_vol = get_vcore_sram_suspend_vol();

#ifdef MTK_SPM_PMIC_GS_DUMP
	pmic_gs_dump_info.lp_sdev = lp_sdev;
	pmic_gs_dump_info.pmic_str = pmic_str;
	pmic_gs_dump_info.pmic_num = LP_PMIC_SLAVE_NUM;
#ifdef MTK_SPM_PMIC_GS_DUMP_SUSPEND
	pmic_gs_dump_info.scen_gs[SUSPEND].data = pmic_gs_suspend;
#endif
#ifdef MTK_SPM_PMIC_GS_DUMP_SODI3
	pmic_gs_dump_info.scen_gs[SODI3].data = pmic_gs_sodi3;
#endif
#ifdef MTK_SPM_PMIC_GS_DUMP_DPIDLE
	pmic_gs_dump_info.scen_gs[DPIDLE].data = pmic_gs_dpidle;
#endif
	return register_pmic_gs_info(&pmic_gs_dump_info);
#else
	return 0;
#endif
}

#ifdef MTK_SPM_PMIC_LP_SUPPORT
MTK_PLAT_SETUP_0_INIT(spm_lp_setting_init);
#endif

void set_vcore_lp_enable(bool enable)
{
	vcore_lp_enable = enable;
}

bool get_vcore_lp_enable(void)
{
	return vcore_lp_enable;
}

void set_vsram_lp_enable(bool enable)
{
	vcore_sram_lp_enable = enable;
}

bool get_vsram_lp_enable(void)
{
	return vcore_sram_lp_enable;
}

void set_vsram_lp_volt(uint32_t volt)
{
	/* Allow 0.35V ~ 0.75V */
	if (volt < VSRAM_CORE_0_35V || volt > VSRAM_CORE_0_75V)
		return;
	vcore_sram_suspend_vol = volt;
}

uint32_t get_vsram_lp_volt(void)
{
	return vcore_sram_suspend_vol;
}

static int pmic_lp_setting(uint8_t data)
{
	int ret;

	if (data != 0x0 && data != 0x1)
		return -ENODEV;
	/* SUSPEND:VS1 HW2&RC9 normal mode */
	PMIC_BUCK_SET_LP(MT6363, VS1, HW2,
			 false, OP_MODE_LP, HW_LP);
	PMIC_BUCK_SET_LP(MT6363, VS1, RC9,
			 false, OP_MODE_MU, HW_ON);
	PMIC_LDO_SET_LP(MT6363, VIO18, HW2,
			false, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6363, VIO18, RC9,
			false, OP_MODE_MU, HW_ON);

	/* Switch DIG18 to VIO18 for power saving */
	ret = spmi_ext_register_writel(lp_sdev[LP_MT6363], MT6363_VIO18_SWITCH, &data, 1);
	if (ret)
		INFO("MT6363 RG_VIO18 spmi failed\n");

	ret = spmi_ext_register_writel(lp_sdev[LP_MT6373], MT6373_VIO18_SWITCH, &data, 1);
	if (ret)
		INFO("MT6373 RG_VIO18 spmi failed\n");
	return ret;
}

int do_spm_low_power(enum SPM_PWR_TYPE type, uint32_t cmd)
{
	int ret;
	uint8_t value;
	bool enter_suspend, vcore_sram_lp = false;

	if (type == SPM_LP_ENTER) {
		enter_suspend = true;
		vcore_sram_lp = vcore_sram_lp_enable;

		if (cmd & MT_RM_CONSTRAINT_ALLOW_AP_PLAT_SUSPEND) {
			value = vcore_sram_suspend_vol;
			if (value < VSRAM_CORE_LOWBOUND ||
				value > VSRAM_CORE_0_75V) {
				INFO("Vsram_core voltage wrong\n");
				panic();
			}
		} else {
			value = VSRAM_CORE_0_55V;
		}
	} else {
		enter_suspend = false;
		vcore_sram_lp = false;
		value = VSRAM_CORE_0_75V;
	}
	/* Enable VA12_2/VSRAM_CPUL HW_LP for suspend/idle */
	PMIC_LDO_SET_LP(MT6363, VA12_2, HW2, enter_suspend, OP_MODE_LP, HW_LP);
	PMIC_LDO_SET_LP(MT6363, VSRAM_CPUL, HW2,
			enter_suspend, OP_MODE_LP, HW_LP);

	if (!(cmd & MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND))
		return 0;
	PMIC_SLVID_BUCK_SET_LP(MT6316, S8, VBUCK1, HW2, vcore_lp_enable,
			       OP_MODE_LP, HW_LP);
	PMIC_BUCK_SET_LP(MT6363, VBUCK4, HW0, vcore_sram_lp,
			 OP_MODE_LP, HW_LP);
	PMIC_BUCK_SET_LP(MT6363, VBUCK4, HW2, !enter_suspend,
			 OP_MODE_LP, HW_LP);
	ret = spmi_ext_register_writel(lp_sdev[LP_MT6363], MT6363_LP_REG, &value, 1);
	if (ret)
		INFO("BUCK(VSRAM_CORE) spmi write failed\n");

	if (cmd & MT_RM_CONSTRAINT_ALLOW_AP_PLAT_SUSPEND)
		pmic_lp_setting(enter_suspend ? 1 : 0);

	return 0;
}
