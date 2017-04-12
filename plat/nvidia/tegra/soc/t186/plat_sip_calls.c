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
#define TEGRA_SIP_SYSTEM_SHUTDOWN_STATE			0x82FFFE01
#define TEGRA_SIP_GET_ACTMON_CLK_COUNTERS		0x82FFFE02
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
	int impl, cpu;
	uint32_t base, core_clk_ctr, ref_clk_ctr;

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
		write_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X1, core_clk_ctr);
		write_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X2, ref_clk_ctr);

		return 0;

	default:
		break;
	}

	return -ENOTSUP;
}
