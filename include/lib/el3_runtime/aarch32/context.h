/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include <lib/utils_def.h>

/*******************************************************************************
 * Constants that allow assembler code to access members of and the 'regs'
 * structure at their correct offsets.
 ******************************************************************************/
#define CTX_REGS_OFFSET		U(0x0)
#define CTX_GPREG_R0		U(0x0)
#define CTX_GPREG_R1		U(0x4)
#define CTX_GPREG_R2		U(0x8)
#define CTX_GPREG_R3		U(0xC)
#define CTX_LR			U(0x10)
#define CTX_SCR			U(0x14)
#define CTX_SPSR		U(0x18)
#define CTX_NS_SCTLR		U(0x1C)
#define CTX_REGS_END		U(0x20)

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <lib/cassert.h>

/*
 * Common constants to help define the 'cpu_context' structure and its
 * members below.
 */
#define WORD_SHIFT		U(2)
#define DEFINE_REG_STRUCT(name, num_regs)	\
	typedef struct name {			\
		uint32_t _regs[num_regs];	\
	}  __aligned(8) name##_t

/* Constants to determine the size of individual context structures */
#define CTX_REG_ALL		(CTX_REGS_END >> WORD_SHIFT)

DEFINE_REG_STRUCT(regs, CTX_REG_ALL);

#undef CTX_REG_ALL

#define read_ctx_reg(ctx, offset)	((ctx)->_regs[offset >> WORD_SHIFT])
#define write_ctx_reg(ctx, offset, val)	(((ctx)->_regs[offset >> WORD_SHIFT]) \
					 = val)
typedef struct cpu_context {
	regs_t regs_ctx;
} cpu_context_t;

/* Macros to access members of the 'cpu_context_t' structure */
#define get_regs_ctx(h)		(&((cpu_context_t *) h)->regs_ctx)

/*
 * Compile time assertions related to the 'cpu_context' structure to
 * ensure that the assembler and the compiler view of the offsets of
 * the structure members is the same.
 */
CASSERT(CTX_REGS_OFFSET == __builtin_offsetof(cpu_context_t, regs_ctx), \
	assert_core_context_regs_offset_mismatch);

#endif /* __ASSEMBLER__ */

#endif /* CONTEXT_H */
