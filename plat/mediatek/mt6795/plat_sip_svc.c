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
#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <console.h>
#include <debug.h>
#include <mmio.h>
#include <mt_cpuxgpt.h>
#include <mtk_sip_svc.h>
#include <plat_private.h>
#include <platform.h>
#include <platform_def.h>
#include <mtk_plat_common.h>
#include <runtime_svc.h>
#include <xlat_tables.h>

/*******************************************************************************
 * SIP top level handler for servicing SMCs.
 ******************************************************************************/
uint64_t mediatek_plat_sip_handler(uint32_t smc_fid,
				uint64_t x1,
				uint64_t x2,
				uint64_t x3,
				uint64_t x4,
				void *cookie,
				void *handle,
				uint64_t flags)
{
	uint64_t rc = 0;
	uint32_t ns;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);

	VERBOSE("sip_smc_handler\n");
	VERBOSE("id=0x%x\n", smc_fid);
	VERBOSE("x1=0x%lx, x2=0x%lx, x3=0x%lx, x4=0x%lx\n", x1, x2, x3, x4);

	if (!ns) {
		/* SiP SMC service secure world's call */
		switch (smc_fid) {
		default:
			rc = SMC_UNK;
		}
	} else {
		/* SiP SMC service normal world's call */
		switch (smc_fid) {
		default:
			rc = SMC_UNK;
		}
	}

	if (SMC_UNK == rc)
	{
			console_init(gteearg.atf_log_port,
				UART_CLOCK, UART_BAUDRATE);
			ERROR("%s: unhandled NS(%d) SMC (0x%x)\n", __func__, ns, smc_fid);
			console_uninit();
	}
	SMC_RET1(handle, rc);
}
