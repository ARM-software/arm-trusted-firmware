/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <bl_common.h>
#include <debug.h>
#include <errno.h>
#include <memctrl.h>
#include <runtime_svc.h>
#include <tegra_private.h>

/*******************************************************************************
 * Common Tegra SiP SMCs
 ******************************************************************************/
#define TEGRA_SIP_NEW_VIDEOMEM_REGION		0x82000003
#define TEGRA_SIP_FIQ_NS_ENTRYPOINT		0x82000005
#define TEGRA_SIP_FIQ_NS_GET_CONTEXT		0x82000006

/*******************************************************************************
 * SoC specific SiP handler
 ******************************************************************************/
#pragma weak plat_sip_handler
int plat_sip_handler(uint32_t smc_fid,
		     uint64_t x1,
		     uint64_t x2,
		     uint64_t x3,
		     uint64_t x4,
		     void *cookie,
		     void *handle,
		     uint64_t flags)
{
	return -ENOTSUP;
}

/*******************************************************************************
 * This function is responsible for handling all SiP calls
 ******************************************************************************/
uint64_t tegra_sip_handler(uint32_t smc_fid,
			   uint64_t x1,
			   uint64_t x2,
			   uint64_t x3,
			   uint64_t x4,
			   void *cookie,
			   void *handle,
			   uint64_t flags)
{
	int err;

	/* Check if this is a SoC specific SiP */
	err = plat_sip_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);
	if (err == 0)
		SMC_RET1(handle, err);

	switch (smc_fid) {

	case TEGRA_SIP_NEW_VIDEOMEM_REGION:

		/* clean up the high bits */
		x1 = (uint32_t)x1;
		x2 = (uint32_t)x2;

		/*
		 * Check if Video Memory overlaps TZDRAM (contains bl31/bl32)
		 * or falls outside of the valid DRAM range
		 */
		err = bl31_check_ns_address(x1, x2);
		if (err)
			SMC_RET1(handle, err);

		/*
		 * Check if Video Memory is aligned to 1MB.
		 */
		if ((x1 & 0xFFFFF) || (x2 & 0xFFFFF)) {
			ERROR("Unaligned Video Memory base address!\n");
			SMC_RET1(handle, -ENOTSUP);
		}

		/* new video memory carveout settings */
		tegra_memctrl_videomem_setup(x1, x2);

		SMC_RET1(handle, 0);
		break;

	/*
	 * The NS world registers the address of its handler to be
	 * used for processing the FIQ. This is normally used by the
	 * NS FIQ debugger driver to detect system hangs by programming
	 * a watchdog timer to fire a FIQ interrupt.
	 */
	case TEGRA_SIP_FIQ_NS_ENTRYPOINT:

		if (!x1)
			SMC_RET1(handle, SMC_UNK);

		/*
		 * TODO: Check if x1 contains a valid DRAM address
		 */

		/* store the NS world's entrypoint */
		tegra_fiq_set_ns_entrypoint(x1);

		SMC_RET1(handle, 0);
		break;

	/*
	 * The NS world's FIQ handler issues this SMC to get the NS EL1/EL0
	 * CPU context when the FIQ interrupt was triggered. This allows the
	 * NS world to understand the CPU state when the watchdog interrupt
	 * triggered.
	 */
	case TEGRA_SIP_FIQ_NS_GET_CONTEXT:

		/* retrieve context registers when FIQ triggered */
		tegra_fiq_get_intr_context();

		SMC_RET0(handle);
		break;

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	tegra_sip_fast,

	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	tegra_sip_handler
);
