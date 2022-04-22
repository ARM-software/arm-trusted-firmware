/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPM_COMMON_H
#define SPM_COMMON_H

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

#ifndef __ASSEMBLER__

#include <stdint.h>

/* Assembly helpers */
uint64_t spm_secure_partition_enter(uint64_t *c_rt_ctx);
void __dead2 spm_secure_partition_exit(uint64_t c_rt_ctx, uint64_t ret);

#endif /* __ASSEMBLER__ */

#endif /* SPM_COMMON_H */
