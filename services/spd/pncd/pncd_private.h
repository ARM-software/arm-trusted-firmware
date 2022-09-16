/*
 * Copyright (c) 2021-2022, ARM Limited and Contributors. All rights reserved.
 * Portions copyright (c) 2021-2022, ProvenRun S.A.S. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PNCD_PRIVATE_H__
#define __PNCD_PRIVATE_H__

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif /* __ASSEMBLER __ */

#include <context.h>
#ifndef __ASSEMBLER__
#include <lib/cassert.h>
#endif /* __ASSEMBLER __ */

#include <platform_def.h>

/*******************************************************************************
 * Constants that allow assembler code to preserve callee-saved registers of the
 * C runtime context while performing a security state switch.
 ******************************************************************************/
#define PNCD_C_RT_CTX_X19		U(0x0)
#define PNCD_C_RT_CTX_X20		U(0x8)
#define PNCD_C_RT_CTX_X21		U(0x10)
#define PNCD_C_RT_CTX_X22		U(0x18)
#define PNCD_C_RT_CTX_X23		U(0x20)
#define PNCD_C_RT_CTX_X24		U(0x28)
#define PNCD_C_RT_CTX_X25		U(0x30)
#define PNCD_C_RT_CTX_X26		U(0x38)
#define PNCD_C_RT_CTX_X27		U(0x40)
#define PNCD_C_RT_CTX_X28		U(0x48)
#define PNCD_C_RT_CTX_X29		U(0x50)
#define PNCD_C_RT_CTX_X30		U(0x58)
#define PNCD_C_RT_CTX_SIZE		U(0x60)
#define PNCD_C_RT_CTX_ENTRIES		(PNCD_C_RT_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLER__

/* AArch64 callee saved general purpose register context structure. */
DEFINE_REG_STRUCT(c_rt_regs, PNCD_C_RT_CTX_ENTRIES);

/*
 * Compile time assertion to ensure that both the compiler and linker
 * have the same double word aligned view of the size of the C runtime
 * register context.
 */
CASSERT(sizeof(c_rt_regs_t) == PNCD_C_RT_CTX_SIZE,
		assert_spd_c_rt_regs_size_mismatch);

/*******************************************************************************
 * Structure which helps the SPD to maintain the per-cpu state of the SP.
 * 'mpidr'          - mpidr of the CPU running PNC
 * 'c_rt_ctx'       - stack address to restore C runtime context from after
 *                    returning from a synchronous entry into the SP.
 * 'cpu_ctx'        - space to maintain SP architectural state
 ******************************************************************************/
typedef struct pnc_context {
	uint64_t mpidr;
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
} pnc_context_t;

/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
uint64_t pncd_enter_sp(uint64_t *c_rt_ctx);
void __dead2 pncd_exit_sp(uint64_t c_rt_ctx, uint64_t ret);
uint64_t pncd_synchronous_sp_entry(pnc_context_t *pnc_ctx);
void __dead2 pncd_synchronous_sp_exit(pnc_context_t *pnc_ctx, uint64_t ret);
void pncd_init_pnc_ep_state(struct entry_point_info *pnc_ep,
				uint64_t pc,
				pnc_context_t *pnc_ctx);
#endif /* __ASSEMBLER__ */

#endif /* __PNCD_PRIVATE_H__ */
