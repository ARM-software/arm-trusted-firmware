/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMCCC_HELPERS_H
#define SMCCC_HELPERS_H

#include <lib/smccc.h>

/* These are offsets to registers in smc_ctx_t */
#define SMC_CTX_GPREG_R0	U(0x0)
#define SMC_CTX_GPREG_R1	U(0x4)
#define SMC_CTX_GPREG_R2	U(0x8)
#define SMC_CTX_GPREG_R3	U(0xC)
#define SMC_CTX_GPREG_R4	U(0x10)
#define SMC_CTX_GPREG_R5	U(0x14)
#define SMC_CTX_SP_USR		U(0x34)
#define SMC_CTX_SPSR_MON	U(0x78)
#define SMC_CTX_SP_MON		U(0x7C)
#define SMC_CTX_LR_MON		U(0x80)
#define SMC_CTX_SCR		U(0x84)
#define SMC_CTX_PMCR		U(0x88)
#define SMC_CTX_SIZE		U(0x90)

#ifndef __ASSEMBLY__

#include <stdint.h>

#include <lib/cassert.h>

/*
 * The generic structure to save arguments and callee saved registers during
 * an SMC. Also this structure is used to store the result return values after
 * the completion of SMC service.
 */
typedef struct smc_ctx {
	u_register_t r0;
	u_register_t r1;
	u_register_t r2;
	u_register_t r3;
	u_register_t r4;
	u_register_t r5;
	u_register_t r6;
	u_register_t r7;
	u_register_t r8;
	u_register_t r9;
	u_register_t r10;
	u_register_t r11;
	u_register_t r12;
	/* spsr_usr doesn't exist */
	u_register_t sp_usr;
	u_register_t lr_usr;
	u_register_t spsr_irq;
	u_register_t sp_irq;
	u_register_t lr_irq;
	u_register_t spsr_fiq;
	u_register_t sp_fiq;
	u_register_t lr_fiq;
	u_register_t spsr_svc;
	u_register_t sp_svc;
	u_register_t lr_svc;
	u_register_t spsr_abt;
	u_register_t sp_abt;
	u_register_t lr_abt;
	u_register_t spsr_und;
	u_register_t sp_und;
	u_register_t lr_und;
	u_register_t spsr_mon;
	/*
	 * `sp_mon` will point to the C runtime stack in monitor mode. But prior
	 * to exit from SMC, this will point to the `smc_ctx_t` so that
	 * on next entry due to SMC, the `smc_ctx_t` can be easily accessed.
	 */
	u_register_t sp_mon;
	u_register_t lr_mon;
	u_register_t scr;
	u_register_t pmcr;
	/*
	 * The workaround for CVE-2017-5715 requires storing information in
	 * the bottom 3 bits of the stack pointer.  Add a padding field to
	 * force the size of the struct to be a multiple of 8.
	 */
	u_register_t pad;
} smc_ctx_t __aligned(8);

/*
 * Compile time assertions related to the 'smc_context' structure to
 * ensure that the assembler and the compiler view of the offsets of
 * the structure members is the same.
 */
CASSERT(SMC_CTX_GPREG_R0 == __builtin_offsetof(smc_ctx_t, r0), \
	assert_smc_ctx_greg_r0_offset_mismatch);
CASSERT(SMC_CTX_GPREG_R1 == __builtin_offsetof(smc_ctx_t, r1), \
	assert_smc_ctx_greg_r1_offset_mismatch);
CASSERT(SMC_CTX_GPREG_R2 == __builtin_offsetof(smc_ctx_t, r2), \
	assert_smc_ctx_greg_r2_offset_mismatch);
CASSERT(SMC_CTX_GPREG_R3 == __builtin_offsetof(smc_ctx_t, r3), \
	assert_smc_ctx_greg_r3_offset_mismatch);
CASSERT(SMC_CTX_GPREG_R4 == __builtin_offsetof(smc_ctx_t, r4), \
	assert_smc_ctx_greg_r4_offset_mismatch);
CASSERT(SMC_CTX_SP_USR == __builtin_offsetof(smc_ctx_t, sp_usr), \
	assert_smc_ctx_sp_usr_offset_mismatch);
CASSERT(SMC_CTX_LR_MON == __builtin_offsetof(smc_ctx_t, lr_mon), \
	assert_smc_ctx_lr_mon_offset_mismatch);
CASSERT(SMC_CTX_SPSR_MON == __builtin_offsetof(smc_ctx_t, spsr_mon), \
	assert_smc_ctx_spsr_mon_offset_mismatch);

CASSERT((sizeof(smc_ctx_t) & 0x7U) == 0U, assert_smc_ctx_not_aligned);
CASSERT(SMC_CTX_SIZE == sizeof(smc_ctx_t), assert_smc_ctx_size_mismatch);

/* Convenience macros to return from SMC handler */
#define SMC_RET0(_h) {				\
	return (uintptr_t)(_h);			\
}
#define SMC_RET1(_h, _r0) {			\
	((smc_ctx_t *)(_h))->r0 = (_r0);	\
	SMC_RET0(_h);				\
}
#define SMC_RET2(_h, _r0, _r1) {		\
	((smc_ctx_t *)(_h))->r1 = (_r1);	\
	SMC_RET1(_h, (_r0));			\
}
#define SMC_RET3(_h, _r0, _r1, _r2) {		\
	((smc_ctx_t *)(_h))->r2 = (_r2);	\
	SMC_RET2(_h, (_r0), (_r1));		\
}
#define SMC_RET4(_h, _r0, _r1, _r2, _r3) {	\
	((smc_ctx_t *)(_h))->r3 = (_r3);	\
	SMC_RET3(_h, (_r0), (_r1), (_r2));	\
}

/*
 * Helper macro to retrieve the SMC parameters from smc_ctx_t.
 */
#define get_smc_params_from_ctx(_hdl, _r1, _r2, _r3, _r4) {	\
		_r1 = ((smc_ctx_t *)_hdl)->r1;		\
		_r2 = ((smc_ctx_t *)_hdl)->r2;		\
		_r3 = ((smc_ctx_t *)_hdl)->r3;		\
		_r4 = ((smc_ctx_t *)_hdl)->r4;		\
		}

/* ------------------------------------------------------------------------
 * Helper APIs for setting and retrieving appropriate `smc_ctx_t`.
 * These functions need to implemented by the BL including this library.
 * ------------------------------------------------------------------------
 */

/* Get the pointer to `smc_ctx_t` corresponding to the security state. */
void *smc_get_ctx(unsigned int security_state);

/* Set the next `smc_ctx_t` corresponding to the security state. */
void smc_set_next_ctx(unsigned int security_state);

/* Get the pointer to next `smc_ctx_t` already set by `smc_set_next_ctx()`. */
void *smc_get_next_ctx(void);

#endif /*__ASSEMBLY__*/

#endif /* SMCCC_HELPERS_H */
