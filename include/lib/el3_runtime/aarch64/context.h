/*
 * Copyright (c) 2013-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include <lib/utils_def.h>

/*******************************************************************************
 * Constants that allow assembler code to access members of and the 'gp_regs'
 * structure at their correct offsets.
 ******************************************************************************/
#define CTX_GPREGS_OFFSET	U(0x0)
#define CTX_GPREG_X0		U(0x0)
#define CTX_GPREG_X1		U(0x8)
#define CTX_GPREG_X2		U(0x10)
#define CTX_GPREG_X3		U(0x18)
#define CTX_GPREG_X4		U(0x20)
#define CTX_GPREG_X5		U(0x28)
#define CTX_GPREG_X6		U(0x30)
#define CTX_GPREG_X7		U(0x38)
#define CTX_GPREG_X8		U(0x40)
#define CTX_GPREG_X9		U(0x48)
#define CTX_GPREG_X10		U(0x50)
#define CTX_GPREG_X11		U(0x58)
#define CTX_GPREG_X12		U(0x60)
#define CTX_GPREG_X13		U(0x68)
#define CTX_GPREG_X14		U(0x70)
#define CTX_GPREG_X15		U(0x78)
#define CTX_GPREG_X16		U(0x80)
#define CTX_GPREG_X17		U(0x88)
#define CTX_GPREG_X18		U(0x90)
#define CTX_GPREG_X19		U(0x98)
#define CTX_GPREG_X20		U(0xa0)
#define CTX_GPREG_X21		U(0xa8)
#define CTX_GPREG_X22		U(0xb0)
#define CTX_GPREG_X23		U(0xb8)
#define CTX_GPREG_X24		U(0xc0)
#define CTX_GPREG_X25		U(0xc8)
#define CTX_GPREG_X26		U(0xd0)
#define CTX_GPREG_X27		U(0xd8)
#define CTX_GPREG_X28		U(0xe0)
#define CTX_GPREG_X29		U(0xe8)
#define CTX_GPREG_LR		U(0xf0)
#define CTX_GPREG_SP_EL0	U(0xf8)
#define CTX_GPREGS_END		U(0x100)

/*******************************************************************************
 * Constants that allow assembler code to access members of and the 'el3_state'
 * structure at their correct offsets. Note that some of the registers are only
 * 32-bits wide but are stored as 64-bit values for convenience
 ******************************************************************************/
#define CTX_EL3STATE_OFFSET	(CTX_GPREGS_OFFSET + CTX_GPREGS_END)
#define CTX_SCR_EL3		U(0x0)
#define CTX_ESR_EL3		U(0x8)
#define CTX_RUNTIME_SP		U(0x10)
#define CTX_SPSR_EL3		U(0x18)
#define CTX_ELR_EL3		U(0x20)
#define CTX_PMCR_EL0		U(0x28)
#define CTX_IS_IN_EL3		U(0x30)
#define CTX_EL3STATE_END	U(0x40) /* Align to the next 16 byte boundary */

/*******************************************************************************
 * Constants that allow assembler code to access members of and the
 * 'el1_sys_regs' structure at their correct offsets. Note that some of the
 * registers are only 32-bits wide but are stored as 64-bit values for
 * convenience
 ******************************************************************************/
#define CTX_EL1_SYSREGS_OFFSET	(CTX_EL3STATE_OFFSET + CTX_EL3STATE_END)
#define CTX_SPSR_EL1		U(0x0)
#define CTX_ELR_EL1		U(0x8)
#define CTX_SCTLR_EL1		U(0x10)
#define CTX_TCR_EL1		U(0x18)
#define CTX_CPACR_EL1		U(0x20)
#define CTX_CSSELR_EL1		U(0x28)
#define CTX_SP_EL1		U(0x30)
#define CTX_ESR_EL1		U(0x38)
#define CTX_TTBR0_EL1		U(0x40)
#define CTX_TTBR1_EL1		U(0x48)
#define CTX_MAIR_EL1		U(0x50)
#define CTX_AMAIR_EL1		U(0x58)
#define CTX_ACTLR_EL1		U(0x60)
#define CTX_TPIDR_EL1		U(0x68)
#define CTX_TPIDR_EL0		U(0x70)
#define CTX_TPIDRRO_EL0		U(0x78)
#define CTX_PAR_EL1		U(0x80)
#define CTX_FAR_EL1		U(0x88)
#define CTX_AFSR0_EL1		U(0x90)
#define CTX_AFSR1_EL1		U(0x98)
#define CTX_CONTEXTIDR_EL1	U(0xa0)
#define CTX_VBAR_EL1		U(0xa8)

/*
 * If the platform is AArch64-only, there is no need to save and restore these
 * AArch32 registers.
 */
#if CTX_INCLUDE_AARCH32_REGS
#define CTX_SPSR_ABT		U(0xb0)	/* Align to the next 16 byte boundary */
#define CTX_SPSR_UND		U(0xb8)
#define CTX_SPSR_IRQ		U(0xc0)
#define CTX_SPSR_FIQ		U(0xc8)
#define CTX_DACR32_EL2		U(0xd0)
#define CTX_IFSR32_EL2		U(0xd8)
#define CTX_AARCH32_END		U(0xe0) /* Align to the next 16 byte boundary */
#else
#define CTX_AARCH32_END		U(0xb0)	/* Align to the next 16 byte boundary */
#endif /* CTX_INCLUDE_AARCH32_REGS */

/*
 * If the timer registers aren't saved and restored, we don't have to reserve
 * space for them in the context
 */
#if NS_TIMER_SWITCH
#define CTX_CNTP_CTL_EL0	(CTX_AARCH32_END + U(0x0))
#define CTX_CNTP_CVAL_EL0	(CTX_AARCH32_END + U(0x8))
#define CTX_CNTV_CTL_EL0	(CTX_AARCH32_END + U(0x10))
#define CTX_CNTV_CVAL_EL0	(CTX_AARCH32_END + U(0x18))
#define CTX_CNTKCTL_EL1		(CTX_AARCH32_END + U(0x20))
#define CTX_TIMER_SYSREGS_END	(CTX_AARCH32_END + U(0x30)) /* Align to the next 16 byte boundary */
#else
#define CTX_TIMER_SYSREGS_END	CTX_AARCH32_END
#endif /* NS_TIMER_SWITCH */

#if CTX_INCLUDE_MTE_REGS
#define CTX_TFSRE0_EL1		(CTX_TIMER_SYSREGS_END + U(0x0))
#define CTX_TFSR_EL1		(CTX_TIMER_SYSREGS_END + U(0x8))
#define CTX_RGSR_EL1		(CTX_TIMER_SYSREGS_END + U(0x10))
#define CTX_GCR_EL1		(CTX_TIMER_SYSREGS_END + U(0x18))

/* Align to the next 16 byte boundary */
#define CTX_MTE_REGS_END	(CTX_TIMER_SYSREGS_END + U(0x20))
#else
#define CTX_MTE_REGS_END	CTX_TIMER_SYSREGS_END
#endif /* CTX_INCLUDE_MTE_REGS */

/*
 * End of system registers.
 */
#define CTX_EL1_SYSREGS_END		CTX_MTE_REGS_END

/*
 * EL2 register set
 */

#if CTX_INCLUDE_EL2_REGS
/* For later discussion
 * ICH_AP0R<n>_EL2
 * ICH_AP1R<n>_EL2
 * AMEVCNTVOFF0<n>_EL2
 * AMEVCNTVOFF1<n>_EL2
 * ICH_LR<n>_EL2
 */
#define CTX_EL2_SYSREGS_OFFSET	(CTX_EL1_SYSREGS_OFFSET + CTX_EL1_SYSREGS_END)

#define CTX_ACTLR_EL2		U(0x0)
#define CTX_AFSR0_EL2		U(0x8)
#define CTX_AFSR1_EL2		U(0x10)
#define CTX_AMAIR_EL2		U(0x18)
#define CTX_CNTHCTL_EL2		U(0x20)
#define CTX_CNTVOFF_EL2		U(0x28)
#define CTX_CPTR_EL2		U(0x30)
#define CTX_DBGVCR32_EL2	U(0x38)
#define CTX_ELR_EL2		U(0x40)
#define CTX_ESR_EL2		U(0x48)
#define CTX_FAR_EL2		U(0x50)
#define CTX_HACR_EL2		U(0x58)
#define CTX_HCR_EL2		U(0x60)
#define CTX_HPFAR_EL2		U(0x68)
#define CTX_HSTR_EL2		U(0x70)
#define CTX_ICC_SRE_EL2		U(0x78)
#define CTX_ICH_HCR_EL2		U(0x80)
#define CTX_ICH_VMCR_EL2	U(0x88)
#define CTX_MAIR_EL2		U(0x90)
#define CTX_MDCR_EL2		U(0x98)
#define CTX_PMSCR_EL2		U(0xa0)
#define CTX_SCTLR_EL2		U(0xa8)
#define CTX_SPSR_EL2		U(0xb0)
#define CTX_SP_EL2		U(0xb8)
#define CTX_TCR_EL2		U(0xc0)
#define CTX_TPIDR_EL2		U(0xc8)
#define CTX_TTBR0_EL2		U(0xd0)
#define CTX_VBAR_EL2		U(0xd8)
#define CTX_VMPIDR_EL2		U(0xe0)
#define CTX_VPIDR_EL2		U(0xe8)
#define CTX_VTCR_EL2		U(0xf0)
#define CTX_VTTBR_EL2		U(0xf8)

// Only if MTE registers in use
#define CTX_TFSR_EL2		U(0x100)

// Only if ENABLE_MPAM_FOR_LOWER_ELS==1
#define CTX_MPAM2_EL2		U(0x108)
#define CTX_MPAMHCR_EL2		U(0x110)
#define CTX_MPAMVPM0_EL2	U(0x118)
#define CTX_MPAMVPM1_EL2	U(0x120)
#define CTX_MPAMVPM2_EL2	U(0x128)
#define CTX_MPAMVPM3_EL2	U(0x130)
#define CTX_MPAMVPM4_EL2	U(0x138)
#define CTX_MPAMVPM5_EL2	U(0x140)
#define CTX_MPAMVPM6_EL2	U(0x148)
#define CTX_MPAMVPM7_EL2	U(0x150)
#define CTX_MPAMVPMV_EL2	U(0x158)

// Starting with Armv8.6
#define CTX_HAFGRTR_EL2		U(0x160)
#define CTX_HDFGRTR_EL2		U(0x168)
#define CTX_HDFGWTR_EL2		U(0x170)
#define CTX_HFGITR_EL2		U(0x178)
#define CTX_HFGRTR_EL2		U(0x180)
#define CTX_HFGWTR_EL2		U(0x188)
#define CTX_CNTPOFF_EL2		U(0x190)

// Starting with Armv8.4
#define CTX_CONTEXTIDR_EL2	U(0x198)
#define CTX_SDER32_EL2		U(0x1a0)
#define CTX_TTBR1_EL2		U(0x1a8)
#define CTX_VDISR_EL2		U(0x1b0)
#define CTX_VNCR_EL2		U(0x1b8)
#define CTX_VSESR_EL2		U(0x1c0)
#define CTX_VSTCR_EL2		U(0x1c8)
#define CTX_VSTTBR_EL2		U(0x1d0)
#define CTX_TRFCR_EL2		U(0x1d8)

// Starting with Armv8.5
#define CTX_SCXTNUM_EL2		U(0x1e0)
/* Align to the next 16 byte boundary */
#define CTX_EL2_SYSREGS_END	U(0x1f0)

#endif /* CTX_INCLUDE_EL2_REGS */

/*******************************************************************************
 * Constants that allow assembler code to access members of and the 'fp_regs'
 * structure at their correct offsets.
 ******************************************************************************/
#if CTX_INCLUDE_EL2_REGS
# define CTX_FPREGS_OFFSET	(CTX_EL2_SYSREGS_OFFSET + CTX_EL2_SYSREGS_END)
#else
# define CTX_FPREGS_OFFSET	(CTX_EL1_SYSREGS_OFFSET + CTX_EL1_SYSREGS_END)
#endif
#if CTX_INCLUDE_FPREGS
#define CTX_FP_Q0		U(0x0)
#define CTX_FP_Q1		U(0x10)
#define CTX_FP_Q2		U(0x20)
#define CTX_FP_Q3		U(0x30)
#define CTX_FP_Q4		U(0x40)
#define CTX_FP_Q5		U(0x50)
#define CTX_FP_Q6		U(0x60)
#define CTX_FP_Q7		U(0x70)
#define CTX_FP_Q8		U(0x80)
#define CTX_FP_Q9		U(0x90)
#define CTX_FP_Q10		U(0xa0)
#define CTX_FP_Q11		U(0xb0)
#define CTX_FP_Q12		U(0xc0)
#define CTX_FP_Q13		U(0xd0)
#define CTX_FP_Q14		U(0xe0)
#define CTX_FP_Q15		U(0xf0)
#define CTX_FP_Q16		U(0x100)
#define CTX_FP_Q17		U(0x110)
#define CTX_FP_Q18		U(0x120)
#define CTX_FP_Q19		U(0x130)
#define CTX_FP_Q20		U(0x140)
#define CTX_FP_Q21		U(0x150)
#define CTX_FP_Q22		U(0x160)
#define CTX_FP_Q23		U(0x170)
#define CTX_FP_Q24		U(0x180)
#define CTX_FP_Q25		U(0x190)
#define CTX_FP_Q26		U(0x1a0)
#define CTX_FP_Q27		U(0x1b0)
#define CTX_FP_Q28		U(0x1c0)
#define CTX_FP_Q29		U(0x1d0)
#define CTX_FP_Q30		U(0x1e0)
#define CTX_FP_Q31		U(0x1f0)
#define CTX_FP_FPSR		U(0x200)
#define CTX_FP_FPCR		U(0x208)
#if CTX_INCLUDE_AARCH32_REGS
#define CTX_FP_FPEXC32_EL2	U(0x210)
#define CTX_FPREGS_END		U(0x220) /* Align to the next 16 byte boundary */
#else
#define CTX_FPREGS_END		U(0x210) /* Align to the next 16 byte boundary */
#endif
#else
#define CTX_FPREGS_END		U(0)
#endif

/*******************************************************************************
 * Registers related to CVE-2018-3639
 ******************************************************************************/
#define CTX_CVE_2018_3639_OFFSET	(CTX_FPREGS_OFFSET + CTX_FPREGS_END)
#define CTX_CVE_2018_3639_DISABLE	U(0)
#define CTX_CVE_2018_3639_END		U(0x10) /* Align to the next 16 byte boundary */

/*******************************************************************************
 * Registers related to ARMv8.3-PAuth.
 ******************************************************************************/
#define CTX_PAUTH_REGS_OFFSET	(CTX_CVE_2018_3639_OFFSET + CTX_CVE_2018_3639_END)
#if CTX_INCLUDE_PAUTH_REGS
#define CTX_PACIAKEY_LO		U(0x0)
#define CTX_PACIAKEY_HI		U(0x8)
#define CTX_PACIBKEY_LO		U(0x10)
#define CTX_PACIBKEY_HI		U(0x18)
#define CTX_PACDAKEY_LO		U(0x20)
#define CTX_PACDAKEY_HI		U(0x28)
#define CTX_PACDBKEY_LO		U(0x30)
#define CTX_PACDBKEY_HI		U(0x38)
#define CTX_PACGAKEY_LO		U(0x40)
#define CTX_PACGAKEY_HI		U(0x48)
#define CTX_PAUTH_REGS_END	U(0x50) /* Align to the next 16 byte boundary */
#else
#define CTX_PAUTH_REGS_END	U(0)
#endif /* CTX_INCLUDE_PAUTH_REGS */

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <lib/cassert.h>

/*
 * Common constants to help define the 'cpu_context' structure and its
 * members below.
 */
#define DWORD_SHIFT		U(3)
#define DEFINE_REG_STRUCT(name, num_regs)	\
	typedef struct name {			\
		uint64_t ctx_regs[num_regs];	\
	}  __aligned(16) name##_t

/* Constants to determine the size of individual context structures */
#define CTX_GPREG_ALL		(CTX_GPREGS_END >> DWORD_SHIFT)
#define CTX_EL1_SYSREGS_ALL	(CTX_EL1_SYSREGS_END >> DWORD_SHIFT)
#if CTX_INCLUDE_EL2_REGS
# define CTX_EL2_SYSREGS_ALL	(CTX_EL2_SYSREGS_END >> DWORD_SHIFT)
#endif
#if CTX_INCLUDE_FPREGS
# define CTX_FPREG_ALL		(CTX_FPREGS_END >> DWORD_SHIFT)
#endif
#define CTX_EL3STATE_ALL	(CTX_EL3STATE_END >> DWORD_SHIFT)
#define CTX_CVE_2018_3639_ALL	(CTX_CVE_2018_3639_END >> DWORD_SHIFT)
#if CTX_INCLUDE_PAUTH_REGS
# define CTX_PAUTH_REGS_ALL	(CTX_PAUTH_REGS_END >> DWORD_SHIFT)
#endif

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
 * architectural state during world switches.
 */
DEFINE_REG_STRUCT(el1_sysregs, CTX_EL1_SYSREGS_ALL);


/*
 * AArch64 EL2 system register context structure for preserving the
 * architectural state during world switches.
 */
#if CTX_INCLUDE_EL2_REGS
DEFINE_REG_STRUCT(el2_sysregs, CTX_EL2_SYSREGS_ALL);
#endif

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

/* Function pointer used by CVE-2018-3639 dynamic mitigation */
DEFINE_REG_STRUCT(cve_2018_3639, CTX_CVE_2018_3639_ALL);

/* Registers associated to ARMv8.3-PAuth */
#if CTX_INCLUDE_PAUTH_REGS
DEFINE_REG_STRUCT(pauth, CTX_PAUTH_REGS_ALL);
#endif

/*
 * Macros to access members of any of the above structures using their
 * offsets
 */
#define read_ctx_reg(ctx, offset)	((ctx)->ctx_regs[(offset) >> DWORD_SHIFT])
#define write_ctx_reg(ctx, offset, val)	(((ctx)->ctx_regs[(offset) >> DWORD_SHIFT]) \
					 = (uint64_t) (val))

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
	el1_sysregs_t el1_sysregs_ctx;
#if CTX_INCLUDE_EL2_REGS
	el2_sysregs_t el2_sysregs_ctx;
#endif
#if CTX_INCLUDE_FPREGS
	fp_regs_t fpregs_ctx;
#endif
	cve_2018_3639_t cve_2018_3639_ctx;
#if CTX_INCLUDE_PAUTH_REGS
	pauth_t pauth_ctx;
#endif
} cpu_context_t;

/* Macros to access members of the 'cpu_context_t' structure */
#define get_el3state_ctx(h)	(&((cpu_context_t *) h)->el3state_ctx)
#if CTX_INCLUDE_FPREGS
# define get_fpregs_ctx(h)	(&((cpu_context_t *) h)->fpregs_ctx)
#endif
#define get_el1_sysregs_ctx(h)	(&((cpu_context_t *) h)->el1_sysregs_ctx)
#if CTX_INCLUDE_EL2_REGS
# define get_el2_sysregs_ctx(h)	(&((cpu_context_t *) h)->el2_sysregs_ctx)
#endif
#define get_gpregs_ctx(h)	(&((cpu_context_t *) h)->gpregs_ctx)
#define get_cve_2018_3639_ctx(h)	(&((cpu_context_t *) h)->cve_2018_3639_ctx)
#if CTX_INCLUDE_PAUTH_REGS
# define get_pauth_ctx(h)	(&((cpu_context_t *) h)->pauth_ctx)
#endif

/*
 * Compile time assertions related to the 'cpu_context' structure to
 * ensure that the assembler and the compiler view of the offsets of
 * the structure members is the same.
 */
CASSERT(CTX_GPREGS_OFFSET == __builtin_offsetof(cpu_context_t, gpregs_ctx), \
	assert_core_context_gp_offset_mismatch);
CASSERT(CTX_EL1_SYSREGS_OFFSET == __builtin_offsetof(cpu_context_t, el1_sysregs_ctx), \
	assert_core_context_el1_sys_offset_mismatch);
#if CTX_INCLUDE_EL2_REGS
CASSERT(CTX_EL2_SYSREGS_OFFSET == __builtin_offsetof(cpu_context_t, el2_sysregs_ctx), \
	assert_core_context_el2_sys_offset_mismatch);
#endif
#if CTX_INCLUDE_FPREGS
CASSERT(CTX_FPREGS_OFFSET == __builtin_offsetof(cpu_context_t, fpregs_ctx), \
	assert_core_context_fp_offset_mismatch);
#endif
CASSERT(CTX_EL3STATE_OFFSET == __builtin_offsetof(cpu_context_t, el3state_ctx), \
	assert_core_context_el3state_offset_mismatch);
CASSERT(CTX_CVE_2018_3639_OFFSET == __builtin_offsetof(cpu_context_t, cve_2018_3639_ctx), \
	assert_core_context_cve_2018_3639_offset_mismatch);
#if CTX_INCLUDE_PAUTH_REGS
CASSERT(CTX_PAUTH_REGS_OFFSET == __builtin_offsetof(cpu_context_t, pauth_ctx), \
	assert_core_context_pauth_offset_mismatch);
#endif

/*
 * Helper macro to set the general purpose registers that correspond to
 * parameters in an aapcs_64 call i.e. x0-x7
 */
#define set_aapcs_args0(ctx, x0)				do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X0, x0);	\
	} while (0)
#define set_aapcs_args1(ctx, x0, x1)				do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X1, x1);	\
		set_aapcs_args0(ctx, x0);				\
	} while (0)
#define set_aapcs_args2(ctx, x0, x1, x2)			do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X2, x2);	\
		set_aapcs_args1(ctx, x0, x1);				\
	} while (0)
#define set_aapcs_args3(ctx, x0, x1, x2, x3)			do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X3, x3);	\
		set_aapcs_args2(ctx, x0, x1, x2);			\
	} while (0)
#define set_aapcs_args4(ctx, x0, x1, x2, x3, x4)		do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X4, x4);	\
		set_aapcs_args3(ctx, x0, x1, x2, x3);			\
	} while (0)
#define set_aapcs_args5(ctx, x0, x1, x2, x3, x4, x5)		do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X5, x5);	\
		set_aapcs_args4(ctx, x0, x1, x2, x3, x4);		\
	} while (0)
#define set_aapcs_args6(ctx, x0, x1, x2, x3, x4, x5, x6)	do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X6, x6);	\
		set_aapcs_args5(ctx, x0, x1, x2, x3, x4, x5);		\
	} while (0)
#define set_aapcs_args7(ctx, x0, x1, x2, x3, x4, x5, x6, x7)	do {	\
		write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X7, x7);	\
		set_aapcs_args6(ctx, x0, x1, x2, x3, x4, x5, x6);	\
	} while (0)

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
void el1_sysregs_context_save(el1_sysregs_t *regs);
void el1_sysregs_context_restore(el1_sysregs_t *regs);

#if CTX_INCLUDE_EL2_REGS
void el2_sysregs_context_save(el2_sysregs_t *regs);
void el2_sysregs_context_restore(el2_sysregs_t *regs);
#endif

#if CTX_INCLUDE_FPREGS
void fpregs_context_save(fp_regs_t *regs);
void fpregs_context_restore(fp_regs_t *regs);
#endif

#endif /* __ASSEMBLER__ */

#endif /* CONTEXT_H */
