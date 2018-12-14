/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <lib/el3_runtime/pubsub.h>
#include <lib/extensions/sve.h>

bool sve_supported(void)
{
	uint64_t features;

	features = read_id_aa64pfr0_el1() >> ID_AA64PFR0_SVE_SHIFT;
	return (features & ID_AA64PFR0_SVE_MASK) == 1U;
}

static void *disable_sve_hook(const void *arg)
{
	uint64_t cptr;

	if (!sve_supported())
		return (void *)-1;

	/*
	 * Disable SVE, SIMD and FP access for the Secure world.
	 * As the SIMD/FP registers are part of the SVE Z-registers, any
	 * use of SIMD/FP functionality will corrupt the SVE registers.
	 * Therefore it is necessary to prevent use of SIMD/FP support
	 * in the Secure world as well as SVE functionality.
	 */
	cptr = read_cptr_el3();
	cptr = (cptr | TFP_BIT) & ~(CPTR_EZ_BIT);
	write_cptr_el3(cptr);

	/*
	 * No explicit ISB required here as ERET to switch to Secure
	 * world covers it
	 */
	return (void *)0;
}

static void *enable_sve_hook(const void *arg)
{
	uint64_t cptr;

	if (!sve_supported())
		return (void *)-1;

	/*
	 * Enable SVE, SIMD and FP access for the Non-secure world.
	 */
	cptr = read_cptr_el3();
	cptr = (cptr | CPTR_EZ_BIT) & ~(TFP_BIT);
	write_cptr_el3(cptr);

	/*
	 * No explicit ISB required here as ERET to switch to Non-secure
	 * world covers it
	 */
	return (void *)0;
}

void sve_enable(bool el2_unused)
{
	uint64_t cptr;

	if (!sve_supported())
		return;

#if CTX_INCLUDE_FPREGS
	/*
	 * CTX_INCLUDE_FPREGS is not supported on SVE enabled systems.
	 */
	assert(0);
#endif
	/*
	 * Update CPTR_EL3 to enable access to SVE functionality for the
	 * Non-secure world.
	 * NOTE - assumed that CPTR_EL3.TFP is set to allow access to
	 * the SIMD, floating-point and SVE support.
	 *
	 * CPTR_EL3.EZ: Set to 1 to enable access to SVE  functionality
	 *  in the Non-secure world.
	 */
	cptr = read_cptr_el3();
	cptr |= CPTR_EZ_BIT;
	write_cptr_el3(cptr);

	/*
	 * Need explicit ISB here to guarantee that update to ZCR_ELx
	 * and CPTR_EL2.TZ do not result in trap to EL3.
	 */
	isb();

	/*
	 * Ensure lower ELs have access to full vector length.
	 */
	write_zcr_el3(ZCR_EL3_LEN_MASK);

	if (el2_unused) {
		/*
		 * Update CPTR_EL2 to enable access to SVE functionality
		 * for Non-secure world, EL2 and Non-secure EL1 and EL0.
		 * NOTE - assumed that CPTR_EL2.TFP is set to allow
		 * access to the SIMD, floating-point and SVE support.
		 *
		 * CPTR_EL2.TZ: Set to 0 to enable access to SVE support
		 *  for EL2 and Non-secure EL1 and EL0.
		 */
		cptr = read_cptr_el2();
		cptr &= ~(CPTR_EL2_TZ_BIT);
		write_cptr_el2(cptr);

		/*
		 * Ensure lower ELs have access to full vector length.
		 */
		write_zcr_el2(ZCR_EL2_LEN_MASK);
	}
	/*
	 * No explicit ISB required here as ERET to switch to
	 * Non-secure world covers it.
	 */
}

SUBSCRIBE_TO_EVENT(cm_exited_normal_world, disable_sve_hook);
SUBSCRIBE_TO_EVENT(cm_entering_normal_world, enable_sve_hook);
