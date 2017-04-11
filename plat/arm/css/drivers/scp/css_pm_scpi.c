/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <assert.h>
#include <css_pm.h>
#include <debug.h>
#include <plat_arm.h>
#include "../scpi/css_scpi.h"
#include "css_scp.h"

/*
 * This file implements the SCP power management functions using SCPI protocol.
 */

/*
 * Helper function to inform power down state to SCP.
 */
void css_scp_suspend(const psci_power_state_t *target_state)
{
	uint32_t cluster_state = scpi_power_on;
	uint32_t system_state = scpi_power_on;

	/* Check if power down at system power domain level is requested */
	if (CSS_SYSTEM_PWR_STATE(target_state) == ARM_LOCAL_STATE_OFF)
		system_state = scpi_power_retention;

	/* Cluster is to be turned off, so disable coherency */
	if (CSS_CLUSTER_PWR_STATE(target_state) == ARM_LOCAL_STATE_OFF)
		cluster_state = scpi_power_off;

	/*
	 * Ask the SCP to power down the appropriate components depending upon
	 * their state.
	 */
	scpi_set_css_power_state(read_mpidr_el1(),
				 scpi_power_off,
				 cluster_state,
				 system_state);
}

/*
 * Helper function to turn off a CPU power domain and its parent power domains
 * if applicable. Since SCPI doesn't differentiate between OFF and suspend, we
 * call the suspend helper here.
 */
void css_scp_off(const psci_power_state_t *target_state)
{
	css_scp_suspend(target_state);
}

/*
 * Helper function to turn ON a CPU power domain and its parent power domains
 * if applicable.
 */
void css_scp_on(u_register_t mpidr)
{
	/*
	 * SCP takes care of powering up parent power domains so we
	 * only need to care about level 0
	 */
	scpi_set_css_power_state(mpidr, scpi_power_on, scpi_power_on,
				 scpi_power_on);
}

/*
 * Helper function to get the power state of a power domain node as reported
 * by the SCP.
 */
int css_scp_get_power_state(u_register_t mpidr, unsigned int power_level)
{
	int rc, element;
	unsigned int cpu_state, cluster_state;

	/*
	 * The format of 'power_level' is implementation-defined, but 0 must
	 * mean a CPU. We also allow 1 to denote the cluster
	 */
	if (power_level != ARM_PWR_LVL0 && power_level != ARM_PWR_LVL1)
		return PSCI_E_INVALID_PARAMS;

	/* Query SCP */
	rc = scpi_get_css_power_state(mpidr, &cpu_state, &cluster_state);
	if (rc != 0)
		return PSCI_E_INVALID_PARAMS;

	/* Map power states of CPU and cluster to expected PSCI return codes */
	if (power_level == ARM_PWR_LVL0) {
		/*
		 * The CPU state returned by SCP is an 8-bit bit mask
		 * corresponding to each CPU in the cluster
		 */
		element = mpidr & MPIDR_AFFLVL_MASK;
		return CSS_CPU_PWR_STATE(cpu_state, element) ==
			CSS_CPU_PWR_STATE_ON ? HW_ON : HW_OFF;
	} else {
		assert(cluster_state == CSS_CLUSTER_PWR_STATE_ON ||
				cluster_state == CSS_CLUSTER_PWR_STATE_OFF);
		return cluster_state == CSS_CLUSTER_PWR_STATE_ON ? HW_ON :
			HW_OFF;
	}
}

/*
 * Helper function to shutdown the system via SCPI.
 */
void __dead2 css_scp_sys_shutdown(void)
{
	uint32_t response;

	/*
	 * Disable GIC CPU interface to prevent pending interrupt
	 * from waking up the AP from WFI.
	 */
	plat_arm_gic_cpuif_disable();

	/* Send the power down request to the SCP */
	response = scpi_sys_power_state(scpi_system_shutdown);

	if (response != SCP_OK) {
		ERROR("CSS System Off: SCP error %u.\n", response);
		panic();
	}
	wfi();
	ERROR("CSS System Off: operation not handled.\n");
	panic();
}

/*
 * Helper function to reset the system via SCPI.
 */
void __dead2 css_scp_sys_reboot(void)
{
	uint32_t response;

	/*
	 * Disable GIC CPU interface to prevent pending interrupt
	 * from waking up the AP from WFI.
	 */
	plat_arm_gic_cpuif_disable();

	/* Send the system reset request to the SCP */
	response = scpi_sys_power_state(scpi_system_reboot);

	if (response != SCP_OK) {
		ERROR("CSS System Reset: SCP error %u.\n", response);
		panic();
	}
	wfi();
	ERROR("CSS System Reset: operation not handled.\n");
	panic();
}
