/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IMX_SIP_SVC_H__
#define __IMX_SIP_SVC_H__

/* SMC function IDs for SiP Service queries */
#define IMX_SIP_CPUFREQ			0xC2000001
#define IMX_SIP_SET_CPUFREQ		0x00

#define IMX_SIP_SRTC			0xC2000002
#define IMX_SIP_SRTC_SET_TIME		0x00

#define IMX_SIP_BUILDINFO			0xC2000003
#define IMX_SIP_BUILDINFO_GET_COMMITHASH	0x00

#define IMX_SIP_GET_SOC_INFO		0xC2000006

#define IMX_SIP_WAKEUP_SRC		0xC2000009
#define IMX_SIP_WAKEUP_SRC_SCU		0x1
#define IMX_SIP_WAKEUP_SRC_IRQSTEER	0x2

#define IMX_SIP_OTP			0xC200000A
#define IMX_SIP_OTP_READ		0x1
#define IMX_SIP_OTP_WRITE		0x2

#define IMX_SIP_MISC_SET_TEMP		0xC200000C

#if defined(PLAT_imx8mq)
int imx_soc_info_handler(uint32_t smc_fid, u_register_t x1,
			 u_register_t x2, u_register_t x3);
#endif

#if (defined(PLAT_imx8qm) || defined(PLAT_imx8qx))
int imx_cpufreq_handler(uint32_t smc_fid, u_register_t x1,
			u_register_t x2, u_register_t x3);
int imx_srtc_handler(uint32_t smc_fid, void *handle, u_register_t x1,
		     u_register_t x2, u_register_t x3, u_register_t x4);
int imx_wakeup_src_handler(uint32_t smc_fid, u_register_t x1,
			   u_register_t x2, u_register_t x3);
int imx_otp_handler(uint32_t smc_fid, void *handle,
		    u_register_t x1, u_register_t x2, u_register_t x3);
int imx_misc_set_temp_handler(uint32_t smc_fid, u_register_t x1,
			      u_register_t x2, u_register_t x3,
			      u_register_t x4);
#endif
uint64_t imx_buildinfo_handler(uint32_t smc_fid, u_register_t x1,
			       u_register_t x2, u_register_t x3,
			       u_register_t x4);

#endif /* __IMX_SIP_SVC_H__ */
