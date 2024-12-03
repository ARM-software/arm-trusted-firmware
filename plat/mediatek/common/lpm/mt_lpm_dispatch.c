/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>

#include <lpm/mt_lpm_dispatch.h>

struct mt_dispatch_ctrl mt_dispatcher __section("mt_lpm_s") = {
	.enable = 0,
};

struct mt_dispatch_ctrl mt_secure_dispatcher __section("mt_secure_lpm_s") = {
	.enable = 0,
};

u_register_t invoke_mt_lpm_dispatch(u_register_t x1,
				    u_register_t x2,
				    u_register_t x3,
				    u_register_t x4,
				    void *handle,
				    struct smccc_res *smccc_ret)
{
	uint64_t res = 0;
	uint32_t user;

	if (!IS_MT_LPM_SMC(x1))
		return 0;

	user = MT_LPM_SMC_USER(x1);
	if ((user < MT_LPM_SMC_USER_MAX) &&
	    (mt_dispatcher.enable & (BIT(user)))) {
		res = mt_dispatcher.fn[user](MT_LPM_SMC_USER_ID(x1),
					     x2,
					     x3,
					     x4,
					     handle,
					     smccc_ret);
	}

	return res;
}
DECLARE_SMC_HANDLER(MTK_SIP_MTK_LPM_CONTROL, invoke_mt_lpm_dispatch);

u_register_t invoke_mt_secure_lpm_dispatch(u_register_t x1,
					   u_register_t x2,
					   u_register_t x3,
					   u_register_t x4,
					   void *handle,
					   struct smccc_res *smccc_ret)
{
	uint64_t res = 0;
	uint32_t user;

	if (!IS_MT_LPM_SMC(x1))
		return 0;

	user = MT_LPM_SMC_USER(x1);
	if (mt_secure_dispatcher.enable & (BIT(user))) {
		res = mt_secure_dispatcher.fn[user](MT_LPM_SMC_USER_ID(x1),
						    x2,
						    x3,
						    x4,
						    handle,
						    smccc_ret);
	}

	return res;
}
DECLARE_SMC_HANDLER(MTK_SIP_BL_LPM_CONTROL, invoke_mt_secure_lpm_dispatch);

/* Check lpm smc user number at compile time */
CASSERT(MT_LPM_SMC_USER_MAX <= MTK_DISPATCH_ID_MAX,
	lpm_smc_user_declare_too_large);

void mt_lpm_dispatcher_registry(unsigned int id, mt_lpm_dispatch_fn fn)
{
	if (id >= MT_LPM_SMC_USER_MAX)
		return;

	mt_dispatcher.enable |= BIT(id);
	mt_dispatcher.fn[id] = fn;
}

void mt_secure_lpm_dispatcher_registry(unsigned int id, mt_lpm_dispatch_fn fn)
{
	if (id >= MT_LPM_SMC_USER_MAX)
		return;

	mt_secure_dispatcher.enable |= BIT(id);
	mt_secure_dispatcher.fn[id] = fn;
}
