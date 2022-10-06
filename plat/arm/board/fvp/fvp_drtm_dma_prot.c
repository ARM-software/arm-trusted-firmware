/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>

#include <drivers/arm/smmu_v3.h>
#include <lib/utils_def.h>
#include <plat/arm/common/arm_config.h>
#include <plat/common/platform.h>

#include <platform_def.h>

/**
 * Array mentioning number of SMMUs supported by FVP
 */
static const uintptr_t fvp_smmus[] = {
	PLAT_FVP_SMMUV3_BASE,
};

bool plat_has_non_host_platforms(void)
{
	/* FVP base platforms typically have GPU, as per FVP Reference guide */
	return true;
}

bool plat_has_unmanaged_dma_peripherals(void)
{
	/*
	 * FVP Reference guide does not show devices that are described as
	 * DMA-capable but not managed by an SMMU in the FVP documentation.
	 * However, the SMMU seems to have only been introduced in the RevC
	 * revision.
	 */
	return (arm_config.flags & ARM_CONFIG_FVP_HAS_SMMUV3) == 0;
}

unsigned int plat_get_total_smmus(void)
{
	if ((arm_config.flags & ARM_CONFIG_FVP_HAS_SMMUV3) != 0U) {
		return ARRAY_SIZE(fvp_smmus);
	} else {
		return 0;
	}
}

void plat_enumerate_smmus(const uintptr_t **smmus_out,
			  size_t *smmu_count_out)
{
	if ((arm_config.flags & ARM_CONFIG_FVP_HAS_SMMUV3) != 0U) {
		*smmus_out = fvp_smmus;
		*smmu_count_out = ARRAY_SIZE(fvp_smmus);
	} else {
		*smmus_out = NULL;
		*smmu_count_out = 0;
	}
}

/* DRTM DMA Protection Features */
static const plat_drtm_dma_prot_features_t dma_prot_features = {
	.max_num_mem_prot_regions = 0, /* No protection regions are present */
	.dma_protection_support = 0x1 /* Complete DMA protection only */
};

const plat_drtm_dma_prot_features_t *plat_drtm_get_dma_prot_features(void)
{
	return &dma_prot_features;
}

uint64_t plat_drtm_dma_prot_get_max_table_bytes(void)
{
	return 0U;
}
