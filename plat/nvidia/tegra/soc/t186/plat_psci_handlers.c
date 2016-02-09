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
#include <psci.h>
#include <t18x_ari.h>
#include <tegra_private.h>

/* state id mask */
#define TEGRA186_STATE_ID_MASK		0xF
/* constants to get power state's wake time */
#define TEGRA186_WAKE_TIME_MASK		0xFFFFFF
#define TEGRA186_WAKE_TIME_SHIFT	4

static unsigned int wake_time[PLATFORM_CORE_COUNT];

int32_t tegra_soc_validate_power_state(unsigned int power_state,
					psci_power_state_t *req_state)
{
	int state_id = psci_get_pstate_id(power_state) & TEGRA186_STATE_ID_MASK;
	int cpu = read_mpidr() & MPIDR_CPU_MASK;
	int impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;

	if (impl == DENVER_IMPL)
		cpu |= 0x4;

	wake_time[cpu] = (power_state  >> TEGRA186_WAKE_TIME_SHIFT) &
			 TEGRA186_WAKE_TIME_MASK;

	/* Sanity check the requested state id */
	switch (state_id) {
	case PSTATE_ID_CORE_IDLE:
	case PSTATE_ID_CORE_POWERDN:
		/*
		 * Core powerdown request only for afflvl 0
		 */
		req_state->pwr_domain_state[MPIDR_AFFLVL0] = state_id;

		break;

	default:
		ERROR("%s: unsupported state id (%d)\n", __func__, state_id);
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	const plat_local_state_t *pwr_domain_state;
	unsigned int stateid_afflvl0;
	int cpu = read_mpidr() & MPIDR_CPU_MASK;
	int impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;

	if (impl == DENVER_IMPL)
		cpu |= 0x4;

	/* get the state ID */
	pwr_domain_state = target_state->pwr_domain_state;
	stateid_afflvl0 = pwr_domain_state[MPIDR_AFFLVL0] &
		TEGRA186_STATE_ID_MASK;

	if (stateid_afflvl0 == PSTATE_ID_CORE_IDLE) {

		/* Prepare for cpu idle */
		(void)mce_command_handler(MCE_CMD_ENTER_CSTATE,
			TEGRA_ARI_CORE_C6, wake_time[cpu], 0);

	} else if (stateid_afflvl0 == PSTATE_ID_CORE_POWERDN) {

		/* Prepare for cpu powerdn */
		(void)mce_command_handler(MCE_CMD_ENTER_CSTATE,
			TEGRA_ARI_CORE_C7, wake_time[cpu], 0);

	} else {
		ERROR("%s: Unknown state id\n", __func__);
		return PSCI_E_NOT_SUPPORTED;
	}

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_on(u_register_t mpidr)
{
	int target_cpu = mpidr & MPIDR_CPU_MASK;
	int target_cluster = (mpidr & MPIDR_CLUSTER_MASK) >>
			MPIDR_AFFINITY_BITS;

	if (target_cluster > MPIDR_AFFLVL1) {
		ERROR("%s: unsupported CPU (0x%lx)\n", __func__, mpidr);
		return PSCI_E_NOT_PRESENT;
	}

	/* construct the target CPU # */
	target_cpu |= (target_cluster << 2);

	mce_command_handler(MCE_CMD_ONLINE_CORE, target_cpu, 0, 0);

	return PSCI_E_SUCCESS;
}

int tegra_soc_pwr_domain_off(const psci_power_state_t *target_state)
{
	cpu_context_t *ctx = cm_get_context(NON_SECURE);
	gp_regs_t *gp_regs = get_gpregs_ctx(ctx);

	assert(ctx);
	assert(gp_regs);

	/* Turn off wake_mask */
	write_ctx_reg(gp_regs, CTX_GPREG_X4, 0);
	write_ctx_reg(gp_regs, CTX_GPREG_X5, 0);
	write_ctx_reg(gp_regs, CTX_GPREG_X6, 1);
	mce_command_handler(MCE_CMD_UPDATE_CSTATE_INFO, TEGRA_ARI_CLUSTER_CC7,
		0, TEGRA_ARI_SYSTEM_SC7);

	/* Turn off CPU */
	return mce_command_handler(MCE_CMD_ENTER_CSTATE, TEGRA_ARI_CORE_C7,
			MCE_CORE_SLEEP_TIME_INFINITE, 0);
}

__dead2 void tegra_soc_prepare_system_off(void)
{
	mce_enter_ccplex_state(TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_POWER_OFF);
}

int tegra_soc_prepare_system_reset(void)
{
	mce_enter_ccplex_state(TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_REBOOT);

	return PSCI_E_SUCCESS;
}
