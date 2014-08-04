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

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

/*******************************************************************************
 * Constants that allow assembler code to access members of and the 'gp_regs'
 * structure at their correct offsets.
 ******************************************************************************/
#define CTX_GPREGS_OFFSET	0x0
#define CTX_GPREG_X0		0x0
#define CTX_GPREG_X1		0x8
#define CTX_GPREG_X2		0x10
#define CTX_GPREG_X3		0x18
#define CTX_GPREG_X4		0x20
#define CTX_GPREG_X5		0x28
#define CTX_GPREG_X6		0x30
#define CTX_GPREG_X7		0x38
#define CTX_GPREG_X8		0x40
#define CTX_GPREG_X9		0x48
#define CTX_GPREG_X10		0x50
#define CTX_GPREG_X11		0x58
#define CTX_GPREG_X12		0x60
#define CTX_GPREG_X13		0x68
#define CTX_GPREG_X14		0x70
#define CTX_GPREG_X15		0x78
#define CTX_GPREG_X16		0x80
#define CTX_GPREG_X17		0x88
#define CTX_GPREG_X18		0x90
#define CTX_GPREG_X19		0x98
#define CTX_GPREG_X20		0xa0
#define CTX_GPREG_X21		0xa8
#define CTX_GPREG_X22		0xb0
#define CTX_GPREG_X23		0xb8
#define CTX_GPREG_X24		0xc0
#define CTX_GPREG_X25		0xc8
#define CTX_GPREG_X26		0xd0
#define CTX_GPREG_X27		0xd8
#define CTX_GPREG_X28		0xe0
#define CTX_GPREG_X29		0xe8
#define CTX_GPREG_LR		0xf0
#define CTX_GPREG_SP_EL0	0xf8
#define CTX_GPREGS_END		0x100

/*******************************************************************************
 * Constants that allow assembler code to access members of and the 'el3_state'
 * structure at their correct offsets. Note that some of the registers are only
 * 32-bits wide but are stored as 64-bit values for convenience
 ******************************************************************************/
#define CTX_EL3STATE_OFFSET	(CTX_GPREGS_OFFSET + CTX_GPREGS_END)
#define CTX_SCR_EL3		0x0
#define CTX_RUNTIME_SP		0x8
#define CTX_SPSR_EL3		0x10
#define CTX_ELR_EL3		0x18
#define CTX_EL3STATE_END	0x20

/*******************************************************************************
 * Constants that allow assembler code to access members of and the
 * 'el1_sys_regs' structure at their correct offsets. Note that some of the
 * registers are only 32-bits wide but are stored as 64-bit values for
 * convenience
 ******************************************************************************/
#define CTX_SYSREGS_OFFSET	(CTX_EL3STATE_OFFSET + CTX_EL3STATE_END)
#define CTX_SPSR_EL1		0x0
#define CTX_ELR_EL1		0x8
#define CTX_SPSR_ABT		0x10
#define CTX_SPSR_UND		0x18
#define CTX_SPSR_IRQ		0x20
#define CTX_SPSR_FIQ		0x28
#define CTX_SCTLR_EL1		0x30
#define CTX_ACTLR_EL1		0x38
#define CTX_CPACR_EL1		0x40
#define CTX_CSSELR_EL1		0x48
#define CTX_SP_EL1		0x50
#define CTX_ESR_EL1		0x58
#define CTX_TTBR0_EL1		0x60
#define CTX_TTBR1_EL1		0x68
#define CTX_MAIR_EL1		0x70
#define CTX_AMAIR_EL1		0x78
#define CTX_TCR_EL1		0x80
#define CTX_TPIDR_EL1		0x88
#define CTX_TPIDR_EL0		0x90
#define CTX_TPIDRRO_EL0		0x98
#define CTX_DACR32_EL2		0xa0
#define CTX_IFSR32_EL2		0xa8
#define CTX_PAR_EL1		0xb0
#define CTX_FAR_EL1		0xb8
#define CTX_AFSR0_EL1		0xc0
#define CTX_AFSR1_EL1		0xc8
#define CTX_CONTEXTIDR_EL1	0xd0
#define CTX_VBAR_EL1		0xd8
/*
 * If the timer registers aren't saved and restored, we don't have to reserve
 * space for them in the context
 */
#if NS_TIMER_SWITCH
#define CTX_CNTP_CTL_EL0	0xe0
#define CTX_CNTP_CVAL_EL0	0xe8
#define CTX_CNTV_CTL_EL0	0xf0
#define CTX_CNTV_CVAL_EL0	0xf8
#define CTX_CNTKCTL_EL1		0x100
#define CTX_FP_FPEXC32_EL2	0x108
#define CTX_SYSREGS_END		0x110
#else
#define CTX_FP_FPEXC32_EL2	0xe0
#define CTX_SYSREGS_END		0xf0
#endif

/*******************************************************************************
 * Constants that allow assembler code to access members of and the 'fp_regs'
 * structure at their correct offsets.
 ******************************************************************************/
#if CTX_INCLUDE_FPREGS
#define CTX_FPREGS_OFFSET	(CTX_SYSREGS_OFFSET + CTX_SYSREGS_END)
#define CTX_FP_Q0		0x0
#define CTX_FP_Q1		0x10
#define CTX_FP_Q2		0x20
#define CTX_FP_Q3		0x30
#define CTX_FP_Q4		0x40
#define CTX_FP_Q5		0x50
#define CTX_FP_Q6		0x60
#define CTX_FP_Q7		0x70
#define CTX_FP_Q8		0x80
#define CTX_FP_Q9		0x90
#define CTX_FP_Q10		0xa0
#define CTX_FP_Q11		0xb0
#define CTX_FP_Q12		0xc0
#define CTX_FP_Q13		0xd0
#define CTX_FP_Q14		0xe0
#define CTX_FP_Q15		0xf0
#define CTX_FP_Q16		0x100
#define CTX_FP_Q17		0x110
#define CTX_FP_Q18		0x120
#define CTX_FP_Q19		0x130
#define CTX_FP_Q20		0x140
#define CTX_FP_Q21		0x150
#define CTX_FP_Q22		0x160
#define CTX_FP_Q23		0x170
#define CTX_FP_Q24		0x180
#define CTX_FP_Q25		0x190
#define CTX_FP_Q26		0x1a0
#define CTX_FP_Q27		0x1b0
#define CTX_FP_Q28		0x1c0
#define CTX_FP_Q29		0x1d0
#define CTX_FP_Q30		0x1e0
#define CTX_FP_Q31		0x1f0
#define CTX_FP_FPSR		0x200
#define CTX_FP_FPCR		0x208
#define CTX_FPREGS_END		0x210
#endif

#ifndef __ASSEMBLY__

#include <cassert.h>
#include <platform_def.h>	/* for CACHE_WRITEBACK_GRANULE */
#include <stdint.h>

/*
 * Common constants to help define the 'cpu_context' structure and its
 * members below.
 */
#define DWORD_SHIFT		3
#define DEFINE_REG_STRUCT(name, num_regs)	\
	typedef struct name {			\
		uint64_t _regs[num_regs];	\
	}  __aligned(16) name##_t

/* Constants to determine the size of individual context structures */
#define CTX_GPREG_ALL		(CTX_GPREGS_END >> DWORD_SHIFT)
#define CTX_SYSREG_ALL		(CTX_SYSREGS_END >> DWORD_SHIFT)
#if CTX_INCLUDE_FPREGS
#define CTX_FPREG_ALL		(CTX_FPREGS_END >> DWORD_SHIFT)
#endif
#define CTX_EL3STATE_ALL	(CTX_EL3STATE_END >> DWORD_SHIFT)

/*
 * AArch64 general purpose register context structure. Usually x0-x18,
 * lr are saved as the compiler is expected to preserve the remaining
 * callee saved registers if used by the C runtime and the assembler
 * does not touch the remaining. But in case of world switch during
 * exception handling, we need to save the callee registers too.
 */
DEFINE_REG_STRUCT(gp_regs, CTX_GPREG_ALL);

/*
 * AArch64 EL1 system register context structure for preserving the
 * architectural state during switches from one security state to
 * another in EL1.
 */
DEFINE_REG_STRUCT(el1_sys_regs, CTX_SYSREG_ALL);

/*
 * AArch64 floating point register context structure for preserving
 * the floating point state during switches from one security state to
 * another.
 */
#if CTX_INCLUDE_FPREGS
DEFINE_REG_STRUCT(fp_regs, CTX_FPREG_ALL);
#endif

/*
 * Miscellaneous registers used by EL3 firmware to maintain its state
 * across exception entries and exits
 */
DEFINE_REG_STRUCT(el3_state, CTX_EL3STATE_ALL);

/*
 * Macros to access members of any of the above structures using their
 * offsets
 */
#define read_ctx_reg(ctx, offset)	((ctx)->_regs[offset >> DWORD_SHIFT])
#define write_ctx_reg(ctx, offset, val)	(((ctx)->_regs[offset >> DWORD_SHIFT]) \
					 = val)

/*
 * Top-level context structure which is used by EL3 firmware to
 * preserve the state of a core at EL1 in one of the two security
 * states and save enough EL3 meta data to be able to return to that
 * EL and security state. The context management library will be used
 * to ensure that SP_EL3 always points to an instance of this
 * structure at exception entry and exit. Each instance will
 * correspond to either the secure or the non-secure state.
 */
typedef struct cpu_context {
	gp_regs_t gpregs_ctx;
	el3_state_t el3state_ctx;
	el1_sys_regs_t sysregs_ctx;
#if CTX_INCLUDE_FPREGS
	fp_regs_t fpregs_ctx;
#endif
} cpu_context_t;

/* Macros to access members of the 'cpu_context_t' structure */
#define get_el3state_ctx(h)	(&((cpu_context_t *) h)->el3state_ctx)
#if CTX_INCLUDE_FPREGS
#define get_fpregs_ctx(h)	(&((cpu_context_t *) h)->fpregs_ctx)
#endif
#define get_sysregs_ctx(h)	(&((cpu_context_t *) h)->sysregs_ctx)
#define get_gpregs_ctx(h)	(&((cpu_context_t *) h)->gpregs_ctx)

/*
 * Compile time assertions related to the 'cpu_context' structure to
 * ensure that the assembler and the compiler view of the offsets of
 * the structure members is the same.
 */
CASSERT(CTX_GPREGS_OFFSET == __builtin_offsetof(cpu_context_t, gpregs_ctx), \
	assert_core_context_gp_offset_mismatch);
CASSERT(CTX_SYSREGS_OFFSET == __builtin_offsetof(cpu_context_t, sysregs_ctx), \
	assert_core_context_sys_offset_mismatch);
#if CTX_INCLUDE_FPREGS
CASSERT(CTX_FPREGS_OFFSET == __builtin_offsetof(cpu_context_t, fpregs_ctx), \
	assert_core_context_fp_offset_mismatch);
#endif
CASSERT(CTX_EL3STATE_OFFSET == __builtin_offsetof(cpu_context_t, el3state_ctx), \
	assert_core_context_el3state_offset_mismatch);

/*
 * Helper macro to set the general purpose registers that correspond to
 * parameters in an aapcs_64 call i.e. x0-x7
 */
#define set_aapcs_args0(ctx, x0)				do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X0, x0);	\
	} while (0);
#define set_aapcs_args1(ctx, x0, x1)				do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X1, x1);	\
		set_aapcs_args0(ctx, x0);				\
	} while (0);
#define set_aapcs_args2(ctx, x0, x1, x2)			do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X2, x2);	\
		set_aapcs_args1(ctx, x0, x1);				\
	} while (0);
#define set_aapcs_args3(ctx, x0, x1, x2, x3)			do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X3, x3);	\
		set_aapcs_args2(ctx, x0, x1, x2);			\
	} while (0);
#define set_aapcs_args4(ctx, x0, x1, x2, x3, x4)		do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X4, x4);	\
		set_aapcs_args3(ctx, x0, x1, x2, x3);			\
	} while (0);
#define set_aapcs_args5(ctx, x0, x1, x2, x3, x4, x5)		do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X5, x5);	\
		set_aapcs_args4(ctx, x0, x1, x2, x3, x4);		\
	} while (0);
#define set_aapcs_args6(ctx, x0, x1, x2, x3, x4, x5, x6)	do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X6, x6);	\
		set_aapcs_args5(ctx, x0, x1, x2, x3, x4, x5);		\
	} while (0);
#define set_aapcs_args7(ctx, x0, x1, x2, x3, x4, x5, x6, x7)	do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X7, x7);	\
		set_aapcs_args6(ctx, x0, x1, x2, x3, x4, x5, x6);	\
	} while (0);

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
void el1_sysregs_context_save(el1_sys_regs_t *regs);
void el1_sysregs_context_restore(el1_sys_regs_t *regs);
#if CTX_INCLUDE_FPREGS
void fpregs_context_save(fp_regs_t *regs);
void fpregs_context_restore(fp_regs_t *regs);
#endif


#undef CTX_SYSREG_ALL
#if CTX_INCLUDE_FPREGS
#undef CTX_FPREG_ALL
#endif
#undef CTX_GPREG_ALL
#undef CTX_EL3STATE_ALL

#endif /* __ASSEMBLY__ */

#endif /* __CONTEXT_H__ */
