/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <denver.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>

#include <mce.h>
#include <mce_private.h>
#include <t18x_ari.h>
#include <tegra_def.h>
#include <tegra_platform.h>

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
	.read_write_uncore_perfmon = ari_read_write_uncore_perfmon,
	.misc_ccplex = ari_misc_ccplex
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
	.read_write_uncore_perfmon = ari_read_write_uncore_perfmon,
	.misc_ccplex = ari_misc_ccplex
};

typedef struct {
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
	uint64_t mpidr = read_mpidr();
	uint64_t cpuid = mpidr & MPIDR_CPU_MASK;
	uint64_t impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;

	/*
	 * T186 has 2 CPU clusters, one with Denver CPUs and the other with
	 * ARM CortexA-57 CPUs. Each cluster consists of 4 CPUs and the CPU
	 * numbers start from 0. In order to get the proper arch_mce_ops_t
	 * struct, we have to convert the Denver CPU ids to the corresponding
	 * indices in the mce_ops_table array.
	 */
	if (impl == DENVER_IMPL) {
		cpuid |= 0x4U;
	}

	return mce_cfg_table[cpuid].ari_base;
}

static arch_mce_ops_t *mce_get_curr_cpu_ops(void)
{
	uint64_t mpidr = read_mpidr();
	uint64_t cpuid = mpidr & MPIDR_CPU_MASK;
	uint64_t impl = (read_midr() >> MIDR_IMPL_SHIFT) &
			MIDR_IMPL_MASK;

	/*
	 * T186 has 2 CPU clusters, one with Denver CPUs and the other with
	 * ARM CortexA-57 CPUs. Each cluster consists of 4 CPUs and the CPU
	 * numbers start from 0. In order to get the proper arch_mce_ops_t
	 * struct, we have to convert the Denver CPU ids to the corresponding
	 * indices in the mce_ops_table array.
	 */
	if (impl == DENVER_IMPL) {
		cpuid |= 0x4U;
	}

	return mce_cfg_table[cpuid].ops;
}

/*******************************************************************************
 * Common handler for all MCE commands
 ******************************************************************************/
int32_t mce_command_handler(uint64_t cmd, uint64_t arg0, uint64_t arg1,
			uint64_t arg2)
{
	const arch_mce_ops_t *ops;
	gp_regs_t *gp_regs = get_gpregs_ctx(cm_get_context(NON_SECURE));
	uint32_t cpu_ari_base;
	uint64_t ret64 = 0, arg3, arg4, arg5;
	int32_t ret = 0;

	assert(gp_regs != NULL);

	/* get a pointer to the CPU's arch_mce_ops_t struct */
	ops = mce_get_curr_cpu_ops();

	/* get the CPU's ARI base address */
	cpu_ari_base = mce_get_curr_cpu_ari_base();

	switch (cmd) {
	case (uint64_t)MCE_CMD_ENTER_CSTATE:
		ret = ops->enter_cstate(cpu_ari_base, arg0, arg1);

		break;

	case (uint64_t)MCE_CMD_UPDATE_CSTATE_INFO:
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

		write_ctx_reg(gp_regs, CTX_GPREG_X4, (0ULL));
		write_ctx_reg(gp_regs, CTX_GPREG_X5, (0ULL));
		write_ctx_reg(gp_regs, CTX_GPREG_X6, (0ULL));

		break;

	case (uint64_t)MCE_CMD_UPDATE_CROSSOVER_TIME:
		ret = ops->update_crossover_time(cpu_ari_base, arg0, arg1);

		break;

	case (uint64_t)MCE_CMD_READ_CSTATE_STATS:
		ret64 = ops->read_cstate_stats(cpu_ari_base, arg0);

		/* update context to return cstate stats value */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, (ret64));
		write_ctx_reg(gp_regs, CTX_GPREG_X2, (ret64));

		break;

	case (uint64_t)MCE_CMD_WRITE_CSTATE_STATS:
		ret = ops->write_cstate_stats(cpu_ari_base, arg0, arg1);

		break;

	case (uint64_t)MCE_CMD_IS_CCX_ALLOWED:
		ret = ops->is_ccx_allowed(cpu_ari_base, arg0, arg1);

		/* update context to return CCx status value */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, (uint64_t)(ret));

		break;

	case (uint64_t)MCE_CMD_IS_SC7_ALLOWED:
		ret = ops->is_sc7_allowed(cpu_ari_base, arg0, arg1);

		/* update context to return SC7 status value */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, (uint64_t)(ret));
		write_ctx_reg(gp_regs, CTX_GPREG_X3, (uint64_t)(ret));

		break;

	case (uint64_t)MCE_CMD_ONLINE_CORE:
		ret = ops->online_core(cpu_ari_base, arg0);

		break;

	case (uint64_t)MCE_CMD_CC3_CTRL:
		ret = ops->cc3_ctrl(cpu_ari_base, arg0, arg1, arg2);

		break;

	case (uint64_t)MCE_CMD_ECHO_DATA:
		ret64 = ops->call_enum_misc(cpu_ari_base, TEGRA_ARI_MISC_ECHO,
				arg0);

		/* update context to return if echo'd data matched source */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, ((ret64 == arg0) ?
			      1ULL : 0ULL));
		write_ctx_reg(gp_regs, CTX_GPREG_X2, ((ret64 == arg0) ?
			      1ULL : 0ULL));

		break;

	case (uint64_t)MCE_CMD_READ_VERSIONS:
		ret64 = ops->call_enum_misc(cpu_ari_base, TEGRA_ARI_MISC_VERSION,
			arg0);

		/*
		 * version = minor(63:32) | major(31:0). Update context
		 * to return major and minor version number.
		 */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, (ret64));
		write_ctx_reg(gp_regs, CTX_GPREG_X2, (ret64 >> 32ULL));

		break;

	case (uint64_t)MCE_CMD_ENUM_FEATURES:
		ret64 = ops->call_enum_misc(cpu_ari_base,
				TEGRA_ARI_MISC_FEATURE_LEAF_0, arg0);

		/* update context to return features value */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, (ret64));

		break;

	case (uint64_t)MCE_CMD_ROC_FLUSH_CACHE_TRBITS:
		ret = ops->roc_flush_cache_trbits(cpu_ari_base);

		break;

	case (uint64_t)MCE_CMD_ROC_FLUSH_CACHE:
		ret = ops->roc_flush_cache(cpu_ari_base);

		break;

	case (uint64_t)MCE_CMD_ROC_CLEAN_CACHE:
		ret = ops->roc_clean_cache(cpu_ari_base);

		break;

	case (uint64_t)MCE_CMD_ENUM_READ_MCA:
		ret64 = ops->read_write_mca(cpu_ari_base, arg0, &arg1);

		/* update context to return MCA data/error */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, (ret64));
		write_ctx_reg(gp_regs, CTX_GPREG_X2, (arg1));
		write_ctx_reg(gp_regs, CTX_GPREG_X3, (ret64));

		break;

	case (uint64_t)MCE_CMD_ENUM_WRITE_MCA:
		ret64 = ops->read_write_mca(cpu_ari_base, arg0, &arg1);

		/* update context to return MCA error */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, (ret64));
		write_ctx_reg(gp_regs, CTX_GPREG_X3, (ret64));

		break;

#if ENABLE_CHIP_VERIFICATION_HARNESS
	case (uint64_t)MCE_CMD_ENABLE_LATIC:
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

	case (uint64_t)MCE_CMD_UNCORE_PERFMON_REQ:
		ret = ops->read_write_uncore_perfmon(cpu_ari_base, arg0, &arg1);

		/* update context to return data */
		write_ctx_reg(gp_regs, CTX_GPREG_X1, (arg1));
		break;

	case (uint64_t)MCE_CMD_MISC_CCPLEX:
		ops->misc_ccplex(cpu_ari_base, arg0, arg1);

		break;

	default:
		ERROR("unknown MCE command (%llu)\n", cmd);
		ret = EINVAL;
		break;
	}

	return ret;
}

/*******************************************************************************
 * Handler to update the reset vector for CPUs
 ******************************************************************************/
int32_t mce_update_reset_vector(void)
{
	const arch_mce_ops_t *ops = mce_get_curr_cpu_ops();

	ops->update_reset_vector(mce_get_curr_cpu_ari_base());

	return 0;
}

static int32_t mce_update_ccplex_gsc(tegra_ari_gsc_index_t gsc_idx)
{
	const arch_mce_ops_t *ops = mce_get_curr_cpu_ops();

	ops->update_ccplex_gsc(mce_get_curr_cpu_ari_base(), gsc_idx);

	return 0;
}

/*******************************************************************************
 * Handler to update carveout values for Video Memory Carveout region
 ******************************************************************************/
int32_t mce_update_gsc_videomem(void)
{
	return mce_update_ccplex_gsc(TEGRA_ARI_GSC_VPR_IDX);
}

/*******************************************************************************
 * Handler to update carveout values for TZDRAM aperture
 ******************************************************************************/
int32_t mce_update_gsc_tzdram(void)
{
	return mce_update_ccplex_gsc(TEGRA_ARI_GSC_TZ_DRAM_IDX);
}

/*******************************************************************************
 * Handler to update carveout values for TZ SysRAM aperture
 ******************************************************************************/
int32_t mce_update_gsc_tzram(void)
{
	return mce_update_ccplex_gsc(TEGRA_ARI_GSC_TZRAM);
}

/*******************************************************************************
 * Handler to shutdown/reset the entire system
 ******************************************************************************/
__dead2 void mce_enter_ccplex_state(uint32_t state_idx)
{
	const arch_mce_ops_t *ops = mce_get_curr_cpu_ops();

	/* sanity check state value */
	if ((state_idx != TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_POWER_OFF) &&
	    (state_idx != TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_REBOOT)) {
		panic();
	}

	ops->enter_ccplex_state(mce_get_curr_cpu_ari_base(), state_idx);

	/* wait till the CCPLEX powers down */
	for (;;) {
		;
	}

}

/*******************************************************************************
 * Handler to issue the UPDATE_CSTATE_INFO request
 ******************************************************************************/
void mce_update_cstate_info(const mce_cstate_info_t *cstate)
{
	const arch_mce_ops_t *ops = mce_get_curr_cpu_ops();

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
	const arch_mce_ops_t *ops;
	uint32_t cpu_ari_base;
	uint64_t version;
	uint32_t major, minor;

	/*
	 * MCE firmware is not supported on simulation platforms.
	 */
	if (tegra_platform_is_emulation()) {

		INFO("MCE firmware is not supported\n");

	} else {
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
}
