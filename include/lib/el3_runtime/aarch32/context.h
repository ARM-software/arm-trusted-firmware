/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

/*******************************************************************************
 * Constants that allow assembler code to access members of and the 'regs'
 * structure at their correct offsets.
 ******************************************************************************/
#define CTX_REGS_OFFSET		0x0
#define CTX_GPREG_R0		0x0
#define CTX_GPREG_R1		0x4
#define CTX_GPREG_R2		0x8
#define CTX_GPREG_R3		0xC
#define CTX_LR			0x10
#define CTX_SCR			0x14
#define CTX_SPSR		0x18
#define CTX_NS_SCTLR		0x1C
#define CTX_REGS_END		0x20

#ifndef __ASSEMBLY__

#include <cassert.h>
#include <stdint.h>

/*
 * Common constants to help define the 'cpu_context' structure and its
 * members below.
 */
#define WORD_SHIFT		2
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

#endif /* __ASSEMBLY__ */

#endif /* __CONTEXT_H__ */
