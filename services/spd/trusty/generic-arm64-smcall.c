/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <platform_def.h>

#include "generic-arm64-smcall.h"

int trusty_disable_serial_debug;

struct dputc_state {
	char linebuf[128];
	unsigned l;
};

static struct dputc_state dputc_state[2];

static void trusty_dputc(char ch, int secure)
{
	unsigned i;
	struct dputc_state *s = &dputc_state[!secure];

	if (trusty_disable_serial_debug)
		return;

	s->linebuf[s->l++] = ch;
	if (s->l == sizeof(s->linebuf) || ch == '\n') {
		if (secure)
			printf("secure os: ");
		else
			printf("non-secure os: ");
		for (i = 0; i < s->l; i++) {
			putchar(s->linebuf[i]);
		}
		if (ch != '\n') {
			printf(" <...>\n");
		}
		s->l = 0;
	}
}

static uint64_t trusty_get_reg_base(uint32_t reg)
{
	switch (reg) {
	case 0:
		return PLAT_ARM_GICD_BASE;

	case 1:
		return PLAT_ARM_GICC_BASE;

	default:
		NOTICE("%s(0x%x) unknown reg\n", __func__, reg);
		return SMC_UNK;
	}
}

static uintptr_t trusty_generic_platform_smc(uint32_t smc_fid,
			 u_register_t x1,
			 u_register_t x2,
			 u_register_t x3,
			 u_register_t x4,
			 void *cookie,
			 void *handle,
			 u_register_t flags)
{
	switch (smc_fid) {
	case SMC_FC_DEBUG_PUTC:
		trusty_dputc(x1, is_caller_secure(flags));
		SMC_RET1(handle, 0);

	case SMC_FC_GET_REG_BASE:
	case SMC_FC64_GET_REG_BASE:
		SMC_RET1(handle, trusty_get_reg_base(x1));

	default:
		NOTICE("%s(0x%x, 0x%lx) unknown smc\n", __func__, smc_fid, x1);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Define a SPD runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	trusty_fast,

	SMC_ENTITY_PLATFORM_MONITOR,
	SMC_ENTITY_PLATFORM_MONITOR,
	SMC_TYPE_FAST,
	NULL,
	trusty_generic_platform_smc
);

