/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SPM_PRIVATE_H__
#define __SPM_PRIVATE_H__

#include <context.h>

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


#ifndef __ASSEMBLY__

#include <spinlock.h>
#include <stdint.h>
#include <xlat_tables_v2.h>

/* Handle on the Secure partition translation context */
extern xlat_ctx_t *secure_partition_xlat_ctx_handle;

struct entry_point_info;

typedef struct secure_partition_context {
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
	unsigned int sp_init_in_progress;
	spinlock_t lock;
} secure_partition_context_t;

uint64_t spm_secure_partition_enter(uint64_t *c_rt_ctx);
void __dead2 spm_secure_partition_exit(uint64_t c_rt_ctx, uint64_t ret);
void spm_init_sp_ep_state(struct entry_point_info *sp_ep_info,
			  uint64_t pc,
			  secure_partition_context_t *sp_ctx_ptr);
#endif /* __ASSEMBLY__ */

#endif /* __SPM_PRIVATE_H__ */
