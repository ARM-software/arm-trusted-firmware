/*
 * Copyright (c) 2017-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <lib/cassert.h>
#include <lib/el3_runtime/pubsub.h>
#include <lib/extensions/sve.h>

CASSERT(SVE_VECTOR_LEN <= 2048, assert_sve_vl_too_long);
CASSERT(SVE_VECTOR_LEN >= 128, assert_sve_vl_too_short);
CASSERT((SVE_VECTOR_LEN % 128) == 0, assert_sve_vl_granule);

/*
 * Converts SVE vector size restriction in bytes to LEN according to ZCR_EL3 documentation.
 * VECTOR_SIZE = (LEN+1) * 128
 */
#define CONVERT_SVE_LENGTH(x)	(((x / 128) - 1))

void sve_enable(cpu_context_t *context)
{
	u_register_t cptr_el3;

	cptr_el3 = read_ctx_reg(get_el3state_ctx(context), CTX_CPTR_EL3);

	/* Enable access to SVE functionality for all ELs. */
	cptr_el3 = (cptr_el3 | CPTR_EZ_BIT) & ~(TFP_BIT);
	write_ctx_reg(get_el3state_ctx(context), CTX_CPTR_EL3, cptr_el3);

	/* Restrict maximum SVE vector length (SVE_VECTOR_LEN+1) * 128. */
	write_ctx_reg(get_el3state_ctx(context), CTX_ZCR_EL3,
		(ZCR_EL3_LEN_MASK & CONVERT_SVE_LENGTH(SVE_VECTOR_LEN)));
}

void sve_disable(cpu_context_t *context)
{
	u_register_t reg;
	el3_state_t *state;

	/* Get the context state. */
	state = get_el3state_ctx(context);

	/* Disable SVE and FPU since they share registers. */
	reg = read_ctx_reg(state, CTX_CPTR_EL3);
	reg &= ~CPTR_EZ_BIT;	/* Trap SVE */
	reg |= TFP_BIT;		/* Trap FPU/SIMD */
	write_ctx_reg(state, CTX_CPTR_EL3, reg);
}
