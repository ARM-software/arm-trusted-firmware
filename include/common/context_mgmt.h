/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __CM_H__
#define __CM_H__

#include <arch.h>
#include <bl_common.h>
#include <common_def.h>

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct entry_point_info;

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
void cm_init(void);
void *cm_get_context_by_mpidr(uint64_t mpidr,
			      uint32_t security_state) __warn_deprecated;
void cm_set_context_by_mpidr(uint64_t mpidr,
			     void *context,
			     uint32_t security_state) __warn_deprecated;
void *cm_get_context_by_index(unsigned int cpu_idx,
			      unsigned int security_state);
void cm_set_context_by_index(unsigned int cpu_idx,
			     void *context,
			     unsigned int security_state);
void *cm_get_context(uint32_t security_state);
void cm_set_context(void *context, uint32_t security_state);
inline void cm_set_next_context(void *context);
void cm_init_context(uint64_t mpidr,
		     const struct entry_point_info *ep) __warn_deprecated;
void cm_init_my_context(const struct entry_point_info *ep);
void cm_init_context_by_index(unsigned int cpu_idx,
			      const struct entry_point_info *ep);
void cm_prepare_el3_exit(uint32_t security_state);
void cm_el1_sysregs_context_save(uint32_t security_state);
void cm_el1_sysregs_context_restore(uint32_t security_state);
void cm_set_elr_el3(uint32_t security_state, uint64_t entrypoint);
void cm_set_elr_spsr_el3(uint32_t security_state,
			 uint64_t entrypoint, uint32_t spsr);
void cm_write_scr_el3_bit(uint32_t security_state,
			  uint32_t bit_pos,
			  uint32_t value);
void cm_set_next_eret_context(uint32_t security_state);
uint32_t cm_get_scr_el3(uint32_t security_state);

/* Inline definitions */

/*******************************************************************************
 * This function is used to program the context that's used for exception
 * return. This initializes the SP_EL3 to a pointer to a 'cpu_context' set for
 * the required security state
 ******************************************************************************/
inline void cm_set_next_context(void *context)
{
#if DEBUG
	uint64_t sp_mode;

	/*
	 * Check that this function is called with SP_EL0 as the stack
	 * pointer
	 */
	__asm__ volatile("mrs	%0, SPSel\n"
			 : "=r" (sp_mode));

	assert(sp_mode == MODE_SP_EL0);
#endif

	__asm__ volatile("msr	spsel, #1\n"
			 "mov	sp, %0\n"
			 "msr	spsel, #0\n"
			 : : "r" (context));
}
#endif /* __CM_H__ */
