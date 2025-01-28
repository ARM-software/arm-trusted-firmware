/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/mtk_init/mtk_init.h>
#include <mtk_dcm_utils.h>

static void dcm_infra(bool on)
{
	dcm_bcrm_apinfra_io_ctrl_ao_infra_bus_dcm(on);
	dcm_bcrm_apinfra_io_noc_ao_infra_bus_dcm(on);
	dcm_bcrm_apinfra_mem_ctrl_ao_infra_bus_dcm(on);
	dcm_bcrm_apinfra_mem_intf_noc_ao_infra_bus_dcm(on);
}

static void dcm_mcusys(bool on)
{
	dcm_mcusys_par_wrap_mcu_misc_dcm(on);
}

static void dcm_mcusys_acp(bool on)
{
	dcm_mcusys_par_wrap_mcu_acp_dcm(on);
}

static void dcm_mcusys_adb(bool on)
{
	dcm_mcusys_par_wrap_mcu_adb_dcm(on);
}

static void dcm_mcusys_apb(bool on)
{
	dcm_mcusys_par_wrap_mcu_apb_dcm(on);
}

static void dcm_mcusys_bus(bool on)
{
	dcm_mcusys_par_wrap_mcu_bus_qdcm(on);
}

static void dcm_mcusys_cbip(bool on)
{
	dcm_mcusys_par_wrap_mcu_cbip_dcm(on);
}

static void dcm_mcusys_chi_mon(bool on)
{
	dcm_mcusys_par_wrap_mcu_chi_mon_dcm(on);
}

static void dcm_mcusys_core(bool on)
{
	dcm_mcusys_par_wrap_mcu_core_qdcm(on);
}

static void dcm_mcusys_dsu_acp(bool on)
{
	dcm_mcusys_par_wrap_mcu_dsu_acp_dcm(on);
}

static void dcm_mcusys_ebg(bool on)
{
	dcm_mcusys_par_wrap_mcu_ebg_dcm(on);
}

static void dcm_mcusys_gic_spi(bool on)
{
	dcm_mcusys_par_wrap_mcu_gic_spi_dcm(on);
}

static void dcm_mcusys_io(bool on)
{
	dcm_mcusys_par_wrap_mcu_io_dcm(on);
}

static void dcm_mcusys_l3c(bool on)
{
	dcm_mcusys_par_wrap_mcu_l3c_dcm(on);
}

static void dcm_mcusys_stall(bool on)
{
	dcm_mcusys_par_wrap_mcu_stalldcm(on);
}

static void dcm_vlp(bool on)
{
	dcm_vlp_ao_bcrm_vlp_bus_dcm(on);
}

int mtk_dcm_init(void)
{
	dcm_infra(true);
	dcm_mcusys(true);
	dcm_mcusys_acp(true);
	dcm_mcusys_adb(true);
	dcm_mcusys_apb(true);
	dcm_mcusys_bus(true);
	dcm_mcusys_cbip(true);
	dcm_mcusys_chi_mon(true);
	dcm_mcusys_core(true);
	dcm_mcusys_dsu_acp(true);
	dcm_mcusys_ebg(true);
	dcm_mcusys_gic_spi(true);
	dcm_mcusys_io(true);
	dcm_mcusys_l3c(true);
	dcm_mcusys_stall(true);
	dcm_vlp(true);

	return 0;
}

MTK_PLAT_SETUP_0_INIT(mtk_dcm_init);
