/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/gic600_multichip.h>
#include <drivers/arm/rse_comms.h>
#include <drivers/arm/smmu_v3.h>
#include <lib/per_cpu/per_cpu.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <nrd_css_fw_def3.h>
#include <nrd_plat.h>
#include <nrd_variant.h>
#include <rdv3_rse_comms.h>

#define RT_OWNER 0

/*
 * Base addr of the frame that allocated by the platform
 * intended for remote gic to local gic interrupt
 * message communication
 */
#define NRD_RGIC2LGIC_MESSREG_HNI_BASE		UL(0x5FFF0000)

#if (PLATFORM_NODE_COUNT > 1)
/*
 * NUMA node related information for a platform could be populated in by any
 * means. This could come in via device tree, transfer list or could even be
 * hardcoded. For rdv3cfg2, this is statically defined at the moment.
 */
const uintptr_t per_cpu_nodes_base[] = {
	(uintptr_t)PER_CPU_START,
	(uintptr_t)NRD_REMOTE_CHIP_MEM_OFFSET(1),
	(uintptr_t)NRD_REMOTE_CHIP_MEM_OFFSET(2),
	(uintptr_t)NRD_REMOTE_CHIP_MEM_OFFSET(3)

};
#endif

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
	.base_addrs = {
		PLAT_ARM_GICD_BASE,
#if NRD_CHIP_COUNT > 1
		PLAT_ARM_GICD_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(1),
#endif
#if NRD_CHIP_COUNT > 2
		PLAT_ARM_GICD_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(2),
#endif
#if NRD_CHIP_COUNT > 3
		PLAT_ARM_GICD_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(3),
#endif
	},
	.rt_owner = RT_OWNER,
	.chip_count = NRD_CHIP_COUNT,
	.chip_addrs = {
		{
			NRD_RGIC2LGIC_MESSREG_HNI_BASE >> 16,
			(NRD_RGIC2LGIC_MESSREG_HNI_BASE
				+ NRD_REMOTE_CHIP_MEM_OFFSET(1)) >> 16,
			(NRD_RGIC2LGIC_MESSREG_HNI_BASE
				+ NRD_REMOTE_CHIP_MEM_OFFSET(2)) >> 16,
			(NRD_RGIC2LGIC_MESSREG_HNI_BASE
				+ NRD_REMOTE_CHIP_MEM_OFFSET(3)) >> 16,
		},
#if NRD_CHIP_COUNT > 1
		{
			NRD_RGIC2LGIC_MESSREG_HNI_BASE >> 16,
			(NRD_RGIC2LGIC_MESSREG_HNI_BASE
				+ NRD_REMOTE_CHIP_MEM_OFFSET(1)) >> 16,
			(NRD_RGIC2LGIC_MESSREG_HNI_BASE
				+ NRD_REMOTE_CHIP_MEM_OFFSET(2)) >> 16,
			(NRD_RGIC2LGIC_MESSREG_HNI_BASE
				+ NRD_REMOTE_CHIP_MEM_OFFSET(3)) >> 16,
		},
#endif
#if NRD_CHIP_COUNT > 2
		{
			NRD_RGIC2LGIC_MESSREG_HNI_BASE >> 16,
			(NRD_RGIC2LGIC_MESSREG_HNI_BASE
				+ NRD_REMOTE_CHIP_MEM_OFFSET(1)) >> 16,
			(NRD_RGIC2LGIC_MESSREG_HNI_BASE
				+ NRD_REMOTE_CHIP_MEM_OFFSET(2)) >> 16,
			(NRD_RGIC2LGIC_MESSREG_HNI_BASE
				+ NRD_REMOTE_CHIP_MEM_OFFSET(3)) >> 16,
		},
#endif
#if NRD_CHIP_COUNT > 3
		{
			NRD_RGIC2LGIC_MESSREG_HNI_BASE >> 16,
			(NRD_RGIC2LGIC_MESSREG_HNI_BASE
				+ NRD_REMOTE_CHIP_MEM_OFFSET(1)) >> 16,
			(NRD_RGIC2LGIC_MESSREG_HNI_BASE
				+ NRD_REMOTE_CHIP_MEM_OFFSET(2)) >> 16,
			(NRD_RGIC2LGIC_MESSREG_HNI_BASE
				+ NRD_REMOTE_CHIP_MEM_OFFSET(3)) >> 16,
		}
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
#endif /* NRD_PLATFORM_VARIANT == 2 */

static uintptr_t rdv3mc_multichip_gicr_frames[] = {
	/* Chip 0's GICR Base */
	PLAT_ARM_GICR_BASE,
#if (NRD_PLATFORM_VARIANT == 2)
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
#endif /* NRD_PLATFORM_VARIANT == 2 */
	UL(0)	/* Zero Termination */
};

#if (NRD_PLATFORM_VARIANT == 2)
void __init bl31_plat_arch_setup(void)
{
#if (PLATFORM_NODE_COUNT > 1)
	int ret;
#endif
	arm_bl31_plat_arch_setup();
#if (PLATFORM_NODE_COUNT > 1)
	/* Add mmap for all remote chips */
	for (int i = 1; i < ARRAY_SIZE(per_cpu_nodes_base); i++) {
		ret = mmap_add_dynamic_region(
		NRD_REMOTE_CHIP_MEM_OFFSET(i),
		NRD_REMOTE_CHIP_MEM_OFFSET(i),
		NRD_CSS_PAGE_ALIGN_CEIL((PER_CPU_END - PER_CPU_START)),
		MT_MEMORY | MT_RW | EL3_PAS);
		if (ret != 0) {
			ERROR("Failed to add per-cpu mmap (ret=%d)", ret);
			panic();
		}
	}
#endif
}
#endif

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

		gic600_multichip_init(&rdv3mc_multichip_data);
	}
#endif /* NRD_PLATFORM_VARIANT == 2 */
	nrd_bl31_common_platform_setup();

	gic_set_gicr_frames(
		rdv3mc_multichip_gicr_frames);

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
