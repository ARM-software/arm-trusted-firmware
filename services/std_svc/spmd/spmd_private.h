/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMD_PRIVATE_H
#define SPMD_PRIVATE_H

#include <context.h>

/*******************************************************************************
 * Constants that allow assembler code to preserve callee-saved registers of the
 * C runtime context while performing a security state switch.
 ******************************************************************************/
#define SPMD_C_RT_CTX_X19		0x0
#define SPMD_C_RT_CTX_X20		0x8
#define SPMD_C_RT_CTX_X21		0x10
#define SPMD_C_RT_CTX_X22		0x18
#define SPMD_C_RT_CTX_X23		0x20
#define SPMD_C_RT_CTX_X24		0x28
#define SPMD_C_RT_CTX_X25		0x30
#define SPMD_C_RT_CTX_X26		0x38
#define SPMD_C_RT_CTX_X27		0x40
#define SPMD_C_RT_CTX_X28		0x48
#define SPMD_C_RT_CTX_X29		0x50
#define SPMD_C_RT_CTX_X30		0x58

#define SPMD_C_RT_CTX_SIZE		0x60
#define SPMD_C_RT_CTX_ENTRIES		(SPMD_C_RT_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLER__
#include <services/spci_svc.h>
#include <stdint.h>

/*
 * Convert a function no. in a FID to a bit position. All function nos. are
 * between 0 and 0x1f
 */
#define SPCI_FNO_TO_BIT_POS(_fid)	(1 << ((_fid) & U(0x1f)))

typedef enum spmc_state {
	SPMC_STATE_RESET = 0,
	SPMC_STATE_IDLE
} spmc_state_t;

/*
 * Data structure used by the SPM dispatcher (SPMD) in EL3 to track context of
 * the SPM core (SPMC) at the next lower EL.
 */
typedef struct spmd_spm_core_context {
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
	spmc_state_t state;
} spmd_spm_core_context_t;

/*
 * Data structure used by the SPM dispatcher (SPMD) in EL3 to track sequence of
 * SPCI calls from lower ELs.
 *
 * next_smc_bit_map: Per-cpu bit map of SMCs from each world that are expected
 *                   next.
 */
typedef struct spmd_spci_context {
	uint32_t next_smc_bit_map[2];
} spmd_spci_context_t;

/* Functions used to enter/exit a Secure Partition synchronously */
uint64_t spmd_spm_core_sync_entry(spmd_spm_core_context_t *ctx);
__dead2 void spmd_spm_core_sync_exit(uint64_t rc);

/* Assembly helpers */
uint64_t spmd_spm_core_enter(uint64_t *c_rt_ctx);
void __dead2 spmd_spm_core_exit(uint64_t c_rt_ctx, uint64_t ret);

#endif /* __ASSEMBLER__ */

#endif /* SPMD_PRIVATE_H */
