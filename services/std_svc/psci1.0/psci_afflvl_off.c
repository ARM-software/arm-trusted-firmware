/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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
#include <debug.h>
#include <string.h>
#include "psci_private.h"

/******************************************************************************
 * Top level handler which is called when a cpu wants to power itself down.
 * It's assumed that along with turning the cpu off, higher affinity levels
 * will be turned off as far as possible. It finds the highest level to be
 * powered off by traversing the node information and then performs generic,
 * architectural, platform setup and state management required to turn OFF
 * that affinity level and affinity levels below it. e.g. For a cpu that's to
 * be powered OFF, it could mean programming the power controller whereas for
 * a cluster that's to be powered off, it will call the platform specific code
 * which will disable coherency at the interconnect level if the cpu is the
 * last in the cluster and also the program the power controller.
 ******************************************************************************/
int psci_afflvl_off(int end_afflvl)
{
	int rc;
	mpidr_aff_map_nodes_t mpidr_nodes;
	unsigned int max_phys_off_afflvl;

	/*
	 * This function must only be called on platforms where the
	 * CPU_OFF platform hooks have been implemented.
	 */
	assert(psci_plat_pm_ops->affinst_off);

	/*
	 * Collect the pointers to the nodes in the topology tree for
	 * each affinity instance in the mpidr. If this function does
	 * not return successfully then either the mpidr or the affinity
	 * levels are incorrect. Either way, this an internal TF error
	 * therefore assert.
	 */
	rc = psci_get_aff_map_nodes(read_mpidr_el1() & MPIDR_AFFINITY_MASK,
				    MPIDR_AFFLVL0,
				    end_afflvl,
				    mpidr_nodes);
	assert(rc == PSCI_E_SUCCESS);

	/*
	 * This function acquires the lock corresponding to each affinity
	 * level so that by the time all locks are taken, the system topology
	 * is snapshot and state management can be done safely.
	 */
	psci_acquire_afflvl_locks(MPIDR_AFFLVL0,
				  end_afflvl,
				  mpidr_nodes);


	/*
	 * Call the cpu off handler registered by the Secure Payload Dispatcher
	 * to let it do any bookkeeping. Assume that the SPD always reports an
	 * E_DENIED error if SP refuse to power down
	 */
	if (psci_spd_pm && psci_spd_pm->svc_off) {
		rc = psci_spd_pm->svc_off(0);
		if (rc)
			goto exit;
	}

	/*
	 * This function updates the state of each affinity instance
	 * corresponding to the mpidr in the range of affinity levels
	 * specified.
	 */
	psci_do_afflvl_state_mgmt(MPIDR_AFFLVL0,
				  end_afflvl,
				  mpidr_nodes,
				  PSCI_STATE_OFF);

	max_phys_off_afflvl = psci_find_max_phys_off_afflvl(MPIDR_AFFLVL0,
							   end_afflvl,
							   mpidr_nodes);
	assert(max_phys_off_afflvl != PSCI_INVALID_DATA);

	/*
	 * Arch. management. Perform the necessary steps to flush all
	 * cpu caches.
	 */
	psci_do_pwrdown_cache_maintenance(max_phys_off_afflvl);

	/*
	 * Plat. management: Perform platform specific actions to turn this
	 * cpu off e.g. exit cpu coherency, program the power controller etc.
	 */
	psci_plat_pm_ops->affinst_off(max_phys_off_afflvl);

exit:
	/*
	 * Release the locks corresponding to each affinity level in the
	 * reverse order to which they were acquired.
	 */
	psci_release_afflvl_locks(MPIDR_AFFLVL0,
				  end_afflvl,
				  mpidr_nodes);

	/*
	 * Check if all actions needed to safely power down this cpu have
	 * successfully completed. Enter a wfi loop which will allow the
	 * power controller to physically power down this cpu.
	 */
	if (rc == PSCI_E_SUCCESS)
		psci_power_down_wfi();

	return rc;
}
