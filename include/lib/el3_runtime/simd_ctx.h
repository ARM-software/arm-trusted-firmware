/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2022, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SIMD_CTX_H
#define SIMD_CTX_H

#include <stdbool.h>
#include <lib/cassert.h>
#include <lib/utils_def.h>

/*******************************************************************************
 * Constants that allow assembler code to access members of and the 'simd_context'
 * structure at their correct offsets.
 ******************************************************************************/

#if CTX_INCLUDE_FPREGS || CTX_INCLUDE_SVE_REGS
#if CTX_INCLUDE_SVE_REGS
#define SIMD_VECTOR_LEN_BYTES	(SVE_VECTOR_LEN / 8) /* Length of vector in bytes */
#elif CTX_INCLUDE_FPREGS
#define SIMD_VECTOR_LEN_BYTES	U(16) /* 128 bits fixed vector length for FPU */
#endif /* CTX_INCLUDE_SVE_REGS */

#define CTX_SIMD_VECTORS	U(0)
/* there are 32 vector registers, each of size SIMD_VECTOR_LEN_BYTES */
#define CTX_SIMD_FPSR		(CTX_SIMD_VECTORS + (32 * SIMD_VECTOR_LEN_BYTES))
#define CTX_SIMD_FPCR		(CTX_SIMD_FPSR + 8)

#if CTX_INCLUDE_FPREGS && CTX_INCLUDE_AARCH32_REGS
#define CTX_SIMD_FPEXC32	(CTX_SIMD_FPCR + 8)
#define CTX_SIMD_PREDICATES	(CTX_SIMD_FPEXC32 + 16)
#else
#define CTX_SIMD_PREDICATES      (CTX_SIMD_FPCR + 8)
#endif /* CTX_INCLUDE_FPREGS && CTX_INCLUDE_AARCH32_REGS */

/*
 * Each predicate register is 1/8th the size of a vector register and there are 16
 * predicate registers
 */
#define CTX_SIMD_FFR		(CTX_SIMD_PREDICATES + (16 * (SIMD_VECTOR_LEN_BYTES / 8)))

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <lib/cassert.h>

/*
 * Please don't change order of fields in this struct as that may violate
 * alignment requirements and affect how assembly code accesses members of this
 * struct.
 */
typedef struct {
	uint8_t vectors[32][SIMD_VECTOR_LEN_BYTES];
	uint8_t fpsr[8];
	uint8_t fpcr[8];
#if CTX_INCLUDE_FPREGS && CTX_INCLUDE_AARCH32_REGS
	/* 16 bytes to align to next 16 byte boundary when CTX_INCLUDE_SVE_REGS is 0 */
	uint8_t fpexc32_el2[16];
#endif
#if CTX_INCLUDE_SVE_REGS
	/* FFR and each of predicates is one-eigth of the SVE vector length */
	uint8_t predicates[16][SIMD_VECTOR_LEN_BYTES / 8];
	uint8_t ffr[SIMD_VECTOR_LEN_BYTES / 8];
	/* SMCCCv1.3 FID[16] hint bit state recorded on EL3 entry */
	bool hint;
#endif /* CTX_INCLUDE_SVE_REGS */
} __aligned(16) simd_regs_t;

CASSERT(CTX_SIMD_VECTORS == __builtin_offsetof(simd_regs_t, vectors),
		assert_vectors_mismatch);

CASSERT(CTX_SIMD_FPSR == __builtin_offsetof(simd_regs_t, fpsr),
		assert_fpsr_mismatch);

CASSERT(CTX_SIMD_FPCR == __builtin_offsetof(simd_regs_t, fpcr),
		assert_fpcr_mismatch);

#if CTX_INCLUDE_FPREGS && CTX_INCLUDE_AARCH32_REGS
CASSERT(CTX_SIMD_FPEXC32 == __builtin_offsetof(simd_regs_t, fpexc32_el2),
		assert_fpex32_mismtatch);
#endif

#if CTX_INCLUDE_SVE_REGS
CASSERT(CTX_SIMD_PREDICATES == __builtin_offsetof(simd_regs_t, predicates),
		assert_predicates_mismatch);

CASSERT(CTX_SIMD_FFR == __builtin_offsetof(simd_regs_t, ffr),
		assert_ffr_mismatch);
#endif

void simd_ctx_save(uint32_t security_state, bool hint_sve);
void simd_ctx_restore(uint32_t security_state);

#endif /* __ASSEMBLER__ */

#endif /* CTX_INCLUDE_FPREGS || CTX_INCLUDE_SVE_REGS */

#endif /* SIMD_CTX_H */
