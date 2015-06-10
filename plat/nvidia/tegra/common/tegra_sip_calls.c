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

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <debug.h>
#include <errno.h>
#include <memctrl.h>
#include <runtime_svc.h>
#include <tegra_private.h>

#define TEGRA_SIP_NEW_VIDEOMEM_REGION		0x82000003

/*******************************************************************************
 * This function is responsible for handling all SiP calls from the NS world
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
	uint32_t ns;
	int err;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns)
		SMC_RET1(handle, SMC_UNK);

	switch (smc_fid) {

	case TEGRA_SIP_NEW_VIDEOMEM_REGION:

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
