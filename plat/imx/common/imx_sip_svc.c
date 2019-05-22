/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/pmf/pmf.h>
#include <tools_share/uuid.h>
#include <imx_sip_svc.h>

static int32_t imx_sip_setup(void)
{
	return 0;
}

static uintptr_t imx_sip_handler(unsigned int smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3,
			u_register_t x4,
			void *cookie,
			void *handle,
			u_register_t flags)
{
	switch (smc_fid) {
#if defined(PLAT_imx8mq)
	case IMX_SIP_GET_SOC_INFO:
		SMC_RET1(handle, imx_soc_info_handler(smc_fid, x1, x2, x3));
		break;
#endif
#if (defined(PLAT_imx8qm) || defined(PLAT_imx8qx))
	case  IMX_SIP_SRTC:
		return imx_srtc_handler(smc_fid, handle, x1, x2, x3, x4);
	case  IMX_SIP_CPUFREQ:
		SMC_RET1(handle, imx_cpufreq_handler(smc_fid, x1, x2, x3));
		break;
	case  IMX_SIP_WAKEUP_SRC:
		SMC_RET1(handle, imx_wakeup_src_handler(smc_fid, x1, x2, x3));
	case IMX_SIP_OTP_READ:
	case IMX_SIP_OTP_WRITE:
		return imx_otp_handler(smc_fid, handle, x1, x2);
	case IMX_SIP_MISC_SET_TEMP:
		SMC_RET1(handle, imx_misc_set_temp_handler(smc_fid, x1, x2, x3, x4));
#endif
	case  IMX_SIP_BUILDINFO:
		SMC_RET1(handle, imx_buildinfo_handler(smc_fid, x1, x2, x3, x4));
	default:
		WARN("Unimplemented i.MX SiP Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
		break;
	}
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
		imx_sip_svc,
		OEN_SIP_START,
		OEN_SIP_END,
		SMC_TYPE_FAST,
		imx_sip_setup,
		imx_sip_handler
);
