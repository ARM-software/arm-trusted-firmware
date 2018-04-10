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
#include <string.h>
#include <errno.h>
#include <tegra_def.h>
#include <tegra_platform.h>

/*******************************************************************************
 * Common handler for all MCE commands
 ******************************************************************************/
int mce_command_handler(mce_cmd_t cmd, uint64_t arg0, uint64_t arg1,
			uint64_t arg2)
{
	uint64_t ret64 = 0, arg3, arg4, arg5;
	int ret = 0;
	mca_cmd_t mca_cmd;
	uncore_perfmon_req_t req;
	cpu_context_t *ctx = cm_get_context(NON_SECURE);
	gp_regs_t *gp_regs = get_gpregs_ctx(ctx);

	assert(ctx);
	assert(gp_regs);

	switch (cmd) {
	case MCE_CMD_ENTER_CSTATE:
		/* NVG */
		break;

	case MCE_CMD_UPDATE_CSTATE_INFO:
		/*
		 * get the parameters required for the update cstate info
		 * command
		 */
		arg3 = read_ctx_reg(gp_regs, CTX_GPREG_X4);
		arg4 = read_ctx_reg(gp_regs, CTX_GPREG_X5);
		arg5 = read_ctx_reg(gp_regs, CTX_GPREG_X6);

		/* NVG */

		write_ctx_reg(gp_regs, CTX_GPREG_X4, arg3);
		write_ctx_reg(gp_regs, CTX_GPREG_X5, arg4);
		write_ctx_reg(gp_regs, CTX_GPREG_X6, arg5);

		break;

	case MCE_CMD_UPDATE_CROSSOVER_TIME:
		/* NVG */

		break;

	case MCE_CMD_READ_CSTATE_STATS:
		/* NVG */

		/* update context to return cstate stats value */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret64);
		write_ctx_reg(gp_regs, CTX_GPREG_X2, ret64);

		break;

	case MCE_CMD_WRITE_CSTATE_STATS:
		/* NVG */

		break;

	case MCE_CMD_IS_CCX_ALLOWED:
		/* NVG */

		/* update context to return CCx status value */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret);

		break;

	case MCE_CMD_IS_SC7_ALLOWED:
		/* NVG */

		/* update context to return SC7 status value */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret);
		write_ctx_reg(gp_regs, CTX_GPREG_X3, ret);

		break;

	case MCE_CMD_ONLINE_CORE:
		/* NVG */

		break;

	case MCE_CMD_CC3_CTRL:
		/* NVG */

		break;

	case MCE_CMD_ECHO_DATA:
		/* issue NVG to echo data */

		/* update context to return if echo'd data matched source */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret64 == arg0);
		write_ctx_reg(gp_regs, CTX_GPREG_X2, ret64 == arg0);

		break;

	case MCE_CMD_READ_VERSIONS:
		/* get the MCE firmware version */

		/*
		 * version = minor(63:32) | major(31:0). Update context
		 * to return major and minor version number.
		 */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, (uint32_t)ret64);
		write_ctx_reg(gp_regs, CTX_GPREG_X2, (uint32_t)(ret64 >> 32));

		break;

	case MCE_CMD_ENUM_FEATURES:
		break;

	case MCE_CMD_ROC_FLUSH_CACHE_TRBITS:
		/* NVG */

		break;

	case MCE_CMD_ROC_FLUSH_CACHE:
		/* NVG */

		break;

	case MCE_CMD_ROC_CLEAN_CACHE:
		/* NVG */

		break;

	case MCE_CMD_ENUM_READ_MCA:
		memcpy(&mca_cmd, &arg0, sizeof(arg0));

		/* NVG */

		/* update context to return MCA data/error */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret64);
		write_ctx_reg(gp_regs, CTX_GPREG_X2, arg1);
		write_ctx_reg(gp_regs, CTX_GPREG_X3, ret64);

		break;

	case MCE_CMD_ENUM_WRITE_MCA:
		memcpy(&mca_cmd, &arg0, sizeof(arg0));

		/* NVG */

		/* update context to return MCA error */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret64);
		write_ctx_reg(gp_regs, CTX_GPREG_X3, ret64);

		break;

	case MCE_CMD_ENABLE_LATIC:
		/*
		 * This call is not for production use. The constant value,
		 * 0xFFFF0000, is specific to allowing for enabling LATIC on
		 * pre-production parts for the chip verification harness.
		 *
		 * Enabling LATIC allows S/W to read the MINI ISPs in the
		 * CCPLEX. The ISMs are used for various measurements relevant
		 * to particular locations in the Silicon. They are small
		 * counters which can be polled to determine how fast a
		 * particular location in the Silicon is.
		 */
		/* NVG */

		break;

	case MCE_CMD_UNCORE_PERFMON_REQ:
		memcpy(&req, &arg0, sizeof(arg0));
		/* NVG */

		/* update context to return data */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, arg1);
		break;

	case MCE_CMD_MISC_CCPLEX:
		/* NVG */

		break;

	default:
		ERROR("unknown MCE command (%lld)\n", cmd);
		return EINVAL;
	}

	return ret;
}

/*******************************************************************************
 * Handler to update the reset vector for CPUs
 ******************************************************************************/
int mce_update_reset_vector(void)
{
	return 0;
}

static int mce_update_ccplex_gsc(/* GSC ID */)
{
	return 0;
}

/*******************************************************************************
 * Handler to update carveout values for Video Memory Carveout region
 ******************************************************************************/
int mce_update_gsc_videomem(void)
{
	return mce_update_ccplex_gsc();
}

/*******************************************************************************
 * Handler to update carveout values for TZDRAM aperture
 ******************************************************************************/
int mce_update_gsc_tzdram(void)
{
	return mce_update_ccplex_gsc();
}

/*******************************************************************************
 * Handler to update carveout values for TZ SysRAM aperture
 ******************************************************************************/
int mce_update_gsc_tzram(void)
{
	return mce_update_ccplex_gsc();
}

/*******************************************************************************
 * Handler to shutdown/reset the entire system
 ******************************************************************************/
__dead2 void mce_enter_ccplex_state(uint32_t state_idx)
{
	/* sanity check state value */

	/* enter ccplex power state */

	/* wait till the CCPLEX powers down */
	for (;;)
		;

	panic();
}

/*******************************************************************************
 * Handler to issue the UPDATE_CSTATE_INFO request
 ******************************************************************************/
void mce_update_cstate_info(mce_cstate_info_t *cstate)
{
	/* issue the UPDATE_CSTATE_INFO request */
	/* NVG */
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
	if (tegra_platform_is_linsim() || tegra_platform_is_virt_dev_kit())
		return;

	/* get a pointer to the CPU's arch_mce_ops_t struct */

	/*
	 * Read the MCE firmware version and extract the major and minor
	 * version fields
	 */
	version = 0;
	major = (uint32_t)version;
	minor = (uint32_t)(version >> 32);

	INFO("MCE Version - HW=%d:%d, SW=%d:%d\n", major, minor,
		0, 0);

	/*
	 * Verify that the MCE firmware version and the interface header
	 * match
	 */
	if (major != 0) {
		ERROR("MCE major version mismatch\n");
		panic();
	}

	if (minor < 0) {
		ERROR("MCE minor version mismatch\n");
		panic();
	}
}
