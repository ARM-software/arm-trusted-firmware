/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2022, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <lib/el3_runtime/aarch64/context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/el3_runtime/simd_ctx.h>
#include <lib/extensions/sve.h>
#include <plat/common/platform.h>

#if CTX_INCLUDE_FPREGS || CTX_INCLUDE_SVE_REGS

/* SIMD context managed for Secure and Normal Worlds. */
#define SIMD_CTXT_COUNT	2

#if SEPARATE_SIMD_SECTION
__section(".simd_context")
#else
__section(".bss.simd_context")
#endif
static simd_regs_t simd_context[SIMD_CTXT_COUNT][PLATFORM_CORE_COUNT];

void simd_ctx_save(uint32_t security_state, bool hint_sve)
{
	simd_regs_t *regs;

	if (security_state != NON_SECURE && security_state != SECURE) {
		ERROR("Unsupported security state specified for SIMD context: %u\n",
		      security_state);
		panic();
	}

	regs = &simd_context[security_state][plat_my_core_pos()];

#if CTX_INCLUDE_SVE_REGS
	regs->hint = hint_sve;

	if (hint_sve) {
		/*
		 * Hint bit denoting absence of SVE live state. Hence, only
		 * save FP context.
		 */
		fpregs_context_save(regs);
	} else {
		sve_context_save(regs);
	}
#elif CTX_INCLUDE_FPREGS
	fpregs_context_save(regs);
#endif
}

void simd_ctx_restore(uint32_t security_state)
{
	simd_regs_t *regs;

	if (security_state != NON_SECURE && security_state != SECURE) {
		ERROR("Unsupported security state specified for SIMD context: %u\n",
		      security_state);
		panic();
	}

	regs = &simd_context[security_state][plat_my_core_pos()];

#if CTX_INCLUDE_SVE_REGS
	if (regs->hint) {
		fpregs_context_restore(regs);
	} else {
		sve_context_restore(regs);
	}
#elif CTX_INCLUDE_FPREGS
	fpregs_context_restore(regs);
#endif
}
#endif /* CTX_INCLUDE_FPREGS || CTX_INCLUDE_SVE_REGS */
