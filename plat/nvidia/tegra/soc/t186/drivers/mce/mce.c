/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <denver.h>
#include <mce.h>
#include <mmio.h>
#include <string.h>
#include <sys/errno.h>
#include <t18x_ari.h>
#include <tegra_def.h>

/* NVG functions handlers */
static arch_mce_ops_t nvg_mce_ops = {
	.enter_cstate = nvg_enter_cstate,
	.update_cstate_info = nvg_update_cstate_info,
	.update_crossover_time = nvg_update_crossover_time,
	.read_cstate_stats = nvg_read_cstate_stats,
	.write_cstate_stats = nvg_write_cstate_stats,
	.call_enum_misc = ari_enumeration_misc,
	.is_ccx_allowed = nvg_is_ccx_allowed,
	.is_sc7_allowed = nvg_is_sc7_allowed,
	.online_core = nvg_online_core,
	.cc3_ctrl = nvg_cc3_ctrl,
	.update_reset_vector = ari_reset_vector_update,
	.roc_flush_cache = ari_roc_flush_cache,
	.roc_flush_cache_trbits = ari_roc_flush_cache_trbits,
	.roc_clean_cache = ari_roc_clean_cache,
	.read_write_mca = ari_read_write_mca,
	.update_ccplex_gsc = ari_update_ccplex_gsc,
	.enter_ccplex_state = ari_enter_ccplex_state,
	.read_write_uncore_perfmon = ari_read_write_uncore_perfmon
};

/* ARI functions handlers */
static arch_mce_ops_t ari_mce_ops = {
	.enter_cstate = ari_enter_cstate,
	.update_cstate_info = ari_update_cstate_info,
	.update_crossover_time = ari_update_crossover_time,
	.read_cstate_stats = ari_read_cstate_stats,
	.write_cstate_stats = ari_write_cstate_stats,
	.call_enum_misc = ari_enumeration_misc,
	.is_ccx_allowed = ari_is_ccx_allowed,
	.is_sc7_allowed = ari_is_sc7_allowed,
	.online_core = ari_online_core,
	.cc3_ctrl = ari_cc3_ctrl,
	.update_reset_vector = ari_reset_vector_update,
	.roc_flush_cache = ari_roc_flush_cache,
	.roc_flush_cache_trbits = ari_roc_flush_cache_trbits,
	.roc_clean_cache = ari_roc_clean_cache,
	.read_write_mca = ari_read_write_mca,
	.update_ccplex_gsc = ari_update_ccplex_gsc,
	.enter_ccplex_state = ari_enter_ccplex_state,
	.read_write_uncore_perfmon = ari_read_write_uncore_perfmon
};

typedef struct mce_config {
	uint32_t ari_base;
	arch_mce_ops_t *ops;
} mce_config_t;

/* Table to hold the per-CPU ARI base address and function handlers */
static mce_config_t mce_cfg_table[MCE_ARI_APERTURES_MAX] = {
	{
		/* A57 Core 0 */
		.ari_base = TEGRA_MMCRAB_BASE + MCE_ARI_APERTURE_0_OFFSET,
		.ops = &ari_mce_ops,
	},
	{
		/* A57 Core 1 */
		.ari_base = TEGRA_MMCRAB_BASE + MCE_ARI_APERTURE_1_OFFSET,
		.ops = &ari_mce_ops,
	},
	{
		/* A57 Core 2 */
		.ari_base = TEGRA_MMCRAB_BASE + MCE_ARI_APERTURE_2_OFFSET,
		.ops = &ari_mce_ops,
	},
	{
		/* A57 Core 3 */
		.ari_base = TEGRA_MMCRAB_BASE + MCE_ARI_APERTURE_3_OFFSET,
		.ops = &ari_mce_ops,
	},
	{
		/* D15 Core 0 */
		.ari_base = TEGRA_MMCRAB_BASE + MCE_ARI_APERTURE_4_OFFSET,
		.ops = &nvg_mce_ops,
	},
	{
		/* D15 Core 1 */
		.ari_base = TEGRA_MMCRAB_BASE + MCE_ARI_APERTURE_5_OFFSET,
		.ops = &nvg_mce_ops,
	}
};

static uint32_t mce_get_curr_cpu_ari_base(void)
{
	uint32_t mpidr = read_mpidr();
	int cpuid =  mpidr & MPIDR_CPU_MASK;
	int impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;

	/*
	 * T186 has 2 CPU clusters, one with Denver CPUs and the other with
	 * ARM CortexA-57 CPUs. Each cluster consists of 4 CPUs and the CPU
	 * numbers start from 0. In order to get the proper arch_mce_ops_t
	 * struct, we have to convert the Denver CPU ids to the corresponding
	 * indices in the mce_ops_table array.
	 */
	if (impl == DENVER_IMPL)
		cpuid |= 0x4;

	return mce_cfg_table[cpuid].ari_base;
}

static arch_mce_ops_t *mce_get_curr_cpu_ops(void)
{
	uint32_t mpidr = read_mpidr();
	int cpuid =  mpidr & MPIDR_CPU_MASK;
	int impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;

	/*
	 * T186 has 2 CPU clusters, one with Denver CPUs and the other with
	 * ARM CortexA-57 CPUs. Each cluster consists of 4 CPUs and the CPU
	 * numbers start from 0. In order to get the proper arch_mce_ops_t
	 * struct, we have to convert the Denver CPU ids to the corresponding
	 * indices in the mce_ops_table array.
	 */
	if (impl == DENVER_IMPL)
		cpuid |= 0x4;

	return mce_cfg_table[cpuid].ops;
}

/*******************************************************************************
 * Common handler for all MCE commands
 ******************************************************************************/
int mce_command_handler(mce_cmd_t cmd, uint64_t arg0, uint64_t arg1,
			uint64_t arg2)
{
	arch_mce_ops_t *ops;
	uint32_t cpu_ari_base;
	uint64_t ret64 = 0, arg3, arg4, arg5;
	int ret = 0;
	mca_cmd_t mca_cmd;
	uncore_perfmon_req_t req;
	cpu_context_t *ctx = cm_get_context(NON_SECURE);
	gp_regs_t *gp_regs = get_gpregs_ctx(ctx);

	assert(ctx);
	assert(gp_regs);

	/* get a pointer to the CPU's arch_mce_ops_t struct */
	ops = mce_get_curr_cpu_ops();

	/* get the CPU's ARI base address */
	cpu_ari_base = mce_get_curr_cpu_ari_base();

	switch (cmd) {
	case MCE_CMD_ENTER_CSTATE:
		ret = ops->enter_cstate(cpu_ari_base, arg0, arg1);
		if (ret < 0)
			ERROR("%s: enter_cstate failed(%d)\n", __func__, ret);

		break;

	case MCE_CMD_UPDATE_CSTATE_INFO:
		/*
		 * get the parameters required for the update cstate info
		 * command
		 */
		arg3 = read_ctx_reg(gp_regs, CTX_GPREG_X4);
		arg4 = read_ctx_reg(gp_regs, CTX_GPREG_X5);
		arg5 = read_ctx_reg(gp_regs, CTX_GPREG_X6);

		ret = ops->update_cstate_info(cpu_ari_base, (uint32_t)arg0,
				(uint32_t)arg1, (uint32_t)arg2, (uint8_t)arg3,
				(uint32_t)arg4, (uint8_t)arg5);
		if (ret < 0)
			ERROR("%s: update_cstate_info failed(%d)\n",
				__func__, ret);

		write_ctx_reg(gp_regs, CTX_GPREG_X4, 0);
		write_ctx_reg(gp_regs, CTX_GPREG_X5, 0);
		write_ctx_reg(gp_regs, CTX_GPREG_X6, 0);

		break;

	case MCE_CMD_UPDATE_CROSSOVER_TIME:
		ret = ops->update_crossover_time(cpu_ari_base, arg0, arg1);
		if (ret < 0)
			ERROR("%s: update_crossover_time failed(%d)\n",
				__func__, ret);

		break;

	case MCE_CMD_READ_CSTATE_STATS:
		ret64 = ops->read_cstate_stats(cpu_ari_base, arg0);

		/* update context to return cstate stats value */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret64);
		write_ctx_reg(gp_regs, CTX_GPREG_X2, ret64);

		break;

	case MCE_CMD_WRITE_CSTATE_STATS:
		ret = ops->write_cstate_stats(cpu_ari_base, arg0, arg1);
		if (ret < 0)
			ERROR("%s: write_cstate_stats failed(%d)\n",
				__func__, ret);

		break;

	case MCE_CMD_IS_CCX_ALLOWED:
		ret = ops->is_ccx_allowed(cpu_ari_base, arg0, arg1);
		if (ret < 0) {
			ERROR("%s: is_ccx_allowed failed(%d)\n", __func__, ret);
			break;
		}

		/* update context to return CCx status value */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret);

		break;

	case MCE_CMD_IS_SC7_ALLOWED:
		ret = ops->is_sc7_allowed(cpu_ari_base, arg0, arg1);
		if (ret < 0) {
			ERROR("%s: is_sc7_allowed failed(%d)\n", __func__, ret);
			break;
		}

		/* update context to return SC7 status value */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret);
		write_ctx_reg(gp_regs, CTX_GPREG_X3, ret);

		break;

	case MCE_CMD_ONLINE_CORE:
		ret = ops->online_core(cpu_ari_base, arg0);
		if (ret < 0)
			ERROR("%s: online_core failed(%d)\n", __func__, ret);

		break;

	case MCE_CMD_CC3_CTRL:
		ret = ops->cc3_ctrl(cpu_ari_base, arg0, arg1, arg2);
		if (ret < 0)
			ERROR("%s: cc3_ctrl failed(%d)\n", __func__, ret);

		break;

	case MCE_CMD_ECHO_DATA:
		ret64 = ops->call_enum_misc(cpu_ari_base, TEGRA_ARI_MISC_ECHO,
				arg0);

		/* update context to return if echo'd data matched source */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret64 == arg0);
		write_ctx_reg(gp_regs, CTX_GPREG_X2, ret64 == arg0);

		break;

	case MCE_CMD_READ_VERSIONS:
		ret64 = ops->call_enum_misc(cpu_ari_base, TEGRA_ARI_MISC_VERSION,
			arg0);

		/*
		 * version = minor(63:32) | major(31:0). Update context
		 * to return major and minor version number.
		 */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, (uint32_t)ret64);
		write_ctx_reg(gp_regs, CTX_GPREG_X2, (uint32_t)(ret64 >> 32));

		break;

	case MCE_CMD_ENUM_FEATURES:
		ret = ops->call_enum_misc(cpu_ari_base,
				TEGRA_ARI_MISC_FEATURE_LEAF_0, arg0);

		/* update context to return features value */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret64);

		ret = 0;

		break;

	case MCE_CMD_ROC_FLUSH_CACHE_TRBITS:
		ret = ops->roc_flush_cache_trbits(cpu_ari_base);
		if (ret < 0)
			ERROR("%s: flush cache_trbits failed(%d)\n", __func__,
				ret);

		break;

	case MCE_CMD_ROC_FLUSH_CACHE:
		ret = ops->roc_flush_cache(cpu_ari_base);
		if (ret < 0)
			ERROR("%s: flush cache failed(%d)\n", __func__, ret);

		break;

	case MCE_CMD_ROC_CLEAN_CACHE:
		ret = ops->roc_clean_cache(cpu_ari_base);
		if (ret < 0)
			ERROR("%s: clean cache failed(%d)\n", __func__, ret);

		break;

	case MCE_CMD_ENUM_READ_MCA:
		memcpy(&mca_cmd, &arg0, sizeof(arg0));
		ret64 = ops->read_write_mca(cpu_ari_base, mca_cmd, &arg1);

		/* update context to return MCA data/error */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret64);
		write_ctx_reg(gp_regs, CTX_GPREG_X2, arg1);
		write_ctx_reg(gp_regs, CTX_GPREG_X3, ret64);

		break;

	case MCE_CMD_ENUM_WRITE_MCA:
		memcpy(&mca_cmd, &arg0, sizeof(arg0));
		ret64 = ops->read_write_mca(cpu_ari_base, mca_cmd, &arg1);

		/* update context to return MCA error */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ret64);
		write_ctx_reg(gp_regs, CTX_GPREG_X3, ret64);

		break;

#if ENABLE_CHIP_VERIFICATION_HARNESS
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
		ops->enter_ccplex_state(mce_get_curr_cpu_ari_base(),
			0xFFFF0000);

		break;
#endif

	case MCE_CMD_UNCORE_PERFMON_REQ:
		memcpy(&req, &arg0, sizeof(arg0));
		ret = ops->read_write_uncore_perfmon(cpu_ari_base, req, &arg1);

		/* update context to return data */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, arg1);
		break;

	default:
		ERROR("unknown MCE command (%d)\n", cmd);
		return EINVAL;
	}

	return ret;
}

/*******************************************************************************
 * Handler to update the reset vector for CPUs
 ******************************************************************************/
int mce_update_reset_vector(uint32_t addr_lo, uint32_t addr_hi)
{
	arch_mce_ops_t *ops = mce_get_curr_cpu_ops();

	ops->update_reset_vector(mce_get_curr_cpu_ari_base(), addr_lo, addr_hi);

	return 0;
}

static int mce_update_ccplex_gsc(tegra_ari_gsc_index_t gsc_idx)
{
	arch_mce_ops_t *ops = mce_get_curr_cpu_ops();

	ops->update_ccplex_gsc(mce_get_curr_cpu_ari_base(), gsc_idx);

	return 0;
}

/*******************************************************************************
 * Handler to update carveout values for Video Memory Carveout region
 ******************************************************************************/
int mce_update_gsc_videomem(void)
{
	return mce_update_ccplex_gsc(TEGRA_ARI_GSC_VPR_IDX);
}

/*******************************************************************************
 * Handler to update carveout values for TZDRAM aperture
 ******************************************************************************/
int mce_update_gsc_tzdram(void)
{
	return mce_update_ccplex_gsc(TEGRA_ARI_GSC_TZ_DRAM_IDX);
}

/*******************************************************************************
 * Handler to update carveout values for TZ SysRAM aperture
 ******************************************************************************/
int mce_update_gsc_tzram(void)
{
	return mce_update_ccplex_gsc(TEGRA_ARI_GSC_TZRAM);
}

/*******************************************************************************
 * Handler to shutdown/reset the entire system
 ******************************************************************************/
__dead2 void mce_enter_ccplex_state(uint32_t state_idx)
{
	arch_mce_ops_t *ops = mce_get_curr_cpu_ops();

	/* sanity check state value */
	if (state_idx != TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_POWER_OFF &&
	    state_idx != TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_REBOOT)
		panic();

	ops->enter_ccplex_state(mce_get_curr_cpu_ari_base(), state_idx);

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
	arch_mce_ops_t *ops = mce_get_curr_cpu_ops();

	/* issue the UPDATE_CSTATE_INFO request */
	ops->update_cstate_info(mce_get_curr_cpu_ari_base(), cstate->cluster,
		cstate->ccplex, cstate->system, cstate->system_state_force,
		cstate->wake_mask, cstate->update_wake_mask);
}

/*******************************************************************************
 * Handler to read the MCE firmware version and check if it is compatible
 * with interface header the BL3-1 was compiled against
 ******************************************************************************/
void mce_verify_firmware_version(void)
{
	arch_mce_ops_t *ops;
	uint32_t cpu_ari_base;
	uint64_t version;
	uint32_t major, minor, chip_minor, chip_major;

	/* get a pointer to the CPU's arch_mce_ops_t struct */
	ops = mce_get_curr_cpu_ops();

	/* get the CPU's ARI base address */
	cpu_ari_base = mce_get_curr_cpu_ari_base();

	/*
	 * Read the MCE firmware version and extract the major and minor
	 * version fields
	 */
	version = ops->call_enum_misc(cpu_ari_base, TEGRA_ARI_MISC_VERSION, 0);
	major = (uint32_t)version;
	minor = (uint32_t)(version >> 32);

	INFO("MCE Version - HW=%d:%d, SW=%d:%d\n", major, minor,
		TEGRA_ARI_VERSION_MAJOR, TEGRA_ARI_VERSION_MINOR);

	/*
	 * MCE firmware is not running on simulation platforms. Simulation
	 * platforms are identified by v0.3 from the Tegra Chip ID value.
	 */
	chip_major = (mmio_read_32(TEGRA_MISC_BASE + HARDWARE_REVISION_OFFSET) >>
			MAJOR_VERSION_SHIFT) & MAJOR_VERSION_MASK;
	chip_minor = (mmio_read_32(TEGRA_MISC_BASE + HARDWARE_REVISION_OFFSET) >>
			MINOR_VERSION_SHIFT) & MINOR_VERSION_MASK;
	if ((chip_major == 0) && (chip_minor == 3))
		return;

	/*
	 * Verify that the MCE firmware version and the interface header
	 * match
	 */
	if (major != TEGRA_ARI_VERSION_MAJOR) {
		ERROR("ARI major version mismatch\n");
		panic();
	}

	if (minor < TEGRA_ARI_VERSION_MINOR) {
		ERROR("ARI minor version mismatch\n");
		panic();
	}
}
