/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMCCC_HELPERS_H
#define SMCCC_HELPERS_H

#include <lib/smccc.h>

/* Definitions to help the assembler access the SMC/ERET args structure */
#define SMC_ARGS_SIZE		0x40
#define SMC_ARG0		0x0
#define SMC_ARG1		0x8
#define SMC_ARG2		0x10
#define SMC_ARG3		0x18
#define SMC_ARG4		0x20
#define SMC_ARG5		0x28
#define SMC_ARG6		0x30
#define SMC_ARG7		0x38
#define SMC_ARGS_END		0x40

#ifndef __ASSEMBLER__

#include <stdbool.h>

#include <context.h>

#include <platform_def.h> /* For CACHE_WRITEBACK_GRANULE */

/* Convenience macros to return from SMC handler */
#define SMC_RET0(_h)	{					\
	return (uint64_t) (_h);					\
}
#define SMC_RET1(_h, _x0)	{				\
	write_ctx_reg((get_gpregs_ctx(_h)), (CTX_GPREG_X0), (_x0));	\
	SMC_RET0(_h);						\
}
#define SMC_RET2(_h, _x0, _x1)	{				\
	write_ctx_reg((get_gpregs_ctx(_h)), (CTX_GPREG_X1), (_x1));	\
	SMC_RET1(_h, (_x0));					\
}
#define SMC_RET3(_h, _x0, _x1, _x2)	{			\
	write_ctx_reg((get_gpregs_ctx(_h)), (CTX_GPREG_X2), (_x2));	\
	SMC_RET2(_h, (_x0), (_x1));				\
}
#define SMC_RET4(_h, _x0, _x1, _x2, _x3)	{		\
	write_ctx_reg((get_gpregs_ctx(_h)), (CTX_GPREG_X3), (_x3));	\
	SMC_RET3(_h, (_x0), (_x1), (_x2));			\
}
#define SMC_RET5(_h, _x0, _x1, _x2, _x3, _x4)	{		\
	write_ctx_reg((get_gpregs_ctx(_h)), (CTX_GPREG_X4), (_x4));	\
	SMC_RET4(_h, (_x0), (_x1), (_x2), (_x3));		\
}
#define SMC_RET6(_h, _x0, _x1, _x2, _x3, _x4, _x5)	{	\
	write_ctx_reg((get_gpregs_ctx(_h)), (CTX_GPREG_X5), (_x5));	\
	SMC_RET5(_h, (_x0), (_x1), (_x2), (_x3), (_x4));	\
}
#define SMC_RET7(_h, _x0, _x1, _x2, _x3, _x4, _x5, _x6)	{	\
	write_ctx_reg((get_gpregs_ctx(_h)), (CTX_GPREG_X6), (_x6));	\
	SMC_RET6(_h, (_x0), (_x1), (_x2), (_x3), (_x4), (_x5));	\
}
#define SMC_RET8(_h, _x0, _x1, _x2, _x3, _x4, _x5, _x6, _x7) {	\
	write_ctx_reg((get_gpregs_ctx(_h)), (CTX_GPREG_X7), (_x7));	\
	SMC_RET7(_h, (_x0), (_x1), (_x2), (_x3), (_x4), (_x5), (_x6));	\
}

/*
 * Convenience macros to access general purpose registers using handle provided
 * to SMC handler. These take the offset values defined in context.h
 */
#define SMC_GET_GP(_h, _g)					\
	read_ctx_reg((get_gpregs_ctx(_h)), (_g))
#define SMC_SET_GP(_h, _g, _v)					\
	write_ctx_reg((get_gpregs_ctx(_h)), (_g), (_v))

/*
 * Convenience macros to access EL3 context registers using handle provided to
 * SMC handler. These take the offset values defined in context.h
 */
#define SMC_GET_EL3(_h, _e)					\
	read_ctx_reg((get_el3state_ctx(_h)), (_e))
#define SMC_SET_EL3(_h, _e, _v)					\
	write_ctx_reg((get_el3state_ctx(_h)), (_e), (_v))

/*
 * Helper macro to retrieve the SMC parameters from cpu_context_t.
 */
#define get_smc_params_from_ctx(_hdl, _x1, _x2, _x3, _x4)	\
	do {							\
		const gp_regs_t *regs = get_gpregs_ctx(_hdl);	\
		_x1 = read_ctx_reg(regs, CTX_GPREG_X1);		\
		_x2 = read_ctx_reg(regs, CTX_GPREG_X2);		\
		_x3 = read_ctx_reg(regs, CTX_GPREG_X3);		\
		_x4 = read_ctx_reg(regs, CTX_GPREG_X4);		\
	} while (false)

typedef struct {
	uint64_t _regs[SMC_ARGS_END >> 3];
} __aligned(CACHE_WRITEBACK_GRANULE) smc_args_t;

/*
 * Ensure that the assembler's view of the size of the tsp_args is the
 * same as the compilers.
 */
CASSERT(sizeof(smc_args_t) == SMC_ARGS_SIZE, assert_sp_args_size_mismatch);

static inline smc_args_t smc_helper(uint32_t func, uint64_t arg0,
	       uint64_t arg1, uint64_t arg2,
	       uint64_t arg3, uint64_t arg4,
	       uint64_t arg5, uint64_t arg6)
{
	smc_args_t ret_args = {0};

	register uint64_t r0 __asm__("x0") = func;
	register uint64_t r1 __asm__("x1") = arg0;
	register uint64_t r2 __asm__("x2") = arg1;
	register uint64_t r3 __asm__("x3") = arg2;
	register uint64_t r4 __asm__("x4") = arg3;
	register uint64_t r5 __asm__("x5") = arg4;
	register uint64_t r6 __asm__("x6") = arg5;
	register uint64_t r7 __asm__("x7") = arg6;

	/* Output registers, also used as inputs ('+' constraint). */
	__asm__ volatile("smc #0"
			: "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4),
			  "+r"(r5), "+r"(r6), "+r"(r7));

	ret_args._regs[0] = r0;
	ret_args._regs[1] = r1;
	ret_args._regs[2] = r2;
	ret_args._regs[3] = r3;
	ret_args._regs[4] = r4;
	ret_args._regs[5] = r5;
	ret_args._regs[6] = r6;
	ret_args._regs[7] = r7;

	return ret_args;
}

#endif /*__ASSEMBLER__*/

#endif /* SMCCC_HELPERS_H */
