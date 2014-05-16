/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <stdint.h>

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
extern void cm_init(void);
extern void *cm_get_context(uint64_t mpidr, uint32_t security_state);
extern void cm_set_context(uint64_t mpidr,
			   void *context,
			   uint32_t security_state);
extern void cm_el3_sysregs_context_save(uint32_t security_state);
extern void cm_el3_sysregs_context_restore(uint32_t security_state);
extern void cm_el1_sysregs_context_save(uint32_t security_state);
extern void cm_el1_sysregs_context_restore(uint32_t security_state);
extern void cm_set_el3_eret_context(uint32_t security_state, uint64_t entrypoint,
		uint32_t spsr, uint32_t scr);
extern void cm_set_el3_elr(uint32_t security_state, uint64_t entrypoint);
extern void cm_set_next_eret_context(uint32_t security_state);
extern void cm_init_pcpu_ptr_cache();
extern void cm_set_pcpu_ptr_cache(const void *pcpu_ptr);
extern void *cm_get_pcpu_ptr_cache(void);

#endif /* __CM_H__ */
