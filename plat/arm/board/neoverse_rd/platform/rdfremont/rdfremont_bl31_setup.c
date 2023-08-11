/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/gic600_multichip.h>
#include <drivers/arm/smmu_v3.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <nrd_plat.h>
#include <nrd_variant.h>

#if (NRD_PLATFORM_VARIANT == 2)
static const mmap_region_t rdfremontmc_dynamic_mmap[] = {
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

static struct gic600_multichip_data rdfremontmc_multichip_data __init = {
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

static uintptr_t rdfremontmc_multichip_gicr_frames[] = {
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
	 * control block on RD-Fremont platforms. This SMMUv3 initialization is
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
		INFO("Enabling multi-chip support for RD-Fremont variant\n");

		for (i = 0; i < ARRAY_SIZE(rdfremontmc_dynamic_mmap); i++) {
			ret = mmap_add_dynamic_region(
					rdfremontmc_dynamic_mmap[i].base_pa,
					rdfremontmc_dynamic_mmap[i].base_va,
					rdfremontmc_dynamic_mmap[i].size,
					rdfremontmc_dynamic_mmap[i].attr);
			if (ret != 0) {
				ERROR("Failed to add entry i: %d (ret=%d)\n",
				       i, ret);
				panic();
			}
		}

		plat_arm_override_gicr_frames(
			rdfremontmc_multichip_gicr_frames);
		gic600_multichip_init(&rdfremontmc_multichip_data);
	}
#endif /* NRD_PLATFORM_VARIANT == 2 */
	nrd_bl31_common_platform_setup();
}
