/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RMMD_INITIAL_CONTEXT_H
#define RMMD_INITIAL_CONTEXT_H

#include <arch.h>

#define NS_SCTLR_EL2		NS_SCTLR_EL2_RES1

#define REALM_ACTLR_EL2	0x0
#define REALM_AFSR0_EL2	0x0
#define REALM_AFSR1_EL2	0x0
#define REALM_AMAIR_EL2	0x0

#define REALM_CNTHCTL_EL2	(CNTHCTL_EL0PCTEN_BIT | CNTHCTL_EL0VCTEN_BIT)

#define REALM_CNTVOFF_EL2	0x0

/*
 * CPTR_EL2
 *   ZEN=1   Trap at EL0 but not at EL2
 *   FPEN=1  Trap at EL0 but not at EL2
 *   TTA=1   Trap trace access
 *   TA=1    Trap AMU access
 */
#define REALM_CPTR_EL2		(					\
					CPTR_EL2_ZEN_DISABLE_EL0	| \
					CPTR_EL2_FPEN_DISABLE_EL0	| \
					CPTR_EL2_TTA_BIT			| \
					CPTR_EL2_TAM_BIT			| \
					CPTR_EL2_RES1			\
				)

#define REALM_HACR_EL2		0x0

/*
 * HCR_EL2
 *   TGE=1
 *   E2H=1
 *   TEA=1
 */
#define REALM_HCR_EL2		(					\
					HCR_TGE_BIT			| \
					HCR_E2H_BIT			| \
					HCR_TEA_BIT			\
				)

#define NS_HCR_EL2		(					\
					HCR_API_BIT			| \
					HCR_APK_BIT			| \
					HCR_RW_BIT			\
				)

/* Attr0=0xFF indicates normal inner/outer write-back non-transient memory */
#define REALM_MAIR_EL2		(0xFF)

#define REALM_MDCR_EL2		(					\
					MDCR_EL2_TPMCR_BIT		| \
					MDCR_EL2_TPM_BIT		| \
					MDCR_EL2_TDA_BIT		\
				)

#define REALM_MPAM_EL2		0x0	/* Only if MPAM is present */

#define REALM_MPAMHCR_EL2	0x0	/* Only if MPAM is present */

#define REALM_PMSCR_EL2	0x0	/* Only if SPE is present */

/*
 * SCTLR_EL2
 *   M=0       MMU disabled.
 *   A=0       Alignment checks disabled.
 *   C=1       Data accesses are cacheable as per translation tables.
 *   SA=1      SP aligned at EL2.
 *   SA0=1     SP alignment check enable for EL0.
 *   CP15BEN=0 EL0 use of CP15xxx instructions UNDEFINED.
 *   NAA=0     Unaligned MA fault at EL2 and EL0.
 *   ITD=0     (A32 only)
 *   SED=1     A32 only, RES1 for non-A32 systems.
 *   EOS=0     ARMv8.5-CSEH, otherwise RES1.
 *   I=1       I$ is on for EL2 and EL0.
 *   DZE=1     Do not trap DC ZVA.
 *   UCT=1     Allow EL0 access to CTR_EL0.
 *   NTWI=1    Don't trap WFI from EL0 to EL2.
 *   NTWE=1    Don't trap WFE from EL0 to EL2.
 *   WXN=1     W implies XN.
 *   TSCXT=1    Trap EL0 accesses to SCXTNUM_EL0.
 *   EIS=0     EL2 exception is context synchronizing.
 *   SPAN=0    Set PSTATE.PAN=1 on exceptions to EL2.
 *   UCI=1     Allow cache maintenance instructions at EL0.
 *   nTLSMD=1  (A32/T32 only)
 *   LSMAOE=1  (A32/T32 only)
 */
#define REALM_SCTLR_EL2 	(					\
					SCTLR_C_BIT			| \
					SCTLR_SA_BIT			| \
					SCTLR_SA0_BIT			| \
					SCTLR_SED_BIT			| \
					SCTLR_I_BIT			| \
					SCTLR_DZE_BIT			| \
					SCTLR_UCT_BIT			| \
					SCTLR_NTWI_BIT			| \
					SCTLR_NTWE_BIT			| \
					SCTLR_WXN_BIT			| \
					SCTLR_TSCXT_BIT			| \
					SCTLR_UCI_BIT			| \
					SCTLR_nTLSMD_BIT		| \
					SCTLR_LSMAOE_BIT		| \
					(U(1) << 22) 			| \
					(U(1) << 11)			\
				)

#define REALM_SCXTNUM_EL2	0x0

/*
 * SPSR_EL2
 *   M=0x9 (0b1001 EL2h)
 *   M[4]=0
 *   DAIF=0xF Exceptions masked on entry.
 *   BTYPE=0  BTI not yet supported.
 *   SSBS=0   Not yet supported.
 *   IL=0     Not an illegal exception return.
 *   SS=0     Not single stepping.
 *   PAN=1    RMM shouldn't access realm memory.
 *   UAO=0
 *   DIT=0
 *   TCO=0
 *   NZCV=0
 */
#define REALM_SPSR_EL2		(					\
					SPSR_M_EL2H			| \
					(0xF << SPSR_DAIF_SHIFT) 	| \
				 	SPSR_PAN_BIT			\
				 )

/*
 * TCR_EL2
 *   T0SZ=16    VA range 48 bits.
 *   EPD0=1     TTBR0_EL2 disabled.
 *   IRGN=0b01  Normal, inner wb ra wa cacheable.
 *   ORGN=0b01  Normal, outer wb ra wa cacheable.
 *   SH=0b11    Inner shareable.
 *   TG0=0      4k pages.
 *   T1SZ=28    36 bit VA.
 *   A1=0       TTBR0_EL2.ASID defines the ASID.
 *   EPD1=0     TTBR1_EL2 not disabled.
 *   IRGN1=0b01 Normal, inner wb ra wa cacheable.
 *   ORGN1=0b01 Normal, outer wb ra wa cacheable.
 *   SH1=0b11   Inner shareable.
 *   TG1=0b10   4k pages.
 *   IPS=0b001  36 bits.
 *   AS=1       16 bit ASID.
 *   TBI0=0     Top byte used for address calc.
 *   TBI1=0     Same.
 *   HA=0       Hardware access flag update disabled.
 *   HD=0       Hardware management of dirty flag disabled.
 *   HPD0=1     Hierarchical permissions disabled.
 *   HPD1=1     Same.
 *   HWU0xx     Hardware cannot use bits 59-62 in page tables.
 *   HWU1xx     Hardware cannot use bits 59-62 in page tables.
 *   TBID0=0    Pointer auth not enabled.
 *   TBID1=0    Pointer auth not enabled.
 *   NFD0=0     SVE not enabled, do not disable stage 1 trans using TTBR0_EL2.
 *   NFD1=0     Same but TTBR1_EL2.
 *   E0PD0=0    EL0 access to anything translated by TTBR0_EL2 will not fault.
 *   E0PD0=0    Same but TTBR1_EL2.
 *   TCCMA0=0   Memory tagging not enabled.
 *   TCCMA1=0   Same.
 */
#define REALM_TCR_EL2		(					\
					(16UL << TCR_T0SZ_SHIFT)	| \
					TCR_EPD0_BIT			| \
					TCR_RGN_INNER_WBA		| \
					TCR_RGN_OUTER_WBA		| \
					TCR_SH_INNER_SHAREABLE		| \
					(28UL << TCR_T1SZ_SHIFT)	| \
					TCR_RGN1_INNER_WBA		| \
					TCR_RGN1_OUTER_WBA		| \
					TCR_SH1_INNER_SHAREABLE		| \
					TCR_TG1_4K			| \
					(1UL << TCR_EL2_IPS_SHIFT)	| \
					TCR_AS_BIT			| \
					TCR_HPD0_BIT			| \
					TCR_HPD1_BIT			\
				 )

#define REALM_TRFCR_EL2	0x0

#define REALM_TTBR0_EL2	0x0

#endif /* RMMD_INITIAL_CONTEXT_H */
