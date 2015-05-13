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

#include <debug.h>
#include <runtime_svc.h>

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

static uint64_t trusty_generic_platform_smc(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags)
{
	switch(smc_fid) {
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

