/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "../xlat_mpu_private.h"
#include <arch.h>
#include <arch_features.h>
#include <lib/cassert.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <fvp_r_arch_helpers.h>

#warning "xlat_mpu library is currently experimental and its API may change in future."

#if ENABLE_ASSERTIONS
/*
 * Return minimum virtual address space size supported by the architecture
 */
uintptr_t xlat_get_min_virt_addr_space_size(void)
{
	uintptr_t ret;

	if (is_armv8_4_ttst_present()) {
		ret = MIN_VIRT_ADDR_SPACE_SIZE_TTST;
	} else {
		ret = MIN_VIRT_ADDR_SPACE_SIZE;
	}
	return ret;
}
#endif /* ENABLE_ASSERTIONS*/

bool is_mpu_enabled_ctx(const xlat_ctx_t *ctx)
{
	if (ctx->xlat_regime == EL1_EL0_REGIME) {
		assert(xlat_arch_current_el() >= 1U);
		return (read_sctlr_el1() & SCTLR_M_BIT) != 0U;
	} else {
		assert(xlat_arch_current_el() >= 2U);
		return (read_sctlr_el2() & SCTLR_M_BIT) != 0U;
	}
}

bool is_dcache_enabled(void)
{
	unsigned int el = get_current_el();

	if (el == 1U) {
		return (read_sctlr_el1() & SCTLR_C_BIT) != 0U;
	} else {  /* must be EL2 */
		return (read_sctlr_el2() & SCTLR_C_BIT) != 0U;
	}
}

unsigned int xlat_arch_current_el(void)
{
	unsigned int el = (unsigned int)GET_EL(read_CurrentEl());

	assert(el > 0U);

	return el;
}

