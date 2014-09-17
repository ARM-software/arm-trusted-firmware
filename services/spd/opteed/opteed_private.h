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

#ifndef __OPTEED_PRIVATE_H__
#define __OPTEED_PRIVATE_H__

#include <arch.h>
#include <context.h>
#include <interrupt_mgmt.h>
#include <platform_def.h>
#include <psci.h>

/*******************************************************************************
 * OPTEE PM state information e.g. OPTEE is suspended, uninitialised etc
 * and macros to access the state information in the per-cpu 'state' flags
 ******************************************************************************/
#define OPTEE_PSTATE_OFF		0
#define OPTEE_PSTATE_ON			1
#define OPTEE_PSTATE_SUSPEND		2
#define OPTEE_PSTATE_SHIFT		0
#define OPTEE_PSTATE_MASK		0x3
#define get_optee_pstate(state)	((state >> OPTEE_PSTATE_SHIFT) & \
				 OPTEE_PSTATE_MASK)
#define clr_optee_pstate(state)	(state &= ~(OPTEE_PSTATE_MASK \
					    << OPTEE_PSTATE_SHIFT))
#define set_optee_pstate(st, pst) do {					       \
					clr_optee_pstate(st);		       \
					st |= (pst & OPTEE_PSTATE_MASK) <<     \
						OPTEE_PSTATE_SHIFT;	       \
				} while (0)


/*******************************************************************************
 * OPTEE execution state information i.e. aarch32 or aarch64
 ******************************************************************************/
#define OPTEE_AARCH32		MODE_RW_32
#define OPTEE_AARCH64		MODE_RW_64

/*******************************************************************************
 * The OPTEED should know the type of OPTEE
 ******************************************************************************/
#define OPTEE_TYPE_UP		PSCI_TOS_NOT_UP_MIG_CAP
#define OPTEE_TYPE_UPM		PSCI_TOS_UP_MIG_CAP
#define OPTEE_TYPE_MP		PSCI_TOS_NOT_PRESENT_MP

/*******************************************************************************
 * OPTEE migrate type information as known to the OPTEED. We assume that
 * the OPTEED is dealing with an MP Secure Payload.
 ******************************************************************************/
#define OPTEE_MIGRATE_INFO		OPTEE_TYPE_MP

/*******************************************************************************
 * Number of cpus that the present on this platform. TODO: Rely on a topology
 * tree to determine this in the future to avoid assumptions about mpidr
 * allocation
 ******************************************************************************/
#define OPTEED_CORE_COUNT		PLATFORM_CORE_COUNT

/*******************************************************************************
 * Constants that allow assembler code to preserve callee-saved registers of the
 * C runtime context while performing a security state switch.
 ******************************************************************************/
#define OPTEED_C_RT_CTX_X19		0x0
#define OPTEED_C_RT_CTX_X20		0x8
#define OPTEED_C_RT_CTX_X21		0x10
#define OPTEED_C_RT_CTX_X22		0x18
#define OPTEED_C_RT_CTX_X23		0x20
#define OPTEED_C_RT_CTX_X24		0x28
#define OPTEED_C_RT_CTX_X25		0x30
#define OPTEED_C_RT_CTX_X26		0x38
#define OPTEED_C_RT_CTX_X27		0x40
#define OPTEED_C_RT_CTX_X28		0x48
#define OPTEED_C_RT_CTX_X29		0x50
#define OPTEED_C_RT_CTX_X30		0x58
#define OPTEED_C_RT_CTX_SIZE		0x60
#define OPTEED_C_RT_CTX_ENTRIES		(OPTEED_C_RT_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLY__

#include <cassert.h>
#include <stdint.h>

typedef uint32_t optee_vector_isn_t;

typedef struct optee_vectors {
	optee_vector_isn_t std_smc_entry;
	optee_vector_isn_t fast_smc_entry;
	optee_vector_isn_t cpu_on_entry;
	optee_vector_isn_t cpu_off_entry;
	optee_vector_isn_t cpu_resume_entry;
	optee_vector_isn_t cpu_suspend_entry;
	optee_vector_isn_t fiq_entry;
	optee_vector_isn_t system_off_entry;
	optee_vector_isn_t system_reset_entry;
} optee_vectors_t;

/*
 * The number of arguments to save during a SMC call for OPTEE.
 * Currently only x1 and x2 are used by OPTEE.
 */
#define OPTEE_NUM_ARGS	0x2

/* AArch64 callee saved general purpose register context structure. */
DEFINE_REG_STRUCT(c_rt_regs, OPTEED_C_RT_CTX_ENTRIES);

/*
 * Compile time assertion to ensure that both the compiler and linker
 * have the same double word aligned view of the size of the C runtime
 * register context.
 */
CASSERT(OPTEED_C_RT_CTX_SIZE == sizeof(c_rt_regs_t),	\
	assert_spd_c_rt_regs_size_mismatch);

/*******************************************************************************
 * Structure which helps the OPTEED to maintain the per-cpu state of OPTEE.
 * 'state'          - collection of flags to track OPTEE state e.g. on/off
 * 'mpidr'          - mpidr to associate a context with a cpu
 * 'c_rt_ctx'       - stack address to restore C runtime context from after
 *                    returning from a synchronous entry into OPTEE.
 * 'cpu_ctx'        - space to maintain OPTEE architectural state
 ******************************************************************************/
typedef struct optee_context {
	uint32_t state;
	uint64_t mpidr;
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
} optee_context_t;

/* OPTEED power management handlers */
extern const spd_pm_ops_t opteed_pm;

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct optee_vectors;

/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
uint64_t opteed_enter_sp(uint64_t *c_rt_ctx);
void __dead2 opteed_exit_sp(uint64_t c_rt_ctx, uint64_t ret);
uint64_t opteed_synchronous_sp_entry(optee_context_t *optee_ctx);
void __dead2 opteed_synchronous_sp_exit(optee_context_t *optee_ctx, uint64_t ret);
void opteed_init_optee_ep_state(struct entry_point_info *optee_ep,
				uint32_t rw,
				uint64_t pc,
				optee_context_t *optee_ctx);

extern optee_context_t opteed_sp_context[OPTEED_CORE_COUNT];
extern uint32_t opteed_rw;
extern struct optee_vectors *optee_vectors;
#endif /*__ASSEMBLY__*/

#endif /* __OPTEED_PRIVATE_H__ */
