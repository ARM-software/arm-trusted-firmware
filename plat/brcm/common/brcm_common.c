/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <plat/common/platform.h>

#include <plat_brcm.h>
#include <platform_def.h>

/* Weak definitions may be overridden in specific BRCM platform */
#pragma weak plat_get_ns_image_entrypoint
#pragma weak plat_brcm_get_mmap

uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return PLAT_BRCM_NS_IMAGE_OFFSET;
#endif
}

uint32_t brcm_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	return 0;
}

uint32_t brcm_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = el_implemented(2) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

const mmap_region_t *plat_brcm_get_mmap(void)
{
	return plat_brcm_mmap;
}
