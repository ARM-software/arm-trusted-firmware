/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>

#include "lib/xlat_mpu/xlat_mpu.h"
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include "xlat_mpu_private.h"

#include <fvp_r_arch_helpers.h>
#include <platform_def.h>

#warning "xlat_mpu library is currently experimental and its API may change in future."


/*
 * MMU configuration register values for the active translation context. Used
 * from the MMU assembly helpers.
 */
uint64_t mmu_cfg_params[MMU_CFG_PARAM_MAX];

/*
 * Allocate and initialise the default translation context for the BL image
 * currently executing.
 */
REGISTER_XLAT_CONTEXT(tf, MAX_MMAP_REGIONS, MAX_XLAT_TABLES,
			PLAT_VIRT_ADDR_SPACE_SIZE, PLAT_PHY_ADDR_SPACE_SIZE);

void mmap_add(const mmap_region_t *mm)
{
	mmap_add_ctx(&tf_xlat_ctx, mm);
}

void __init init_xlat_tables(void)
{
	assert(tf_xlat_ctx.xlat_regime == EL_REGIME_INVALID);

	unsigned int current_el = xlat_arch_current_el();

	if (current_el == 1U) {
		tf_xlat_ctx.xlat_regime = EL1_EL0_REGIME;
	} else {
		assert(current_el == 2U);
		tf_xlat_ctx.xlat_regime = EL2_REGIME;
	}
	/* Note:  If EL3 is supported in future v8-R64, add EL3 assignment */
	init_xlat_tables_ctx(&tf_xlat_ctx);
}

int xlat_get_mem_attributes(uintptr_t base_va, uint32_t *attr)
{
	return xlat_get_mem_attributes_ctx(&tf_xlat_ctx, base_va, attr);
}

void enable_mpu_el2(unsigned int flags)
{
	/* EL2 is strictly MPU on v8-R64, so no need for setup_mpu_cfg() */
	enable_mpu_direct_el2(flags);
}
