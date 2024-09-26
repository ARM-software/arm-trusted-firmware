/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/gic600_multichip.h>
#include <drivers/arm/rse_comms.h>
#include <drivers/arm/smmu_v3.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <nrd_plat.h>
#include <nrd_variant.h>
#include <rdv3_rse_comms.h>

#if (NRD_PLATFORM_VARIANT == 2)
static const mmap_region_t rdv3mc_dynamic_mmap[] = {
#if NRD_CHIP_COUNT > 1
	NRD_CSS_SHARED_RAM_MMAP(1),
	NRD_CSS_PERIPH_MMAP(1),
#endif
#if NRD_CHIP_COUNT > 2
	NRD_CSS_SHARED_RAM_MMAP(2),
	NRD_CSS_PERIPH_MMAP(2),
#endif
#if NRD_CHIP_COUNT > 3
	NRD_CSS_SHARED_RAM_MMAP(3),
	NRD_CSS_PERIPH_MMAP(3),
#endif
};

static struct gic600_multichip_data rdv3mc_multichip_data __init = {
	.rt_owner_base = PLAT_ARM_GICD_BASE,
	.rt_owner = 0,
	.chip_count = NRD_CHIP_COUNT,
	.chip_addrs = {
		PLAT_ARM_GICD_BASE >> 16,
#if NRD_CHIP_COUNT > 1
		(PLAT_ARM_GICD_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(1)) >> 16,
#endif
#if NRD_CHIP_COUNT > 2
		(PLAT_ARM_GICD_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(2)) >> 16,
#endif
#if NRD_CHIP_COUNT > 3
		(PLAT_ARM_GICD_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(3)) >> 16,
#endif
	},
	.spi_ids = {
		{PLAT_ARM_GICD_BASE, 32, 511},
#if NRD_CHIP_COUNT > 1
		{PLAT_ARM_GICD_BASE, 512, 991},
#endif
#if NRD_CHIP_COUNT > 2
		{PLAT_ARM_GICD_BASE, 4096, 4575},
#endif
#if NRD_CHIP_COUNT > 3
		{PLAT_ARM_GICD_BASE, 4576, 5055},
#endif
	}
};

static uintptr_t rdv3mc_multichip_gicr_frames[] = {
	/* Chip 0's GICR Base */
	PLAT_ARM_GICR_BASE,
#if NRD_CHIP_COUNT > 1
	/* Chip 1's GICR BASE */
	PLAT_ARM_GICR_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(1),
#endif
#if NRD_CHIP_COUNT > 2
	/* Chip 2's GICR BASE */
	PLAT_ARM_GICR_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(2),
#endif
#if NRD_CHIP_COUNT > 3
	/* Chip 3's GICR BASE */
	PLAT_ARM_GICR_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(3),
#endif
	UL(0)	/* Zero Termination */
};
#endif /* NRD_PLATFORM_VARIANT == 2 */

void bl31_platform_setup(void)
{
	/*
	 * Perform SMMUv3 GPT configuration for the GPC SMMU present in system
	 * control block on RD-V3 platforms. This SMMUv3 initialization is
	 * not fatal.
	 *
	 * Don't perform smmuv3_security_init() for this instance of SMMUv3 as
	 * the global aborts need not be configured to allow the components in
	 * system control block send transations downstream to SMMUv3.
	 */
	if (smmuv3_init(NRD_CSS_GPC_SMMUV3_BASE) != 0) {
		WARN("Failed initializing System SMMU.\n");
	}

#if (NRD_PLATFORM_VARIANT == 2)
	int ret;
	unsigned int i;

	if (plat_arm_nrd_get_multi_chip_mode() == 0) {
		ERROR("Chip Count is %u but multi-chip mode is not enabled\n",
		       NRD_CHIP_COUNT);
		panic();
	} else {
		INFO("Enabling multi-chip support for RD-V3 variant\n");

		for (i = 0; i < ARRAY_SIZE(rdv3mc_dynamic_mmap); i++) {
			ret = mmap_add_dynamic_region(
					rdv3mc_dynamic_mmap[i].base_pa,
					rdv3mc_dynamic_mmap[i].base_va,
					rdv3mc_dynamic_mmap[i].size,
					rdv3mc_dynamic_mmap[i].attr);
			if (ret != 0) {
				ERROR("Failed to add entry i: %d (ret=%d)\n",
				       i, ret);
				panic();
			}
		}

		plat_arm_override_gicr_frames(
			rdv3mc_multichip_gicr_frames);
		gic600_multichip_init(&rdv3mc_multichip_data);
	}
#endif /* NRD_PLATFORM_VARIANT == 2 */
	nrd_bl31_common_platform_setup();

	if (plat_rse_comms_init() != 0) {
		WARN("Failed initializing AP-RSE comms.\n");
	}
}

#if RESET_TO_BL31
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

#endif /* RESET_TO_BL31 */
