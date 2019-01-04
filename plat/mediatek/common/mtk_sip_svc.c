/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <tools_share/uuid.h>

#include <mtk_plat_common.h>
#include <mtk_sip_svc.h>
#include <plat_sip_calls.h>

/* Mediatek SiP Service UUID */
DEFINE_SVC_UUID2(mtk_sip_svc_uid,
	0xa42b58f7, 0x6242, 0x7d4d, 0x80, 0xe5,
	0x8f, 0x95, 0x05, 0x00, 0x0f, 0x3d);

#pragma weak mediatek_plat_sip_handler
uintptr_t mediatek_plat_sip_handler(uint32_t smc_fid,
				u_register_t x1,
				u_register_t x2,
				u_register_t x3,
				u_register_t x4,
				void *cookie,
				void *handle,
				u_register_t flags)
{
	ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
	SMC_RET1(handle, SMC_UNK);
}

/*
 * This function handles Mediatek defined SiP Calls */
uintptr_t mediatek_sip_handler(uint32_t smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3,
			u_register_t x4,
			void *cookie,
			void *handle,
			u_register_t flags)
{
	uint32_t ns;

	/* if parameter is sent from SMC32. Clean top 32 bits */
	clean_top_32b_of_param(smc_fid, &x1, &x2, &x3, &x4);

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns) {
		/* SiP SMC service secure world's call */
		;
	} else {
		/* SiP SMC service normal world's call */
		switch (smc_fid) {
#if MTK_SIP_SET_AUTHORIZED_SECURE_REG_ENABLE
		case MTK_SIP_SET_AUTHORIZED_SECURE_REG: {
			/* only use ret here */
			uint64_t ret;

			ret = mt_sip_set_authorized_sreg((uint32_t)x1,
				(uint32_t)x2);
			SMC_RET1(handle, ret);
		}
#endif
#if MTK_SIP_KERNEL_BOOT_ENABLE
		case MTK_SIP_KERNEL_BOOT_AARCH32:
			boot_to_kernel(x1, x2, x3, x4);
			SMC_RET0(handle);
#endif
		default:
			/* Do nothing in default case */
			break;
		}
	}

	return mediatek_plat_sip_handler(smc_fid, x1, x2, x3, x4,
					cookie, handle, flags);

}

/*
 * This function is responsible for handling all SiP calls from the NS world
 */
uintptr_t sip_smc_handler(uint32_t smc_fid,
			 u_register_t x1,
			 u_register_t x2,
			 u_register_t x3,
			 u_register_t x4,
			 void *cookie,
			 void *handle,
			 u_register_t flags)
{
	switch (smc_fid) {
	case SIP_SVC_CALL_COUNT:
		/* Return the number of Mediatek SiP Service Calls. */
		SMC_RET1(handle,
			 MTK_COMMON_SIP_NUM_CALLS + MTK_PLAT_SIP_NUM_CALLS);

	case SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, mtk_sip_svc_uid);

	case SIP_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, MTK_SIP_SVC_VERSION_MAJOR,
			MTK_SIP_SVC_VERSION_MINOR);

	default:
		return mediatek_sip_handler(smc_fid, x1, x2, x3, x4,
			cookie, handle, flags);
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
