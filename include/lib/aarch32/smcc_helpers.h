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

#ifndef __SMCC_HELPERS_H__
#define __SMCC_HELPERS_H__

#include <smcc.h>

/* These are offsets to registers in smc_ctx_t */
#define SMC_CTX_GPREG_R0	0x0
#define SMC_CTX_GPREG_R1	0x4
#define SMC_CTX_GPREG_R2	0x8
#define SMC_CTX_GPREG_R3	0xC
#define SMC_CTX_GPREG_R4	0x10
#define SMC_CTX_SP_USR		0x34
#define SMC_CTX_SPSR_MON	0x78
#define SMC_CTX_LR_MON		0x7C
#define SMC_CTX_SIZE		0x80

#ifndef __ASSEMBLY__
#include <cassert.h>
#include <types.h>

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
	/* No need to save 'sp_mon' because we are already in monitor mode */
	u_register_t lr_mon;
} smc_ctx_t;

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

/* Return a UUID in the SMC return registers */
#define SMC_UUID_RET(_h, _uuid) \
	SMC_RET4(handle, ((const uint32_t *) &(_uuid))[0], \
			 ((const uint32_t *) &(_uuid))[1], \
			 ((const uint32_t *) &(_uuid))[2], \
			 ((const uint32_t *) &(_uuid))[3])

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
void *smc_get_ctx(int security_state);

/* Set the next `smc_ctx_t` corresponding to the security state. */
void smc_set_next_ctx(int security_state);

/* Get the pointer to next `smc_ctx_t` already set by `smc_set_next_ctx()`. */
void *smc_get_next_ctx(void);

#endif /*__ASSEMBLY__*/
#endif /* __SMCC_HELPERS_H__ */
