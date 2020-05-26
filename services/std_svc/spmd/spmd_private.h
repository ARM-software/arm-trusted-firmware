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
#include <services/ffa_svc.h>
#include <stdint.h>

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
 * Reserve ID for NS physical FFA Endpoint.
 */
#define FFA_NS_ENDPOINT_ID		U(0)

/* Mask and shift to check valid secure FFA Endpoint ID. */
#define SPMC_SECURE_ID_MASK		U(1)
#define SPMC_SECURE_ID_SHIFT		U(15)

/* Functions used to enter/exit SPMC synchronously */
uint64_t spmd_spm_core_sync_entry(spmd_spm_core_context_t *ctx);
__dead2 void spmd_spm_core_sync_exit(uint64_t rc);

/* Assembly helpers */
uint64_t spmd_spm_core_enter(uint64_t *c_rt_ctx);
void __dead2 spmd_spm_core_exit(uint64_t c_rt_ctx, uint64_t ret);

/* SPMC context on current CPU get helper */
spmd_spm_core_context_t *spmd_get_context(void);

#endif /* __ASSEMBLER__ */

#endif /* SPMD_PRIVATE_H */
