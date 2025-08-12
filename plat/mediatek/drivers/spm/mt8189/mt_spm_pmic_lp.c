/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <drivers/pmic/mt6359p_set_lowpower.h>
#include <drivers/spmi/spmi_common.h>
#include <lib/mtk_init/mtk_init.h>
#include <mt_spm_constraint.h>
#include <mt_spm_pmic_lp.h>
#include <pmic_wrap_init_common.h>

#ifdef MTK_SPM_PMIC_GS_DUMP
static struct pmic_gs_info pmic_gs_dump_info;
static char *pmic_str[] = {
	[LP_MT6365] = "MT6365",
	[LP_MT6319_S7] = "MT6319",
};
#endif

struct spmi_device *lp_sdev[LP_PMIC_SLAVE_NUM];

#define VCORE_BASE_UV			40000
#define VCORE_TO_PMIC_VAL(volt)		(((volt) - VCORE_BASE_UV + 625 - 1) / 625)
#define PMIC_VAL_TO_VCORE(pmic)		(((pmic) * 625) + VCORE_BASE_UV)

#define VCORE_0_45V			VCORE_TO_PMIC_VAL(45000)
#define VCORE_0_55V			VCORE_TO_PMIC_VAL(55000)
#define VCORE_0_75V			VCORE_TO_PMIC_VAL(75000)

#define VSRAM_CORE_BASE_UV		40000
#define VSRAM_CORE_TO_PMIC_VAL(volt)	(((volt) - VSRAM_CORE_BASE_UV + 625 - 1) / 625)
#define PMIC_VAL_TO_VSRAM_CORE(pmic)	(((pmic) * 625) + VSRAM_CORE_BASE_UV)

#define VSRAM_CORE_0_40V		VSRAM_CORE_TO_PMIC_VAL(40000)
#define VSRAM_CORE_0_55V		VSRAM_CORE_TO_PMIC_VAL(55000)
#define VSRAM_CORE_0_75V		VSRAM_CORE_TO_PMIC_VAL(75000)

static uint32_t vcore_sram_suspend_vol = VSRAM_CORE_0_55V;
static uint32_t vcore_suspend_vol = VCORE_0_55V;
static bool vcore_sram_lp_enable = true;
static bool vcore_lp_enable = true;

static uint32_t get_vcore_sram_suspend_vol(void)
{
	uint8_t value = VSRAM_CORE_0_55V;
#ifdef MTK_SUSPEND_VOL_BIN_SUPPORT
	static const uint32_t vlp_vb_efuse_val_arr[] = { 24, 24, 24, 24, 24, 24,
							 22, 21, 20, 19, 18, 17,
							 16, 15, 14, 13 };
	uint32_t volbin_efuse_reg;

	volbin_efuse_reg = mmio_read_32(SOC_VB_REG);
	INFO("11#@# %s(%d) enable voltage bin support with efuse value: 0x%x\n",
	     __func__, __LINE__, volbin_efuse_reg);
	volbin_efuse_reg = (volbin_efuse_reg >> SUSPEND_VB_USE_SHIFT) &
			   SUSPEND_VB_USE_MASK;

	if (volbin_efuse_reg < ARRAY_SIZE(vlp_vb_efuse_val_arr))
		value = vlp_vb_efuse_val_arr[volbin_efuse_reg];
#endif
#ifdef MTK_AGING_FLAVOR_LOAD
	value -= SUSPEND_AGING_VAL_SHIFT;
	if (value > SUSPEND_AGING_VAL_DEFAULT)
		value = SUSPEND_AGING_VAL_DEFAULT;
	INFO("11#@# %s(%d) enable aging with value: %d\n", __func__, __LINE__,
	     value);
#endif
#ifdef MTK_AGING_HV_FLAVOR_LOAD
	/* ignore binning and configure as sign off + 5% */
	value = SUSPEND_AGING_HV_VAL_DEFAULT;
	INFO("11#@# %s(%d) enable aging hv with value: %d\n", __func__,
	     __LINE__, value);
#endif
	return value;
}

#ifdef MTK_SPM_PMIC_GS_DUMP
static void mt_spm_dump_pmic_gs(uint32_t cmd)
{
#ifdef MTK_SPM_PMIC_GS_DUMP_SUSPEND
	if (cmd & MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND) { /*Suspend enter*/
		mt_spm_pmic_gs_dump(SUSPEND, LP_MT6365);
	}
#endif
	if (cmd & MT_RM_CONSTRAINT_ALLOW_VCORE_LP) {
		if (cmd & MT_RM_CONSTRAINT_ALLOW_BUS26M_OFF) { /*SODI3 enter*/
#ifdef MTK_SPM_PMIC_GS_DUMP_SODI3
			mt_spm_pmic_gs_dump(SODI3, LP_MT6365);
#endif
		} else { /*DPIDLE enter*/
#ifdef MTK_SPM_PMIC_GS_DUMP_DPIDLE
			mt_spm_pmic_gs_dump(DPIDLE, LP_MT6365);
#endif
		}
	}
}
#endif

int spm_lp_setting_init(void)
{
	/* AOC2.5: VCORE O0 OFF */
	PMIC_BUCK_SET_LP(MT6359P, VPROC2, HW0, true, OP_MODE_LP, HW_OFF);

	/* AOC2.5 : VSRAM_CORE O0 HW:LP */
	PMIC_BUCK_SET_LP(MT6359P, VGPU11, HW0, true, OP_MODE_LP, HW_LP);

	/* Get suspend case VSRAM_CORE sleep voltage */
	vcore_sram_suspend_vol = get_vcore_sram_suspend_vol();
	/* Set VCORE LP default 0.55V */
	vcore_suspend_vol = VCORE_0_55V;

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

MTK_PLAT_SETUP_0_INIT(spm_lp_setting_init);

void set_vcore_lp_enable(bool enable)
{
	vcore_lp_enable = enable;
}

bool get_vcore_lp_enable(void)
{
	return vcore_lp_enable;
}

void set_vcore_lp_volt(uint32_t volt)
{
	/* allow 0.45V ~ 0.75V */
	if (volt < VCORE_0_45V)
		return;
	if (volt > VCORE_0_75V)
		return;
	vcore_suspend_vol = volt;
}

uint32_t get_vcore_lp_volt(void)
{
	return vcore_suspend_vol;
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
	/* allow 0.40V ~ 0.75V */
	if (volt > VSRAM_CORE_0_75V)
		return;
	vcore_sram_suspend_vol = volt;
}

uint32_t get_vsram_lp_volt(void)
{
	return vcore_sram_suspend_vol;
}

int do_spm_low_power(enum SPM_PWR_TYPE type, uint32_t cmd)
{
	int ret = 0;
	uint8_t value = 0;
	uint32_t reg = 0;
	bool vproc2_lp_enable = false;
	bool vgpu11_hw0_lp_enable = false;
	bool vgpu11_hw2_lp_enable = false;

	if (!(cmd & MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND))
		return 0;

	if (type == SPM_LP_ENTER) {
		/* SUSPEND enter */
		vproc2_lp_enable = vcore_lp_enable;
		vgpu11_hw0_lp_enable = vcore_sram_lp_enable;
		vgpu11_hw2_lp_enable = false;
		reg = 0x158E; /* LDO_VSRAM_CORE_VOSEL0 */

		if (cmd & MT_RM_CONSTRAINT_ALLOW_AP_PLAT_SUSPEND)
			value = vcore_sram_suspend_vol; /* SUSPEND: VSRAM_CORE 0.55V */
		else
			value = VSRAM_CORE_0_55V;       /* SODI5.0: VSRAM_CORE 0.55V */
	} else {
		/* SUSPEND leave (SODI3/DPIDLE) */
		vproc2_lp_enable = true;
		vgpu11_hw0_lp_enable = true;
		vgpu11_hw2_lp_enable = true;
		reg = 0x15b4; /* LDO_VSRAM_CORE_VOSEL0 */
		value = VSRAM_CORE_0_75V; /* 0.75V SODI3/DPIDLE */
	}

	PMIC_BUCK_SET_LP(MT6359P, VPROC2, HW0, vproc2_lp_enable, OP_MODE_LP, HW_OFF);
	PMIC_BUCK_SET_LP(MT6359P, VGPU11, HW0, vgpu11_hw0_lp_enable, OP_MODE_LP, HW_LP);
	PMIC_BUCK_SET_LP(MT6359P, VGPU11, HW2, vgpu11_hw2_lp_enable, OP_MODE_LP, HW_LP);

	ret = pwrap_write_field(reg, value, 0xff, 0);
	if (ret) {
		if (type == SPM_LP_ENTER)
			INFO("LDO_VSRAM_CORE_VOSEL0(VSRAM_CORE) spmi write failed\n");
		else
			INFO("BUCK_VBUCK4_CON1(VSRAM_CORE) spmi write failed\n");
	}

#ifdef MTK_SPM_PMIC_GS_DUMP
	if (type == SPM_LP_ENTER)
		mt_spm_dump_pmic_gs(cmd);
#endif

	return 0;
}
