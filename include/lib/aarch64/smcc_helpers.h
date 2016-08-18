/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef __ASSEMBLY__
#include <context.h>

/* Convenience macros to return from SMC handler */
#define SMC_RET0(_h)	{					\
	return (uint64_t) (_h);					\
}
#define SMC_RET1(_h, _x0)	{				\
	write_ctx_reg(get_gpregs_ctx(_h), CTX_GPREG_X0, (_x0));	\
	SMC_RET0(_h);						\
}
#define SMC_RET2(_h, _x0, _x1)	{				\
	write_ctx_reg(get_gpregs_ctx(_h), CTX_GPREG_X1, (_x1));	\
	SMC_RET1(_h, (_x0));					\
}
#define SMC_RET3(_h, _x0, _x1, _x2)	{			\
	write_ctx_reg(get_gpregs_ctx(_h), CTX_GPREG_X2, (_x2));	\
	SMC_RET2(_h, (_x0), (_x1));				\
}
#define SMC_RET4(_h, _x0, _x1, _x2, _x3)	{		\
	write_ctx_reg(get_gpregs_ctx(_h), CTX_GPREG_X3, (_x3));	\
	SMC_RET3(_h, (_x0), (_x1), (_x2));			\
}

/*
 * Convenience macros to access general purpose registers using handle provided
 * to SMC handler. These take the offset values defined in context.h
 */
#define SMC_GET_GP(_h, _g)					\
	read_ctx_reg(get_gpregs_ctx(_h), (_g))
#define SMC_SET_GP(_h, _g, _v)					\
	write_ctx_reg(get_gpregs_ctx(_h), (_g), (_v))

/*
 * Convenience macros to access EL3 context registers using handle provided to
 * SMC handler. These take the offset values defined in context.h
 */
#define SMC_GET_EL3(_h, _e)					\
	read_ctx_reg(get_el3state_ctx(_h), (_e))
#define SMC_SET_EL3(_h, _e, _v)					\
	write_ctx_reg(get_el3state_ctx(_h), (_e), (_v))

/* Return a UUID in the SMC return registers */
#define SMC_UUID_RET(_h, _uuid)					\
	SMC_RET4(handle, ((const uint32_t *) &(_uuid))[0],	\
			 ((const uint32_t *) &(_uuid))[1],	\
			 ((const uint32_t *) &(_uuid))[2],	\
			 ((const uint32_t *) &(_uuid))[3])

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
	} while (0)

#endif /*__ASSEMBLY__*/
#endif /* __SMCC_HELPERS_H__ */
