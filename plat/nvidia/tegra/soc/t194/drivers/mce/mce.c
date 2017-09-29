/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <common/debug.h>
#include <denver.h>
#include <mce.h>
#include <mce_private.h>
#include <mmio.h>
#include <platform_def.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <t194_nvg.h>
#include <tegra_def.h>
#include <tegra_platform.h>

/* Handler to check if MCE firmware is supported */
static bool mce_firmware_not_supported(void)
{
	bool status;

	/* these platforms do not load MCE firmware */
	status = tegra_platform_is_linsim() || tegra_platform_is_qt() ||
		 tegra_platform_is_virt_dev_kit();

	return status;
}

/*******************************************************************************
 * Common handler for all MCE commands
 ******************************************************************************/
int32_t mce_command_handler(uint64_t cmd, uint64_t arg0, uint64_t arg1,
			uint64_t arg2)
{
	uint64_t ret64 = 0, arg3, arg4, arg5;
	int32_t ret = 0;
	cpu_context_t *ctx = cm_get_context(NON_SECURE);
	gp_regs_t *gp_regs = get_gpregs_ctx(ctx);

	assert(ctx);
	assert(gp_regs);

	switch (cmd) {
	case MCE_CMD_ENTER_CSTATE:
		ret = nvg_enter_cstate((uint32_t)arg0, (uint32_t)arg1);
		if (ret < 0) {
			ERROR("%s: enter_cstate failed(%d)\n", __func__, ret);
		}

		break;

	case MCE_CMD_UPDATE_CSTATE_INFO:
		/*
		 * get the parameters required for the update cstate info
		 * command
		 */
		arg3 = read_ctx_reg(gp_regs, ((uint64_t)CTX_GPREG_X4));
		arg4 = read_ctx_reg(gp_regs, ((uint64_t)CTX_GPREG_X5));
		arg5 = read_ctx_reg(gp_regs, ((uint64_t)CTX_GPREG_X6));

		/* arg0 cluster
		 * arg1 ccplex
		 * arg2 system
		 * arg3 sys_state_force => T19x not support
		 * arg4 wake_mask
		 * arg5 update_wake_mask
		 */
		nvg_update_cstate_info((uint32_t)arg0, (uint32_t)arg1,
				(uint32_t)arg2, (uint32_t)arg4, (uint8_t)arg5);

		write_ctx_reg(gp_regs, ((uint64_t)CTX_GPREG_X4), (arg3));
		write_ctx_reg(gp_regs, ((uint64_t)CTX_GPREG_X5), (arg4));
		write_ctx_reg(gp_regs, ((uint64_t)CTX_GPREG_X6), (arg5));

		break;

	case MCE_CMD_UPDATE_CROSSOVER_TIME:
		ret = nvg_update_crossover_time((uint32_t)arg0, (uint32_t)arg1);
		if (ret < 0) {
			ERROR("%s: update_crossover_time failed(%d)\n",
				__func__, ret);
		}

		break;

	case MCE_CMD_READ_CSTATE_STATS:
		ret64 = nvg_get_cstate_stat_query_value();

		/* update context to return cstate stats value */
		write_ctx_reg(gp_regs, ((uint64_t)CTX_GPREG_X1), (ret64));
		write_ctx_reg(gp_regs, ((uint64_t)CTX_GPREG_X2), (ret64));

		break;

	case MCE_CMD_WRITE_CSTATE_STATS:
		ret = nvg_set_cstate_stat_query_value(arg0);

		break;

	case MCE_CMD_IS_SC7_ALLOWED:
		ret = nvg_is_sc7_allowed();
		if (ret < 0) {
			ERROR("%s: is_sc7_allowed failed(%d)\n", __func__, ret);
			break;
		}

		/* update context to return SC7 status value */
		write_ctx_reg(gp_regs, ((uint64_t)CTX_GPREG_X1), ((uint64_t)ret));
		write_ctx_reg(gp_regs, ((uint64_t)CTX_GPREG_X3), ((uint64_t)ret));

		break;

	case MCE_CMD_ONLINE_CORE:
		ret = nvg_online_core((uint32_t)arg0);
		if (ret < 0) {
			ERROR("%s: online_core failed(%d)\n", __func__, ret);
		}

		break;

	case MCE_CMD_CC3_CTRL:
		ret = nvg_cc3_ctrl((uint32_t)arg0, (uint8_t)arg2);
		if (ret < 0) {
			ERROR("%s: cc3_ctrl failed(%d)\n", __func__, ret);
		}

		break;

	case MCE_CMD_READ_VERSIONS:
		/* get the MCE firmware version */
		ret64 = nvg_get_version();

		/*
		 * version = minor(63:32) | major(31:0). Update context
		 * to return major and minor version number.
		 */
		write_ctx_reg(gp_regs, ((uint64_t)CTX_GPREG_X1), (ret64 & (uint64_t)0xFFFF));
		write_ctx_reg(gp_regs, ((uint64_t)CTX_GPREG_X2), (ret64 >> 32));

		break;

	case MCE_CMD_ROC_FLUSH_CACHE_TRBITS:
		ret = nvg_roc_clean_cache_trbits();
		if (ret < 0) {
			ERROR("%s: flush cache_trbits failed(%d)\n", __func__,
				ret);
		}

		break;

	case MCE_CMD_ROC_FLUSH_CACHE:
		ret = nvg_roc_flush_cache();
		if (ret < 0) {
			ERROR("%s: flush cache failed(%d)\n", __func__, ret);
		}

		break;

	case MCE_CMD_ROC_CLEAN_CACHE:
		ret = nvg_roc_clean_cache();
		if (ret < 0) {
			ERROR("%s: clean cache failed(%d)\n", __func__, ret);
		}

		break;

	default:
		ERROR("unknown MCE command (%llu)\n", cmd);
		ret = EINVAL;
		break;
	}

	return ret;
}

/*******************************************************************************
 * Handler to update carveout values for Video Memory Carveout region
 ******************************************************************************/
int32_t mce_update_gsc_videomem(void)
{
	int32_t ret;

	/*
	 * MCE firmware is not running on simulation platforms.
	 */
	if (mce_firmware_not_supported()) {
		ret = -EINVAL;
	} else {
		ret = nvg_update_ccplex_gsc((uint32_t)TEGRA_NVG_CHANNEL_UPDATE_GSC_VPR);
	}

	return ret;
}

/*******************************************************************************
 * Handler to update carveout values for TZDRAM aperture
 ******************************************************************************/
int32_t mce_update_gsc_tzdram(void)
{
	int32_t ret;

	/*
	 * MCE firmware is not running on simulation platforms.
	 */
	if (mce_firmware_not_supported()) {
		ret = -EINVAL;
	} else {
		ret = nvg_update_ccplex_gsc((uint32_t)TEGRA_NVG_CHANNEL_UPDATE_GSC_TZ_DRAM);
	}

	return ret;
}

/*******************************************************************************
 * Handler to update carveout values for TZ SysRAM aperture
 ******************************************************************************/
int32_t mce_update_gsc_tzram(void)
{
	int32_t ret;

	/*
	 * MCE firmware is not running on simulation platforms.
	 */
	if (mce_firmware_not_supported()) {
		ret = -EINVAL;
	} else {
		ret = nvg_update_ccplex_gsc((uint32_t)TEGRA_NVG_CHANNEL_UPDATE_GSC_TZRAM);
	}

	return ret;
}

/*******************************************************************************
 * Handler to issue the UPDATE_CSTATE_INFO request
 ******************************************************************************/
void mce_update_cstate_info(const mce_cstate_info_t *cstate)
{
	/* issue the UPDATE_CSTATE_INFO request */
	nvg_update_cstate_info(cstate->cluster, cstate->ccplex, cstate->system,
		cstate->wake_mask, cstate->update_wake_mask);
}

/*******************************************************************************
 * Handler to read the MCE firmware version and check if it is compatible
 * with interface header the BL3-1 was compiled against
 ******************************************************************************/
void mce_verify_firmware_version(void)
{
	uint64_t version;
	uint32_t major, minor;

	/*
	 * MCE firmware is not running on simulation platforms.
	 */
	if (mce_firmware_not_supported()) {
		return;
	}

	/*
	 * Read the MCE firmware version and extract the major and minor
	 * version fields
	 */
	version = nvg_get_version();
	minor = (uint32_t)version;
	major = (uint32_t)(version >> 32);

	INFO("MCE Version - HW=%d:%d, SW=%d:%d\n", major, minor,
		0, 0);

	/*
	 * Verify that the MCE firmware version and the interface header
	 * match
	 */
	if (major != (uint32_t)TEGRA_NVG_VERSION_MAJOR) {
		ERROR("MCE major version mismatch\n");
		panic();
	}

	if (minor < (uint32_t)TEGRA_NVG_VERSION_MINOR) {
		ERROR("MCE minor version mismatch\n");
		panic();
	}
}
