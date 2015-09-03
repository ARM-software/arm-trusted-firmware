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

#ifndef __TSPD_PRIVATE_H__
#define __TSPD_PRIVATE_H__

#include <arch.h>
#include <context.h>
#include <interrupt_mgmt.h>
#include <platform_def.h>
#include <psci.h>

/*******************************************************************************
 * Secure Payload PM state information e.g. SP is suspended, uninitialised etc
 * and macros to access the state information in the per-cpu 'state' flags
 ******************************************************************************/
#define TSP_PSTATE_OFF		0
#define TSP_PSTATE_ON		1
#define TSP_PSTATE_SUSPEND	2
#define TSP_PSTATE_SHIFT	0
#define TSP_PSTATE_MASK	0x3
#define get_tsp_pstate(state)	((state >> TSP_PSTATE_SHIFT) & TSP_PSTATE_MASK)
#define clr_tsp_pstate(state)	(state &= ~(TSP_PSTATE_MASK \
					    << TSP_PSTATE_SHIFT))
#define set_tsp_pstate(st, pst)	do {					       \
					clr_tsp_pstate(st);		       \
					st |= (pst & TSP_PSTATE_MASK) <<       \
						TSP_PSTATE_SHIFT;	       \
				} while (0);


/*
 * This flag is used by the TSPD to determine if the TSP is servicing a standard
 * SMC request prior to programming the next entry into the TSP e.g. if TSP
 * execution is preempted by a non-secure interrupt and handed control to the
 * normal world. If another request which is distinct from what the TSP was
 * previously doing arrives, then this flag will be help the TSPD to either
 * reject the new request or service it while ensuring that the previous context
 * is not corrupted.
 */
#define STD_SMC_ACTIVE_FLAG_SHIFT	2
#define STD_SMC_ACTIVE_FLAG_MASK	1
#define get_std_smc_active_flag(state)	((state >> STD_SMC_ACTIVE_FLAG_SHIFT) \
					 & STD_SMC_ACTIVE_FLAG_MASK)
#define set_std_smc_active_flag(state)	(state |=                             \
					 1 << STD_SMC_ACTIVE_FLAG_SHIFT)
#define clr_std_smc_active_flag(state)	(state &=                             \
					 ~(STD_SMC_ACTIVE_FLAG_MASK           \
					   << STD_SMC_ACTIVE_FLAG_SHIFT))

/*******************************************************************************
 * Secure Payload execution state information i.e. aarch32 or aarch64
 ******************************************************************************/
#define TSP_AARCH32		MODE_RW_32
#define TSP_AARCH64		MODE_RW_64

/*******************************************************************************
 * The SPD should know the type of Secure Payload.
 ******************************************************************************/
#define TSP_TYPE_UP		PSCI_TOS_NOT_UP_MIG_CAP
#define TSP_TYPE_UPM		PSCI_TOS_UP_MIG_CAP
#define TSP_TYPE_MP		PSCI_TOS_NOT_PRESENT_MP

/*******************************************************************************
 * Secure Payload migrate type information as known to the SPD. We assume that
 * the SPD is dealing with an MP Secure Payload.
 ******************************************************************************/
#define TSP_MIGRATE_INFO		TSP_TYPE_MP

/*******************************************************************************
 * Number of cpus that the present on this platform. TODO: Rely on a topology
 * tree to determine this in the future to avoid assumptions about mpidr
 * allocation
 ******************************************************************************/
#define TSPD_CORE_COUNT		PLATFORM_CORE_COUNT

/*******************************************************************************
 * Constants that allow assembler code to preserve callee-saved registers of the
 * C runtime context while performing a security state switch.
 ******************************************************************************/
#define TSPD_C_RT_CTX_X19		0x0
#define TSPD_C_RT_CTX_X20		0x8
#define TSPD_C_RT_CTX_X21		0x10
#define TSPD_C_RT_CTX_X22		0x18
#define TSPD_C_RT_CTX_X23		0x20
#define TSPD_C_RT_CTX_X24		0x28
#define TSPD_C_RT_CTX_X25		0x30
#define TSPD_C_RT_CTX_X26		0x38
#define TSPD_C_RT_CTX_X27		0x40
#define TSPD_C_RT_CTX_X28		0x48
#define TSPD_C_RT_CTX_X29		0x50
#define TSPD_C_RT_CTX_X30		0x58
#define TSPD_C_RT_CTX_SIZE		0x60
#define TSPD_C_RT_CTX_ENTRIES		(TSPD_C_RT_CTX_SIZE >> DWORD_SHIFT)

/*******************************************************************************
 * Constants that allow assembler code to preserve caller-saved registers of the
 * SP context while performing a TSP preemption.
 * Note: These offsets have to match with the offsets for the corresponding
 * registers in cpu_context as we are using memcpy to copy the values from
 * cpu_context to sp_ctx.
 ******************************************************************************/
#define TSPD_SP_CTX_X0		0x0
#define TSPD_SP_CTX_X1		0x8
#define TSPD_SP_CTX_X2		0x10
#define TSPD_SP_CTX_X3		0x18
#define TSPD_SP_CTX_X4		0x20
#define TSPD_SP_CTX_X5		0x28
#define TSPD_SP_CTX_X6		0x30
#define TSPD_SP_CTX_X7		0x38
#define TSPD_SP_CTX_X8		0x40
#define TSPD_SP_CTX_X9		0x48
#define TSPD_SP_CTX_X10		0x50
#define TSPD_SP_CTX_X11		0x58
#define TSPD_SP_CTX_X12		0x60
#define TSPD_SP_CTX_X13		0x68
#define TSPD_SP_CTX_X14		0x70
#define TSPD_SP_CTX_X15		0x78
#define TSPD_SP_CTX_X16		0x80
#define TSPD_SP_CTX_X17		0x88
#define TSPD_SP_CTX_SIZE	0x90
#define TSPD_SP_CTX_ENTRIES		(TSPD_SP_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLY__

#include <cassert.h>
#include <stdint.h>

/*
 * The number of arguments to save during a SMC call for TSP.
 * Currently only x1 and x2 are used by TSP.
 */
#define TSP_NUM_ARGS	0x2

/* AArch64 callee saved general purpose register context structure. */
DEFINE_REG_STRUCT(c_rt_regs, TSPD_C_RT_CTX_ENTRIES);

/*
 * Compile time assertion to ensure that both the compiler and linker
 * have the same double word aligned view of the size of the C runtime
 * register context.
 */
CASSERT(TSPD_C_RT_CTX_SIZE == sizeof(c_rt_regs_t),	\
	assert_spd_c_rt_regs_size_mismatch);

/* SEL1 Secure payload (SP) caller saved register context structure. */
DEFINE_REG_STRUCT(sp_ctx_regs, TSPD_SP_CTX_ENTRIES);

/*
 * Compile time assertion to ensure that both the compiler and linker
 * have the same double word aligned view of the size of the C runtime
 * register context.
 */
CASSERT(TSPD_SP_CTX_SIZE == sizeof(sp_ctx_regs_t),	\
	assert_spd_sp_regs_size_mismatch);

/*******************************************************************************
 * Structure which helps the SPD to maintain the per-cpu state of the SP.
 * 'saved_spsr_el3' - temporary copy to allow S-EL1 interrupt handling when
 *                    the TSP has been preempted.
 * 'saved_elr_el3'  - temporary copy to allow S-EL1 interrupt handling when
 *                    the TSP has been preempted.
 * 'state'          - collection of flags to track SP state e.g. on/off
 * 'mpidr'          - mpidr to associate a context with a cpu
 * 'c_rt_ctx'       - stack address to restore C runtime context from after
 *                    returning from a synchronous entry into the SP.
 * 'cpu_ctx'        - space to maintain SP architectural state
 * 'saved_tsp_args' - space to store arguments for TSP arithmetic operations
 *                    which will queried using the TSP_GET_ARGS SMC by TSP.
 * 'sp_ctx'         - space to save the SEL1 Secure Payload(SP) caller saved
 *                    register context after it has been preempted by an EL3
 *                    routed NS interrupt and when a Secure Interrupt is taken
 *                    to SP.
 ******************************************************************************/
typedef struct tsp_context {
	uint64_t saved_elr_el3;
	uint32_t saved_spsr_el3;
	uint32_t state;
	uint64_t mpidr;
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
	uint64_t saved_tsp_args[TSP_NUM_ARGS];
#if TSP_NS_INTR_ASYNC_PREEMPT
	sp_ctx_regs_t sp_ctx;
#endif
} tsp_context_t;

/* Helper macros to store and retrieve tsp args from tsp_context */
#define store_tsp_args(tsp_ctx, x1, x2)		do {\
				tsp_ctx->saved_tsp_args[0] = x1;\
				tsp_ctx->saved_tsp_args[1] = x2;\
			} while (0)

#define get_tsp_args(tsp_ctx, x1, x2)	do {\
				x1 = tsp_ctx->saved_tsp_args[0];\
				x2 = tsp_ctx->saved_tsp_args[1];\
			} while (0)

/* TSPD power management handlers */
extern const spd_pm_ops_t tspd_pm;

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct tsp_vectors;

/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
uint64_t tspd_enter_sp(uint64_t *c_rt_ctx);
void __dead2 tspd_exit_sp(uint64_t c_rt_ctx, uint64_t ret);
uint64_t tspd_synchronous_sp_entry(tsp_context_t *tsp_ctx);
void __dead2 tspd_synchronous_sp_exit(tsp_context_t *tsp_ctx, uint64_t ret);
void tspd_init_tsp_ep_state(struct entry_point_info *tsp_ep,
				uint32_t rw,
				uint64_t pc,
				tsp_context_t *tsp_ctx);

extern tsp_context_t tspd_sp_context[TSPD_CORE_COUNT];
extern struct tsp_vectors *tsp_vectors;
#endif /*__ASSEMBLY__*/

#endif /* __TSPD_PRIVATE_H__ */
