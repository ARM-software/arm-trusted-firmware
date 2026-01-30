/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <platform_def.h>

#include <common/smc_validation_framework.h>

/**
 * plat_is_valid_ns_address_range() - Arm platform implementation of the hook.
 *
 * Validates that a given address range lies entirely within the Non-Secure
 * DRAM regions defined for the Arm reference platform. This overrides the
 * weak default provided in common/smc_validation.c.
 */
bool plat_is_valid_ns_address_range(uintptr_t base, size_t size)
{
	/* Range check against Arm standard NS DRAM1 region */
	if ((base >= ARM_NS_DRAM1_BASE) &&
	    ((base + size) <= (ARM_NS_DRAM1_BASE + ARM_NS_DRAM1_SIZE))) {
		return true;
	}

#if defined(__aarch64__) && defined(PLAT_ARM_DRAM2_BASE)
	/* Range check against optional second DRAM region */
	if ((base >= ARM_DRAM2_BASE) &&
	    ((base + size) <= (ARM_DRAM2_BASE + ARM_DRAM2_SIZE))) {
		return true;
	}
#endif

	return false;
}
