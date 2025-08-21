/*
 * Copyright (c) 2019-2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <platform_def.h>
#include <plat/common/platform.h>

#include "socfpga_private.h"

unsigned long socfpga_get_ns_image_entrypoint(void)
{
	return PLAT_NS_IMAGE_OFFSET;
}

/******************************************************************************
 * Gets SPSR for BL32 entry
 *****************************************************************************/
uint32_t socfpga_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	return 0;
}

/******************************************************************************
 * Gets SPSR for BL33 entry
 *****************************************************************************/
uint32_t socfpga_get_spsr_for_bl33_entry(void)
{
	unsigned long el_status;
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	mode = (el_status) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

// common/lib/libc/socfpga_memcpy_s.c
int socfpga_memcpy_s(void *dst, size_t dsize, void *src, size_t ssize)
{
	unsigned int *s = (unsigned int *)src;
	unsigned int *d = (unsigned int *)dst;

	if (!dst || !src)
		return -ENOMEM;

	if (!dsize || !ssize)
		return -ERANGE;

	if (ssize > dsize)
		return -EINVAL;

	if (d > s && d < s + ssize)
		return -EOPNOTSUPP;

	if (s > d && s < d + dsize)
		return -EOPNOTSUPP;

	// Backward word-wise copy
	while (ssize--)
		d[ssize] = s[ssize];

	return 0;
}
