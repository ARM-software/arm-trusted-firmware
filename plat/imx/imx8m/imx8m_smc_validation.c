/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <common/smc_validation_framework.h>

#include <platform_def.h>

/*
 * plat_is_valid_ns_address_range() - i.MX8M override.
 *
 * Accept only the DRAM region as valid Non-Secure memory, excluding the
 * BL32 (OP-TEE) carve-out.  This prevents NS callers from passing
 * pointers into OCRAM (BL31), ROM, MMIO, or secure BL32 memory
 * regions through SMC arguments.
 */
bool plat_is_valid_ns_address_range(uintptr_t base, size_t size)
{
	uintptr_t end = base + size;

	/* Reject addresses outside DRAM */
	if ((base < IMX_DRAM_BASE) ||
	    (end > ((uintptr_t)IMX_DRAM_BASE + IMX_DRAM_SIZE))) {
		return false;
	}

#if defined(BL32_BASE) && defined(BL32_SIZE)
	/* Reject addresses that overlap the BL32 (OP-TEE) region */
	if ((end > BL32_BASE) &&
	    (base < (BL32_BASE + BL32_SIZE))) {
		return false;
	}
#endif

	return true;
}
