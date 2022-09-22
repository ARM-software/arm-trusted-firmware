/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/mtk_init/mtk_init.h>
#include <mtk_dcm.h>
#include <mtk_dcm_utils.h>

static void dcm_armcore(bool mode)
{
	dcm_mp_cpusys_top_bus_pll_div_dcm(mode);
	dcm_mp_cpusys_top_cpu_pll_div_0_dcm(mode);
	dcm_mp_cpusys_top_cpu_pll_div_1_dcm(mode);
}

static void dcm_mcusys(bool on)
{
	dcm_mp_cpusys_top_adb_dcm(on);
	dcm_mp_cpusys_top_apb_dcm(on);
	dcm_mp_cpusys_top_cpubiu_dcm(on);
	dcm_mp_cpusys_top_misc_dcm(on);
	dcm_mp_cpusys_top_mp0_qdcm(on);

	/* CPCCFG_REG */
	dcm_cpccfg_reg_emi_wfifo(on);
	dcm_mp_cpusys_top_last_cor_idle_dcm(on);
}

static void dcm_stall(bool on)
{
	dcm_mp_cpusys_top_core_stall_dcm(on);
	dcm_mp_cpusys_top_fcm_stall_dcm(on);
}

static bool check_dcm_state(void)
{
	bool ret = true;

	ret &= dcm_mp_cpusys_top_bus_pll_div_dcm_is_on();
	ret &= dcm_mp_cpusys_top_cpu_pll_div_0_dcm_is_on();
	ret &= dcm_mp_cpusys_top_cpu_pll_div_1_dcm_is_on();

	ret &= dcm_mp_cpusys_top_adb_dcm_is_on();
	ret &= dcm_mp_cpusys_top_apb_dcm_is_on();
	ret &= dcm_mp_cpusys_top_cpubiu_dcm_is_on();
	ret &= dcm_mp_cpusys_top_misc_dcm_is_on();
	ret &= dcm_mp_cpusys_top_mp0_qdcm_is_on();
	ret &= dcm_cpccfg_reg_emi_wfifo_is_on();
	ret &= dcm_mp_cpusys_top_last_cor_idle_dcm_is_on();

	ret &= dcm_mp_cpusys_top_core_stall_dcm_is_on();
	ret &= dcm_mp_cpusys_top_fcm_stall_dcm_is_on();

	return ret;
}

bool dcm_check_state(uintptr_t addr, unsigned int mask, unsigned int compare)
{
	return ((mmio_read_32(addr) & mask) == compare);
}

int dcm_set_init(void)
{
	int ret;

	dcm_armcore(true);
	dcm_mcusys(true);
	dcm_stall(true);

	if (check_dcm_state() == false) {
		ERROR("Failed to set default dcm on!!\n");
		ret = -1;
	} else {
		INFO("%s, dcm pass\n", __func__);
		ret = 0;
	}

	return ret;
}
MTK_PLAT_SETUP_0_INIT(dcm_set_init);
