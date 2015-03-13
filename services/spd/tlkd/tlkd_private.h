/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#ifndef __TLKD_PRIVATE_H__
#define __TLKD_PRIVATE_H__

#include <arch.h>
#include <context.h>
#include <interrupt_mgmt.h>
#include <platform_def.h>
#include <psci.h>

/*
 * This flag is used by the TLKD to determine if the SP is servicing a standard
 * SMC request prior to programming the next entry into the SP e.g. if SP
 * execution is preempted by a non-secure interrupt and handed control to the
 * normal world. If another request which is distinct from what the SP was
 * previously doing arrives, then this flag will be help the TLKD to either
 * reject the new request or service it while ensuring that the previous context
 * is not corrupted.
 */
#define STD_SMC_ACTIVE_FLAG_SHIFT	2
#define STD_SMC_ACTIVE_FLAG_MASK	1
#define get_std_smc_active_flag(state)	(((state) >> STD_SMC_ACTIVE_FLAG_SHIFT) \
					 & STD_SMC_ACTIVE_FLAG_MASK)
#define set_std_smc_active_flag(state)	((state) |=                           \
					 (1 << STD_SMC_ACTIVE_FLAG_SHIFT))
#define clr_std_smc_active_flag(state)	((state) &=                           \
					 ~(STD_SMC_ACTIVE_FLAG_MASK           \
					   << STD_SMC_ACTIVE_FLAG_SHIFT))

/*******************************************************************************
 * Translate virtual address received from the NS world
 ******************************************************************************/
#define TLK_TRANSLATE_NS_VADDR		4

/*******************************************************************************
 * Secure Payload execution state information i.e. aarch32 or aarch64
 ******************************************************************************/
#define SP_AARCH32		MODE_RW_32
#define SP_AARCH64		MODE_RW_64

/*******************************************************************************
 * Number of cpus that the present on this platform. TODO: Rely on a topology
 * tree to determine this in the future to avoid assumptions about mpidr
 * allocation
 ******************************************************************************/
#define TLKD_CORE_COUNT		PLATFORM_CORE_COUNT

/*******************************************************************************
 * Constants that allow assembler code to preserve callee-saved registers of the
 * C runtime context while performing a security state switch.
 ******************************************************************************/
#define TLKD_C_RT_CTX_X19		0x0
#define TLKD_C_RT_CTX_X20		0x8
#define TLKD_C_RT_CTX_X21		0x10
#define TLKD_C_RT_CTX_X22		0x18
#define TLKD_C_RT_CTX_X23		0x20
#define TLKD_C_RT_CTX_X24		0x28
#define TLKD_C_RT_CTX_X25		0x30
#define TLKD_C_RT_CTX_X26		0x38
#define TLKD_C_RT_CTX_X27		0x40
#define TLKD_C_RT_CTX_X28		0x48
#define TLKD_C_RT_CTX_X29		0x50
#define TLKD_C_RT_CTX_X30		0x58
#define TLKD_C_RT_CTX_SIZE		0x60
#define TLKD_C_RT_CTX_ENTRIES		(TLKD_C_RT_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLY__

#include <cassert.h>
#include <stdint.h>

/* AArch64 callee saved general purpose register context structure. */
DEFINE_REG_STRUCT(c_rt_regs, TLKD_C_RT_CTX_ENTRIES);

/*
 * Compile time assertion to ensure that both the compiler and linker
 * have the same double word aligned view of the size of the C runtime
 * register context.
 */
CASSERT(TLKD_C_RT_CTX_SIZE == sizeof(c_rt_regs_t),	\
	assert_tlkd_c_rt_regs_size_mismatch);

/*******************************************************************************
 * Structure which helps the SPD to maintain the per-cpu state of the SP.
 * 'state'          - collection of flags to track SP state e.g. on/off
 * 'mpidr'          - mpidr to associate a context with a cpu
 * 'c_rt_ctx'       - stack address to restore C runtime context from after
 *                    returning from a synchronous entry into the SP.
 * 'cpu_ctx'        - space to maintain SP architectural state
 * 'saved_tsp_args' - space to store arguments for TSP arithmetic operations
 *                    which will queried using the TSP_GET_ARGS SMC by TSP.
 ******************************************************************************/
typedef struct tlk_context {
	uint32_t state;
	uint64_t mpidr;
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
} tlk_context_t;

/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
uint64_t tlkd_va_translate(uintptr_t va, int type);
uint64_t tlkd_enter_sp(uint64_t *c_rt_ctx);
void __dead2 tlkd_exit_sp(uint64_t c_rt_ctx, uint64_t ret);
uint64_t tlkd_synchronous_sp_entry(tlk_context_t *tlk_ctx);
void __dead2 tlkd_synchronous_sp_exit(tlk_context_t *tlk_ctx,
			uint64_t ret);
void tlkd_init_tlk_ep_state(struct entry_point_info *tlk_entry_point,
				uint32_t rw,
				uint64_t pc,
				tlk_context_t *tlk_ctx);

#endif /*__ASSEMBLY__*/

#endif /* __TLKD_PRIVATE_H__ */
