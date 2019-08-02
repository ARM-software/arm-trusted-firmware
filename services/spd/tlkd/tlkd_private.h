/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TLKD_PRIVATE_H
#define TLKD_PRIVATE_H

#include <platform_def.h>

#include <arch.h>
#include <bl31/interrupt_mgmt.h>
#include <context.h>
#include <lib/psci/psci.h>

/*
 * This flag is used by the TLKD to determine if the SP is servicing a yielding
 * SMC request prior to programming the next entry into the SP e.g. if SP
 * execution is preempted by a non-secure interrupt and handed control to the
 * normal world. If another request which is distinct from what the SP was
 * previously doing arrives, then this flag will be help the TLKD to either
 * reject the new request or service it while ensuring that the previous context
 * is not corrupted.
 */
#define YIELD_SMC_ACTIVE_FLAG_SHIFT	2
#define YIELD_SMC_ACTIVE_FLAG_MASK	1
#define get_yield_smc_active_flag(state)				\
			(((state) >> YIELD_SMC_ACTIVE_FLAG_SHIFT)	\
			& YIELD_SMC_ACTIVE_FLAG_MASK)
#define set_yield_smc_active_flag(state)	((state) |=		\
					 (1 << YIELD_SMC_ACTIVE_FLAG_SHIFT))
#define clr_yield_smc_active_flag(state)	((state) &=		\
					 ~(YIELD_SMC_ACTIVE_FLAG_MASK	\
					 << YIELD_SMC_ACTIVE_FLAG_SHIFT))

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

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <lib/cassert.h>

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

#endif /*__ASSEMBLER__*/

#endif /* TLKD_PRIVATE_H */
