/*
 * Copyright (c) 2017-2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPM_MM_PRIVATE_H
#define SPM_MM_PRIVATE_H

#include <context.h>
#include "spm_common.h"

/*******************************************************************************
 * Constants that allow assembler code to preserve callee-saved registers of the
 * C runtime context while performing a security state switch.
 ******************************************************************************/
#define SP_C_RT_CTX_X19		0x0
#define SP_C_RT_CTX_X20		0x8
#define SP_C_RT_CTX_X21		0x10
#define SP_C_RT_CTX_X22		0x18
#define SP_C_RT_CTX_X23		0x20
#define SP_C_RT_CTX_X24		0x28
#define SP_C_RT_CTX_X25		0x30
#define SP_C_RT_CTX_X26		0x38
#define SP_C_RT_CTX_X27		0x40
#define SP_C_RT_CTX_X28		0x48
#define SP_C_RT_CTX_X29		0x50
#define SP_C_RT_CTX_X30		0x58

#define SP_C_RT_CTX_SIZE	0x60
#define SP_C_RT_CTX_ENTRIES	(SP_C_RT_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

typedef enum sp_state {
	SP_STATE_RESET = 0,
	SP_STATE_IDLE,
	SP_STATE_BUSY
} sp_state_t;

typedef struct sp_context {
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
	xlat_ctx_t *xlat_ctx_handle;

	sp_state_t state;
	spinlock_t state_lock;
} sp_context_t;


void spm_sp_setup(sp_context_t *sp_ctx);

int32_t spm_memory_attributes_get_smc_handler(sp_context_t *sp_ctx,
					      uintptr_t base_va,
					      uint32_t *page_count,
					      uint32_t *attr);
int spm_memory_attributes_set_smc_handler(sp_context_t *sp_ctx,
					  u_register_t page_address,
					  u_register_t pages_count,
					  u_register_t smc_attributes);

#endif /* __ASSEMBLER__ */

#endif /* SPM_MM_PRIVATE_H */
