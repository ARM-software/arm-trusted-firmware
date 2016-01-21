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
#include <runtime_svc.h>
#include <debug.h>
#include <platform.h>
#include <mmio.h>
#include <console.h>
#include <plat_private.h>
#include <mtk_sip_svc.h>
#include "mt_cpuxgpt.h"
#include <xlat_tables.h>
#include <platform_def.h>
#ifdef SECURE_DEINT_SUPPORT
#include <eint.h>
#endif

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
	atf_arg_t_ptr teearg = &gteearg;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);

	VERBOSE("sip_smc_handler\n");
	VERBOSE("id=0x%x\n", smc_fid);
	VERBOSE("x1=0x%lx, x2=0x%lx, x3=0x%lx, x4=0x%lx\n", x1, x2, x3, x4);

	switch (smc_fid) {
	case MTK_SIP_TBASE_HWUID_AARCH32:
		if (ns)
				SMC_RET1(handle, SMC_UNK);
		SMC_RET4(handle, teearg->hwuid[0], teearg->hwuid[1],
				teearg->hwuid[2], teearg->hwuid[3]);
		break;
	case MTK_SIP_KERNEL_BOOT_AARCH32:
		wdt_kernel_cb_addr = 0;
		console_init(gteearg.atf_log_port,
			MT6795_UART_CLOCK, MT6795_BAUDRATE);
		INFO("save kernel info\n");
		save_kernel_info(x1, x2, x3, x4);
		bl31_prepare_kernel_entry(x4);
		INFO("el3_exit\n");
		console_uninit();
		SMC_RET0(handle);
		break;
	case MTK_SIP_LK_WDT_AARCH32:
	case MTK_SIP_LK_WDT_AARCH64:
		set_kernel_k32_64(LINUX_KERNEL_32);
		wdt_kernel_cb_addr = x1;
		INFO("MTK_SIP_LK_WDT : 0x%lx\n",  wdt_kernel_cb_addr);
		rc = teearg->atf_aee_debug_buf_start;
		break;
#ifdef MTK_ATF_RAM_DUMP
	case MTK_SIP_RAM_DUMP_ADDR_AARCH32:
		atf_ram_dump_base = x1<<32 |  (x2&0xffffffff);
		atf_ram_dump_size = x3<<32 | (x4&0xffffffff);
		break;
	case MTK_SIP_RAM_DUMP_ADDR_AARCH64:
		atf_ram_dump_base = x1;
		atf_ram_dump_size = x2;
		break;
#endif
	case MTK_SIP_KERNEL_WDT_AARCH32:
	case MTK_SIP_KERNEL_WDT_AARCH64:
		wdt_kernel_cb_addr = x1;
		INFO("MTK_SIP_KERNEL_WDT : 0x%lx\n",  wdt_kernel_cb_addr);
		INFO("teearg->atf_aee_debug_buf_start : 0x%x\n",
					 teearg->atf_aee_debug_buf_start);
		rc = teearg->atf_aee_debug_buf_start;
		break;
	default:
		rc = SMC_UNK;
		console_init(gteearg.atf_log_port,
			MT6795_UART_CLOCK, MT6795_BAUDRATE);
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		console_uninit();
		assert(0);
	}
	SMC_RET1(handle, rc);
}
