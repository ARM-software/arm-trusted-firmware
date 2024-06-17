/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2022, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SIMD_CTX_H
#define SIMD_CTX_H

/*******************************************************************************
 * Constants that allow assembler code to access members of and the 'simd_context'
 * structure at their correct offsets.
 ******************************************************************************/

#if CTX_INCLUDE_FPREGS

#define SIMD_VECTOR_LEN_BYTES	U(16) /* 128 bits fixed vector length for FPU */

#define CTX_SIMD_VECTORS	U(0)
/* there are 32 vector registers, each of size SIMD_VECTOR_LEN_BYTES */
#define CTX_SIMD_FPSR		(CTX_SIMD_VECTORS + (32 * SIMD_VECTOR_LEN_BYTES))
#define CTX_SIMD_FPCR		(CTX_SIMD_FPSR + 8)

#if  CTX_INCLUDE_AARCH32_REGS
#define CTX_SIMD_FPEXC32	(CTX_SIMD_FPCR + 8)
#endif /* CTX_INCLUDE_AARCH32_REGS */

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
	/* 16 bytes to align to next 16 byte boundary */
	uint8_t fpexc32_el2[16];
#endif
} simd_regs_t __attribute__((aligned(16)));

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

void simd_ctx_save(uint32_t security_state, bool hint_sve);
void simd_ctx_restore(uint32_t security_state);

#endif /* __ASSEMBLER__ */

#endif /* CTX_INCLUDE_FPREGS */

#endif /* SIMD_CTX_H */
