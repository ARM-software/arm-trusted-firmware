/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_LPM_DISPATCH_H
#define MT_LPM_DISPATCH_H

#include <stdint.h>

#include "mt_lpm_smc.h"
#include <mtk_sip_svc.h>

#define MTK_DISPATCH_ID_MAX	32

typedef uint64_t (*mt_lpm_dispatch_fn)(u_register_t x1, u_register_t x2,
				       u_register_t x3, u_register_t x4,
				       void *handle,
				       struct smccc_res *smccc_ret);

struct mt_dispatch_ctrl {
	unsigned int enable;
	mt_lpm_dispatch_fn fn[MT_LPM_SMC_USER_MAX];
};

void mt_lpm_dispatcher_registry(unsigned int id, mt_lpm_dispatch_fn fn);

void mt_secure_lpm_dispatcher_registry(unsigned int id, mt_lpm_dispatch_fn fn);

extern struct mt_dispatch_ctrl mt_dispatcher;
extern struct mt_dispatch_ctrl mt_secure_dispatcher;
#endif
