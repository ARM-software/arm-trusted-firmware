/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <debug.h>
#include <denver.h>
#include <errno.h>
#include <mce.h>
#include <memctrl.h>
#include <runtime_svc.h>
#include <t18x_ari.h>
#include <tegra_private.h>

extern uint32_t tegra186_system_powerdn_state;

/*******************************************************************************
 * Offset to read the ref_clk counter value
 ******************************************************************************/
#define REF_CLK_OFFSET		4

/*******************************************************************************
 * Tegra186 SiP SMCs
 ******************************************************************************/
#define TEGRA_SIP_SYSTEM_SHUTDOWN_STATE			0xC2FFFE01
#define TEGRA_SIP_GET_ACTMON_CLK_COUNTERS		0xC2FFFE02
#define TEGRA_SIP_MCE_CMD_ENTER_CSTATE			0xC2FFFF00
#define TEGRA_SIP_MCE_CMD_UPDATE_CSTATE_INFO		0xC2FFFF01
#define TEGRA_SIP_MCE_CMD_UPDATE_CROSSOVER_TIME		0xC2FFFF02
#define TEGRA_SIP_MCE_CMD_READ_CSTATE_STATS		0xC2FFFF03
#define TEGRA_SIP_MCE_CMD_WRITE_CSTATE_STATS		0xC2FFFF04
#define TEGRA_SIP_MCE_CMD_IS_SC7_ALLOWED		0xC2FFFF05
#define TEGRA_SIP_MCE_CMD_ONLINE_CORE			0xC2FFFF06
#define TEGRA_SIP_MCE_CMD_CC3_CTRL			0xC2FFFF07
#define TEGRA_SIP_MCE_CMD_ECHO_DATA			0xC2FFFF08
#define TEGRA_SIP_MCE_CMD_READ_VERSIONS			0xC2FFFF09
#define TEGRA_SIP_MCE_CMD_ENUM_FEATURES			0xC2FFFF0A
#define TEGRA_SIP_MCE_CMD_ROC_FLUSH_CACHE_TRBITS	0xC2FFFF0B
#define TEGRA_SIP_MCE_CMD_ENUM_READ_MCA			0xC2FFFF0C
#define TEGRA_SIP_MCE_CMD_ENUM_WRITE_MCA		0xC2FFFF0D
#define TEGRA_SIP_MCE_CMD_ROC_FLUSH_CACHE		0xC2FFFF0E
#define TEGRA_SIP_MCE_CMD_ROC_CLEAN_CACHE		0xC2FFFF0F
#define TEGRA_SIP_MCE_CMD_ENABLE_LATIC			0xC2FFFF10
#define TEGRA_SIP_MCE_CMD_UNCORE_PERFMON_REQ		0xC2FFFF11
#define TEGRA_SIP_MCE_CMD_MISC_CCPLEX			0xC2FFFF12

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
	int impl, cpu;
	uint32_t base, core_clk_ctr, ref_clk_ctr;

	if (((smc_fid >> FUNCID_CC_SHIFT) & FUNCID_CC_MASK) == SMC_32) {
		/* 32-bit function, clear top parameter bits */

		x1 = (uint32_t)x1;
		x2 = (uint32_t)x2;
		x3 = (uint32_t)x3;
	}

	/*
	 * Convert SMC FID to SMC64, to support SMC32/SMC64 configurations
	 */
	smc_fid |= (SMC_64 << FUNCID_CC_SHIFT);

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
		write_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X0,
			      (uint64_t)mce_ret);

		return 0;

	case TEGRA_SIP_SYSTEM_SHUTDOWN_STATE:

		/* clean up the high bits */
		x1 = (uint32_t)x1;

		/*
		 * SC8 is a special Tegra186 system state where the CPUs and
		 * DRAM are powered down but the other subsystem is still
		 * alive.
		 */
		if ((x1 == TEGRA_ARI_SYSTEM_SC8) ||
		    (x1 == TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_POWER_OFF)) {

			tegra186_system_powerdn_state = x1;
			flush_dcache_range(
				(uintptr_t)&tegra186_system_powerdn_state,
				sizeof(tegra186_system_powerdn_state));

		} else {

			ERROR("%s: unhandled powerdn state (%d)\n", __func__,
				(uint32_t)x1);
			return -ENOTSUP;
		}

		return 0;

	/*
	 * This function ID reads the Activity monitor's core/ref clock
	 * counter values for a core/cluster.
	 *
	 * x1 = MPIDR of the target core
	 * x2 = MIDR of the target core
	 */
	case TEGRA_SIP_GET_ACTMON_CLK_COUNTERS:

		cpu = (uint32_t)x1 & MPIDR_CPU_MASK;
		impl = ((uint32_t)x2 >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;

		/* sanity check target CPU number */
		if (cpu > PLATFORM_MAX_CPUS_PER_CLUSTER)
			return -EINVAL;

		/* get the base address for the current CPU */
		base = (impl == DENVER_IMPL) ? TEGRA_DENVER_ACTMON_CTR_BASE :
			TEGRA_ARM_ACTMON_CTR_BASE;

		/* read the clock counter values */
		core_clk_ctr = mmio_read_32(base + (8 * cpu));
		ref_clk_ctr = mmio_read_32(base + (8 * cpu) + REF_CLK_OFFSET);

		/* return the counter values as two different parameters */
		write_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X1,
			      (uint64_t)core_clk_ctr);
		write_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X2,
			      (uint64_t)ref_clk_ctr);

		return 0;

	default:
		break;
	}

	return -ENOTSUP;
}
