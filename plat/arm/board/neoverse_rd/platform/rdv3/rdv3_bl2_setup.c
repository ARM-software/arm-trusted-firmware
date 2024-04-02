/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/gpt_rme/gpt_rme.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <nrd_plat.h>

/*
 * The GPT library might modify the gpt regions structure to optimize
 * the layout, so the array cannot be constant.
 */
static pas_region_t pas_regions[] = {
	NRD_PAS_SHARED_SRAM,
	NRD_PAS_SYSTEM_NCI,
	NRD_PAS_DEBUG_NIC,
	NRD_PAS_NS_UART,
	NRD_PAS_REALM_UART,
	NRD_PAS_AP_NS_WDOG,
	NRD_PAS_AP_ROOT_WDOG,
	NRD_PAS_AP_SECURE_WDOG,
	NRD_PAS_SECURE_SRAM_ERB_AP,
	NRD_PAS_NS_SRAM_ERB_AP,
	NRD_PAS_ROOT_SRAM_ERB_AP,
	NRD_PAS_REALM_SRAM_ERB_AP,
	NRD_PAS_SECURE_SRAM_ERB_SCP,
	NRD_PAS_NS_SRAM_ERB_SCP,
	NRD_PAS_ROOT_SRAM_ERB_SCP,
	NRD_PAS_REALM_SRAM_ERB_SCP,
	NRD_PAS_SECURE_SRAM_ERB_MCP,
	NRD_PAS_NS_SRAM_ERB_MCP,
	NRD_PAS_ROOT_SRAM_ERB_MCP,
	NRD_PAS_REALM_SRAM_ERB_MCP,
	NRD_PAS_SECURE_SRAM_ERB_RSE,
	NRD_PAS_NS_SRAM_ERB_RSE,
	NRD_PAS_ROOT_SRAM_ERB_RSE,
	NRD_PAS_REALM_SRAM_ERB_RSE,
	NRD_PAS_RSE_SECURE_SRAM_ERB_RSM,
	NRD_PAS_RSE_NS_SRAM_ERB_RSM,
	NRD_PAS_SCP_SECURE_SRAM_ERB_RSM,
	NRD_PAS_SCP_NS_SRAM_ERB_RSM,
	NRD_PAS_MCP_SECURE_SRAM_ERB_RSM,
	NRD_PAS_MCP_NS_SRAM_ERB_RSM,
	NRD_PAS_AP_SCP_ROOT_MHU,
	NRD_PAS_AP_MCP_NS_MHU,
	NRD_PAS_AP_MCP_SECURE_MHU,
	NRD_PAS_AP_MCP_ROOT_MHU,
	NRD_PAS_AP_RSE_NS_MHU,
	NRD_PAS_AP_RSE_SECURE_MHU,
	NRD_PAS_AP_RSE_ROOT_MHU,
	NRD_PAS_AP_RSE_REALM_MHU,
	NRD_PAS_SCP_MCP_RSE_CROSS_CHIP_MHU,
	NRD_PAS_SYNCNT_MSTUPDTVAL_ADDR,
	NRD_PAS_STM_SYSTEM_ITS,
	NRD_PAS_SCP_MCP_RSE_SHARED_SRAM,
	NRD_PAS_GIC,
	NRD_PAS_NS_DRAM,
#if SPD_spmd && SPMD_SPM_AT_SEL2
	NRD_PAS_BL32,
#endif
	NRD_PAS_RMM,
	NRD_PAS_L1GPT,
	NRD_PAS_CMN,
	NRD_PAS_LCP_PERIPHERAL,
	NRD_PAS_DDR_IO,
	NRD_PAS_SMMU_NCI_IO,
	NRD_PAS_DRAM2_CHIP0,
#if NRD_CHIP_COUNT > 1
	NRD_PAS_DRAM1_CHIP1,
	NRD_PAS_DRAM2_CHIP1,
#endif
#if NRD_CHIP_COUNT > 2
	NRD_PAS_DRAM1_CHIP2,
	NRD_PAS_DRAM2_CHIP2,
#endif
#if NRD_CHIP_COUNT > 3
	NRD_PAS_DRAM1_CHIP3,
	NRD_PAS_DRAM2_CHIP3
#endif
};

static const arm_gpt_info_t arm_gpt_info = {
	.pas_region_base  = pas_regions,
	.pas_region_count = (unsigned int)ARRAY_SIZE(pas_regions),
	.l0_base = (uintptr_t)ARM_L0_GPT_BASE,
	.l1_base = (uintptr_t)ARM_L1_GPT_BASE,
	.l0_size = (size_t)ARM_L0_GPT_SIZE,
	.l1_size = (size_t)ARM_L1_GPT_SIZE,
	.pps = GPCCR_PPS_256TB,
	.pgs = GPCCR_PGS_4K
};

const arm_gpt_info_t *plat_arm_get_gpt_info(void)
{
	return &arm_gpt_info;
}
