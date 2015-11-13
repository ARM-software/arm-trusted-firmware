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
#include <assert.h>
#include <debug.h>
#include <mtk_sip_svc.h>
#include <runtime_svc.h>
#include <uuid.h>

/* Mediatek SiP Service UUID */
DEFINE_SVC_UUID(mtk_sip_svc_uid,
		0xf7582ba4, 0x4262, 0x4d7d, 0x80, 0xe5,
		0x8f, 0x95, 0x05, 0x00, 0x0f, 0x3d);

/*
 * This function handles Mediatek defined SiP Calls */
static uint64_t mediatek_sip_handler(uint32_t smc_fid,
				     uint64_t x1,
				     uint64_t x2,
				     uint64_t x3,
				     uint64_t x4,
				     void *cookie,
				     void *handle)
{
	uint64_t ret;

	switch (smc_fid) {
	case MTK_SIP_SET_AUTHORIZED_SECURE_REG:
		ret = mt_sip_set_authorized_sreg((uint32_t)x1, (uint32_t)x2);
		SMC_RET1(handle, ret);

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}

/*
 * This function is responsible for handling all SiP calls from the NS world
 */
uint64_t sip_smc_handler(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags)
{
	uint32_t ns;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns)
		SMC_RET1(handle, SMC_UNK);

	switch (smc_fid) {
	case SIP_SVC_CALL_COUNT:
		/* Return the number of Mediatek SiP Service Calls. */
		SMC_RET1(handle, MTK_SIP_NUM_CALLS);

	case SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, mtk_sip_svc_uid);

	case SIP_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, MTK_SIP_SVC_VERSION_MAJOR,
			MTK_SIP_SVC_VERSION_MINOR);

	default:
		return mediatek_sip_handler(smc_fid, x1, x2, x3, x4,
			cookie, handle);
	}
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	mediatek_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	sip_smc_handler
);
