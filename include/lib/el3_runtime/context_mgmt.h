/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONTEXT_MGMT_H
#define CONTEXT_MGMT_H

#include <assert.h>
#include <context.h>
#include <stdint.h>

#include <arch.h>

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct entry_point_info;

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
void cm_init(void);
void *cm_get_context_by_index(unsigned int cpu_idx,
			      unsigned int security_state);
void cm_set_context_by_index(unsigned int cpu_idx,
			     void *context,
			     unsigned int security_state);
void *cm_get_context(uint32_t security_state);
void cm_set_context(void *context, uint32_t security_state);
void cm_init_my_context(const struct entry_point_info *ep);
void cm_init_context_by_index(unsigned int cpu_idx,
			      const struct entry_point_info *ep);
void cm_setup_context(cpu_context_t *ctx, const struct entry_point_info *ep);
void cm_prepare_el3_exit(uint32_t security_state);

#ifdef __aarch64__
#if CTX_INCLUDE_EL2_REGS
void cm_el2_sysregs_context_save(uint32_t security_state);
void cm_el2_sysregs_context_restore(uint32_t security_state);
#endif

void cm_el1_sysregs_context_save(uint32_t security_state);
void cm_el1_sysregs_context_restore(uint32_t security_state);
void cm_set_elr_el3(uint32_t security_state, uintptr_t entrypoint);
void cm_set_elr_spsr_el3(uint32_t security_state,
			uintptr_t entrypoint, uint32_t spsr);
void cm_write_scr_el3_bit(uint32_t security_state,
			  uint32_t bit_pos,
			  uint32_t value);
void cm_set_next_eret_context(uint32_t security_state);
u_register_t cm_get_scr_el3(uint32_t security_state);

/* Inline definitions */

/*******************************************************************************
 * This function is used to program the context that's used for exception
 * return. This initializes the SP_EL3 to a pointer to a 'cpu_context' set for
 * the required security state
 ******************************************************************************/
static inline void cm_set_next_context(void *context)
{
#if ENABLE_ASSERTIONS
	uint64_t sp_mode;

	/*
	 * Check that this function is called with SP_EL0 as the stack
	 * pointer
	 */
	__asm__ volatile("mrs	%0, SPSel\n"
			 : "=r" (sp_mode));

	assert(sp_mode == MODE_SP_EL0);
#endif /* ENABLE_ASSERTIONS */

	__asm__ volatile("msr	spsel, #1\n"
			 "mov	sp, %0\n"
			 "msr	spsel, #0\n"
			 : : "r" (context));
}

#else
void *cm_get_next_context(void);
void cm_set_next_context(void *context);
#endif /* __aarch64__ */

#endif /* CONTEXT_MGMT_H */
