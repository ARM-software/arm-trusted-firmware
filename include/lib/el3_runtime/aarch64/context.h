/*
 * Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#if (CTX_INCLUDE_EL2_REGS && IMAGE_BL31)
#include <lib/el3_runtime/context_el2.h>
#else
/**
 * El1 context is required either when:
 * IMAGE_BL1 || ((!CTX_INCLUDE_EL2_REGS) && IMAGE_BL31)
 */
#include <lib/el3_runtime/context_el1.h>
#endif /* (CTX_INCLUDE_EL2_REGS && IMAGE_BL31) */

#include <lib/el3_runtime/simd_ctx.h>
#include <lib/utils_def.h>
#include <platform_def.h> /* For CACHE_WRITEBACK_GRANULE */

#define	CPU_CONTEXT_SECURE	UL(0)
#define	CPU_CONTEXT_NS		UL(1)
#if ENABLE_RME
#define	CPU_CONTEXT_REALM	UL(2)
#define	CPU_CONTEXT_NUM		UL(3)
#else
#define	CPU_CONTEXT_NUM		UL(2)
#endif

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
#define CTX_RUNTIME_SP		U(0x8)
#define CTX_SPSR_EL3		U(0x10)
#define CTX_ELR_EL3		U(0x18)
#define CTX_PMCR_EL0		U(0x20)
#define CTX_IS_IN_EL3		U(0x28)
#define CTX_MDCR_EL3		U(0x30)
/* Constants required in supporting nested exception in EL3 */
#define CTX_SAVED_ELR_EL3	U(0x38)
/*
 * General purpose flag, to save various EL3 states
 * FFH mode : Used to identify if handling nested exception
 * KFH mode : Used as counter value
 */
#define CTX_NESTED_EA_FLAG	U(0x40)
#if FFH_SUPPORT
 #define CTX_SAVED_ESR_EL3	U(0x48)
 #define CTX_SAVED_SPSR_EL3	U(0x50)
 #define CTX_SAVED_GPREG_LR	U(0x58)
 #define CTX_DOUBLE_FAULT_ESR	U(0x60)
 #define CTX_EL3STATE_END	U(0x70) /* Align to the next 16 byte boundary */
#else
 #define CTX_EL3STATE_END	U(0x50) /* Align to the next 16 byte boundary */
#endif /* FFH_SUPPORT */


/*******************************************************************************
 * Registers related to CVE-2018-3639
 ******************************************************************************/
#define CTX_CVE_2018_3639_OFFSET	(CTX_EL3STATE_OFFSET + CTX_EL3STATE_END)
#define CTX_CVE_2018_3639_DISABLE	U(0)
#define CTX_CVE_2018_3639_END		U(0x10) /* Align to the next 16 byte boundary */

/*******************************************************************************
 * Registers related to ERRATA_SPECULATIVE_AT
 *
 * This is essential as with EL1 and EL2 context registers being decoupled,
 * both will not be present for a given build configuration.
 * As ERRATA_SPECULATIVE_AT errata requires SCTLR_EL1 and TCR_EL1 registers
 * independent of the above logic, we need explicit context entries to be
 * reserved for these registers.
 *
 * NOTE: Based on this we end up with following different configurations depending
 * on the presence of errata and inclusion of EL1 or EL2 context.
 *
 * ============================================================================
 * | ERRATA_SPECULATIVE_AT | EL1 context| Memory allocation(Sctlr_el1,Tcr_el1)|
 * ============================================================================
 * |        0              |      0     |            None                     |
 * |        0              |      1     |    EL1 C-Context structure          |
 * |        1              |      0     |    Errata Context Offset Entries    |
 * |        1              |      1     |    Errata Context Offset Entries    |
 * ============================================================================
 *
 * In the above table, when ERRATA_SPECULATIVE_AT=1, EL1_Context=0, it implies
 * there is only EL2 context and memory for SCTLR_EL1 and TCR_EL1 registers is
 * reserved explicitly under ERRATA_SPECULATIVE_AT build flag here.
 *
 * In situations when EL1_Context=1 and  ERRATA_SPECULATIVE_AT=1, since SCTLR_EL1
 * and TCR_EL1 registers will be modified under errata and it happens at the
 * early in the codeflow prior to el1 context (save and restore operations),
 * context memory still will be reserved under the errata logic here explicitly.
 * These registers will not be part of EL1 context save & restore routines.
 *
 * Only when ERRATA_SPECULATIVE_AT=0, EL1_Context=1, for this combination,
 * SCTLR_EL1 and TCR_EL1 will be part of EL1 context structure (context_el1.h)
 * -----------------------------------------------------------------------------
 ******************************************************************************/
#define CTX_ERRATA_SPEC_AT_OFFSET	(CTX_CVE_2018_3639_OFFSET + CTX_CVE_2018_3639_END)
#if ERRATA_SPECULATIVE_AT
#define CTX_ERRATA_SPEC_AT_SCTLR_EL1	U(0x0)
#define CTX_ERRATA_SPEC_AT_TCR_EL1	U(0x8)
#define CTX_ERRATA_SPEC_AT_END		U(0x10) /* Align to the next 16 byte boundary */
#else
#define CTX_ERRATA_SPEC_AT_END		U(0x0)
#endif /* ERRATA_SPECULATIVE_AT */

/*******************************************************************************
 * Registers related to ARMv8.3-PAuth.
 ******************************************************************************/
#define CTX_PAUTH_REGS_OFFSET	(CTX_ERRATA_SPEC_AT_OFFSET + CTX_ERRATA_SPEC_AT_END)
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

/*******************************************************************************
 * Registers related to Morello.
 ******************************************************************************/
#define CTX_DDC_OFFSET	(CTX_PAUTH_REGS_OFFSET + CTX_PAUTH_REGS_END)
#if ENABLE_FEAT_MORELLO
#define CTX_DDC_EL0	U(0x0)
#define CTX_DDC_END	U(0x10) /* Align to the next 16 byte boundary */
#else
#define CTX_DDC_END	U(0)
#endif /* ENABLE_FEAT_MORELLO */

/*******************************************************************************
 * Registers initialised in a per-world context.
 ******************************************************************************/
#define CTX_CPTR_EL3			U(0x0)
#define CTX_MPAM3_EL3			U(0x8)
#if (ENABLE_FEAT_IDTE3 && IMAGE_BL31)
#define CTX_IDREGS_EL3			U(0x10)
#define CTX_PERWORLD_EL3STATE_END	U(0x78)
#else
#define CTX_PERWORLD_EL3STATE_END	U(0x10)
#endif /* ENABLE_FEAT_IDTE3 && IMAGE_BL31 */

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <assert.h>
#include <common/ep_info.h>
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

#define CTX_EL3STATE_ALL	(CTX_EL3STATE_END >> DWORD_SHIFT)
#define CTX_CVE_2018_3639_ALL	(CTX_CVE_2018_3639_END >> DWORD_SHIFT)

#if ERRATA_SPECULATIVE_AT
#define CTX_ERRATA_SPEC_AT_ALL	(CTX_ERRATA_SPEC_AT_END >> DWORD_SHIFT)
#endif
#if CTX_INCLUDE_PAUTH_REGS
# define CTX_PAUTH_REGS_ALL	(CTX_PAUTH_REGS_END >> DWORD_SHIFT)
#endif

#define CTX_DDC_ALL (CTX_DDC_END >> DWORD_SHIFT)

/*
 * AArch64 general purpose register context structure. Usually x0-x18,
 * lr are saved as the compiler is expected to preserve the remaining
 * callee saved registers if used by the C runtime and the assembler
 * does not touch the remaining. But in case of world switch during
 * exception handling, we need to save the callee registers too.
 */
DEFINE_REG_STRUCT(gp_regs, CTX_GPREG_ALL);

/*
 * Miscellaneous registers used by EL3 firmware to maintain its state
 * across exception entries and exits
 */
DEFINE_REG_STRUCT(el3_state, CTX_EL3STATE_ALL);

/* Function pointer used by CVE-2018-3639 dynamic mitigation */
DEFINE_REG_STRUCT(cve_2018_3639, CTX_CVE_2018_3639_ALL);

/* Registers associated to Errata_Speculative */
#if ERRATA_SPECULATIVE_AT
DEFINE_REG_STRUCT(errata_speculative_at, CTX_ERRATA_SPEC_AT_ALL);
#endif

/* Registers associated to ARMv8.3-PAuth */
#if CTX_INCLUDE_PAUTH_REGS
DEFINE_REG_STRUCT(pauth, CTX_PAUTH_REGS_ALL);
#endif

/* Registers associated with Morello */
typedef void *__capability ddc_cap_t;

/*
 * Macros to access members of any of the above structures using their
 * offsets
 */
#define read_ctx_reg(ctx, offset)	((ctx)->ctx_regs[(offset) >> DWORD_SHIFT])
#define write_ctx_reg(ctx, offset, val)	(((ctx)->ctx_regs[(offset) >> DWORD_SHIFT]) \
					 = (uint64_t) (val))

#if ENABLE_FEAT_IDTE3
typedef struct perworld_idreg {
	u_register_t id_aa64pfr0_el1;
	u_register_t id_aa64pfr1_el1;
	u_register_t id_aa64pfr2_el1;
	u_register_t id_aa64smfr0_el1;
	u_register_t id_aa64isar0_el1;
	u_register_t id_aa64isar1_el1;
	u_register_t id_aa64isar2_el1;
	u_register_t id_aa64isar3_el1;
	u_register_t id_aa64mmfr0_el1;
	u_register_t id_aa64mmfr1_el1;
	u_register_t id_aa64mmfr2_el1;
	u_register_t id_aa64mmfr3_el1;
	u_register_t id_aa64mmfr4_el1;
} perworld_idregs_t;
#endif

typedef struct world_amu_regs {
	uint64_t amevcntr02_el0;
	uint64_t amevcntr03_el0;
} world_amu_regs_t;

/*
 * Top-level context structure which is used by EL3 firmware to preserve
 * the state of a core at the next lower EL in a given security state and
 * save enough EL3 meta data to be able to return to that EL and security
 * state. The context management library will be used to ensure that
 * SP_EL3 always points to an instance of this structure at exception
 * entry and exit.
 */
typedef struct cpu_context {
	gp_regs_t gpregs_ctx;
	el3_state_t el3state_ctx;

	cve_2018_3639_t cve_2018_3639_ctx;

#if ERRATA_SPECULATIVE_AT
	errata_speculative_at_t errata_speculative_at_ctx;
#endif

#if CTX_INCLUDE_PAUTH_REGS
	pauth_t pauth_ctx;
#endif

	ddc_cap_t ddc_el0;

#if (CTX_INCLUDE_EL2_REGS && IMAGE_BL31)
	el2_sysregs_t el2_sysregs_ctx;
#else
	/* El1 context should be included only either for IMAGE_BL1,
	 * or for IMAGE_BL31 when CTX_INCLUDE_EL2_REGS=0:
	 * When SPMD_SPM_AT_SEL2=1, SPMC at S-EL2 takes care of saving
	 * and restoring EL1 registers. In this case, BL31 at EL3 can
	 * exclude save and restore of EL1 context registers.
	 */
	el1_sysregs_t el1_sysregs_ctx;
#endif

	/* TODO: the CACHE_WRITEBACK_GRANULE alignment is not necessary if this is
	 * contained in a per-cpu data structure (i.e. cpu_data_t).
	 */
} __aligned(CACHE_WRITEBACK_GRANULE) cpu_context_t;

/*
 * Per-World Context.
 * It stores registers whose values can be shared across CPUs.
 */
typedef struct per_world_context {
	uint64_t ctx_cptr_el3;
	uint64_t ctx_mpam3_el3;
#if (ENABLE_FEAT_IDTE3 && IMAGE_BL31)
	perworld_idregs_t idregs;
#endif
} per_world_context_t;

static inline uint8_t get_cpu_context_index(size_t security_state)
{
	if (security_state == SECURE) {
		return CPU_CONTEXT_SECURE;
#if ENABLE_RME
	} else  if (security_state == REALM) {
		return CPU_CONTEXT_REALM;
#endif
	} else {
		assert(security_state == NON_SECURE);
		return CPU_CONTEXT_NS;
	}
}

extern per_world_context_t per_world_context[CPU_CONTEXT_NUM];

/* Macros to access members of the 'cpu_context_t' structure */
#define get_el3state_ctx(h)	(&((cpu_context_t *) h)->el3state_ctx)

#if (CTX_INCLUDE_EL2_REGS && IMAGE_BL31)
#define get_el2_sysregs_ctx(h)	(&((cpu_context_t *) h)->el2_sysregs_ctx)
#else
#define get_el1_sysregs_ctx(h)	(&((cpu_context_t *) h)->el1_sysregs_ctx)
#endif

#define get_gpregs_ctx(h)	(&((cpu_context_t *) h)->gpregs_ctx)
#define get_cve_2018_3639_ctx(h)	(&((cpu_context_t *) h)->cve_2018_3639_ctx)

#if ERRATA_SPECULATIVE_AT
#define get_errata_speculative_at_ctx(h)	(&((cpu_context_t *) h)->errata_speculative_at_ctx)
#endif

#if CTX_INCLUDE_PAUTH_REGS
# define get_pauth_ctx(h)	(&((cpu_context_t *) h)->pauth_ctx)
#else
# define get_pauth_ctx(h)	NULL
#endif

/*
 * Compile time assertions related to the 'cpu_context' structure to
 * ensure that the assembler and the compiler view of the offsets of
 * the structure members is the same.
 */
CASSERT(CTX_GPREGS_OFFSET == __builtin_offsetof(cpu_context_t, gpregs_ctx),
	assert_core_context_gp_offset_mismatch);

CASSERT(CTX_EL3STATE_OFFSET == __builtin_offsetof(cpu_context_t, el3state_ctx),
	assert_core_context_el3state_offset_mismatch);


CASSERT(CTX_CVE_2018_3639_OFFSET == __builtin_offsetof(cpu_context_t, cve_2018_3639_ctx),
	assert_core_context_cve_2018_3639_offset_mismatch);

#if ERRATA_SPECULATIVE_AT
CASSERT(CTX_ERRATA_SPEC_AT_OFFSET == __builtin_offsetof(cpu_context_t, errata_speculative_at_ctx),
	assert_core_context_errata_speculative_at_offset_mismatch);
#endif

#if CTX_INCLUDE_PAUTH_REGS
CASSERT(CTX_PAUTH_REGS_OFFSET == __builtin_offsetof(cpu_context_t, pauth_ctx),
	assert_core_context_pauth_offset_mismatch);
#endif /* CTX_INCLUDE_PAUTH_REGS */

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
#if CTX_INCLUDE_FPREGS
void fpregs_context_save(simd_regs_t *regs);
void fpregs_context_restore(simd_regs_t *regs);
#endif

/*******************************************************************************
 * The next four inline functions are required for IMAGE_BL1, as well as for
 * IMAGE_BL31 for the below combinations.
 * ============================================================================
 * | ERRATA_SPECULATIVE_AT| CTX_INCLUDE_EL2_REGS |   Combination              |
 * ============================================================================
 * |       0              |       0              |   Valid (EL1 ctx)          |
 * |______________________|______________________|____________________________|
 * |                      |                      | Invalid (No Errata/EL1 Ctx)|
 * |       0              |       1              | Hence commented out.       |
 * |______________________|______________________|____________________________|
 * |                      |                      |                            |
 * |       1              |       0              |   Valid (Errata ctx)       |
 * |______________________|______________________|____________________________|
 * |                      |                      |                            |
 * |       1              |       1              |   Valid (Errata ctx)       |
 * |______________________|______________________|____________________________|
 * ============================================================================
 ******************************************************************************/
#if (IMAGE_BL1 || ((ERRATA_SPECULATIVE_AT) || (!CTX_INCLUDE_EL2_REGS)))

static inline void write_ctx_sctlr_el1_reg_errata(cpu_context_t *ctx, u_register_t val)
{
#if (ERRATA_SPECULATIVE_AT)
	write_ctx_reg(get_errata_speculative_at_ctx(ctx),
		      CTX_ERRATA_SPEC_AT_SCTLR_EL1, val);
#else
	write_el1_ctx_common(get_el1_sysregs_ctx(ctx), sctlr_el1, val);
#endif /* ERRATA_SPECULATIVE_AT */
}

static inline void write_ctx_tcr_el1_reg_errata(cpu_context_t *ctx, u_register_t val)
{
#if (ERRATA_SPECULATIVE_AT)
	write_ctx_reg(get_errata_speculative_at_ctx(ctx),
		      CTX_ERRATA_SPEC_AT_TCR_EL1, val);
#else
	write_el1_ctx_common(get_el1_sysregs_ctx(ctx), tcr_el1, val);
#endif /* ERRATA_SPECULATIVE_AT */
}

static inline u_register_t read_ctx_sctlr_el1_reg_errata(cpu_context_t *ctx)
{
#if (ERRATA_SPECULATIVE_AT)
	return read_ctx_reg(get_errata_speculative_at_ctx(ctx),
		      CTX_ERRATA_SPEC_AT_SCTLR_EL1);
#else
	return read_el1_ctx_common(get_el1_sysregs_ctx(ctx), sctlr_el1);
#endif /* ERRATA_SPECULATIVE_AT */
}

static inline u_register_t read_ctx_tcr_el1_reg_errata(cpu_context_t *ctx)
{
#if (ERRATA_SPECULATIVE_AT)
	return read_ctx_reg(get_errata_speculative_at_ctx(ctx),
		      CTX_ERRATA_SPEC_AT_TCR_EL1);
#else
	return read_el1_ctx_common(get_el1_sysregs_ctx(ctx), tcr_el1);
#endif /* ERRATA_SPECULATIVE_AT */
}

#endif /* (IMAGE_BL1 || ((ERRATA_SPECULATIVE_AT) || (!CTX_INCLUDE_EL2_REGS))) */

#endif /* __ASSEMBLER__ */

#endif /* CONTEXT_H */
