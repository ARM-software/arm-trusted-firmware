/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <common/debug.h>
#include <errno.h>
#include <mce.h>
#include <memctrl.h>
#include <common/runtime_svc.h>
#include <tegra_private.h>

extern uint32_t tegra186_system_powerdn_state;

/*******************************************************************************
 * Tegra186 SiP SMCs
 ******************************************************************************/
#define TEGRA_SIP_NEW_VIDEOMEM_REGION			0x82000003
#define TEGRA_SIP_SYSTEM_SHUTDOWN_STATE			0x82FFFE01
#define TEGRA_SIP_MCE_CMD_ENTER_CSTATE			0x82FFFF00
#define TEGRA_SIP_MCE_CMD_UPDATE_CSTATE_INFO		0x82FFFF01
#define TEGRA_SIP_MCE_CMD_UPDATE_CROSSOVER_TIME		0x82FFFF02
#define TEGRA_SIP_MCE_CMD_READ_CSTATE_STATS		0x82FFFF03
#define TEGRA_SIP_MCE_CMD_WRITE_CSTATE_STATS		0x82FFFF04
#define TEGRA_SIP_MCE_CMD_IS_SC7_ALLOWED		0x82FFFF05
#define TEGRA_SIP_MCE_CMD_ONLINE_CORE			0x82FFFF06
#define TEGRA_SIP_MCE_CMD_CC3_CTRL			0x82FFFF07
#define TEGRA_SIP_MCE_CMD_ECHO_DATA			0x82FFFF08
#define TEGRA_SIP_MCE_CMD_READ_VERSIONS			0x82FFFF09
#define TEGRA_SIP_MCE_CMD_ENUM_FEATURES			0x82FFFF0A
#define TEGRA_SIP_MCE_CMD_ROC_FLUSH_CACHE_TRBITS	0x82FFFF0B
#define TEGRA_SIP_MCE_CMD_ENUM_READ_MCA			0x82FFFF0C
#define TEGRA_SIP_MCE_CMD_ENUM_WRITE_MCA		0x82FFFF0D
#define TEGRA_SIP_MCE_CMD_ROC_FLUSH_CACHE		0x82FFFF0E
#define TEGRA_SIP_MCE_CMD_ROC_CLEAN_CACHE		0x82FFFF0F
#define TEGRA_SIP_MCE_CMD_ENABLE_LATIC			0x82FFFF10
#define TEGRA_SIP_MCE_CMD_UNCORE_PERFMON_REQ		0x82FFFF11
#define TEGRA_SIP_MCE_CMD_MISC_CCPLEX			0x82FFFF12

/*******************************************************************************
 * This function is responsible for handling all T186 SiP calls
 ******************************************************************************/
int plat_sip_handler(uint32_t smc_fid,
		     uint64_t x1,
		     uint64_t x2,
		     uint64_t x3,
		     uint64_t x4,
		     void *cookie,
		     void *handle,
		     uint64_t flags)
{
	int mce_ret;

	switch (smc_fid) {

	/*
	 * Micro Coded Engine (MCE) commands reside in the 0x82FFFF00 -
	 * 0x82FFFFFF SiP SMC space
	 */
	case TEGRA_SIP_MCE_CMD_ENTER_CSTATE:
	case TEGRA_SIP_MCE_CMD_UPDATE_CSTATE_INFO:
	case TEGRA_SIP_MCE_CMD_UPDATE_CROSSOVER_TIME:
	case TEGRA_SIP_MCE_CMD_READ_CSTATE_STATS:
	case TEGRA_SIP_MCE_CMD_WRITE_CSTATE_STATS:
	case TEGRA_SIP_MCE_CMD_IS_SC7_ALLOWED:
	case TEGRA_SIP_MCE_CMD_CC3_CTRL:
	case TEGRA_SIP_MCE_CMD_ECHO_DATA:
	case TEGRA_SIP_MCE_CMD_READ_VERSIONS:
	case TEGRA_SIP_MCE_CMD_ENUM_FEATURES:
	case TEGRA_SIP_MCE_CMD_ROC_FLUSH_CACHE_TRBITS:
	case TEGRA_SIP_MCE_CMD_ENUM_READ_MCA:
	case TEGRA_SIP_MCE_CMD_ENUM_WRITE_MCA:
	case TEGRA_SIP_MCE_CMD_ROC_FLUSH_CACHE:
	case TEGRA_SIP_MCE_CMD_ROC_CLEAN_CACHE:
	case TEGRA_SIP_MCE_CMD_ENABLE_LATIC:
	case TEGRA_SIP_MCE_CMD_UNCORE_PERFMON_REQ:
	case TEGRA_SIP_MCE_CMD_MISC_CCPLEX:

		/* clean up the high bits */
		smc_fid &= MCE_CMD_MASK;

		/* execute the command and store the result */
		mce_ret = mce_command_handler(smc_fid, x1, x2, x3);
		write_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X0, mce_ret);

		return 0;

	case TEGRA_SIP_SYSTEM_SHUTDOWN_STATE:

		/* clean up the high bits */
		x1 = (uint32_t)x1;

		/*
		 * SC8 is a special Tegra186 system state where the CPUs and
		 * DRAM are powered down but the other subsystem is still
		 * alive.
		 */

		return 0;

	default:
		break;
	}

	return -ENOTSUP;
}
