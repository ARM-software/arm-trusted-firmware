/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/spinlock.h>

#include "fpga_private.h"
#include <plat/common/platform.h>
#include <platform_def.h>

unsigned char fpga_power_domain_tree_desc[FPGA_MAX_CLUSTER_COUNT + 2];
unsigned char fpga_valid_mpids[PLATFORM_CORE_COUNT];

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	unsigned int i;

	/*
	* The highest level is the system level. The next level is constituted
	* by clusters and then cores in clusters.
	*
	* This description of the power domain topology is aligned with the CPU
	* indices returned by the plat_core_pos_by_mpidr() and plat_my_core_pos()
	* APIs.
	*
	* A description of the topology tree can be found at
	* https://trustedfirmware-a.readthedocs.io/en/latest/design/psci-pd-tree.html#design
	*/

	if (fpga_power_domain_tree_desc[0] == 0U) {
		/*
		 * As fpga_power_domain_tree_desc[0] == 0, assume that the
		 * Power Domain Topology Tree has not been initialized, so
		 * perform the initialization here.
		 */

		fpga_power_domain_tree_desc[0] = 1U;
		fpga_power_domain_tree_desc[1] = FPGA_MAX_CLUSTER_COUNT;

		for (i = 0U; i < FPGA_MAX_CLUSTER_COUNT; i++) {
			fpga_power_domain_tree_desc[2 + i] =
				(FPGA_MAX_CPUS_PER_CLUSTER *
				 FPGA_MAX_PE_PER_CPU);
		}
	}

	return fpga_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int core_pos;

	mpidr &= (MPID_MASK & ~(MPIDR_AFFLVL_MASK << MPIDR_AFF3_SHIFT));
	mpidr |= (read_mpidr_el1() & MPIDR_MT_MASK);

	if ((MPIDR_AFFLVL2_VAL(mpidr) >= FPGA_MAX_CLUSTER_COUNT) ||
	    (MPIDR_AFFLVL1_VAL(mpidr) >= FPGA_MAX_CPUS_PER_CLUSTER) ||
	    (MPIDR_AFFLVL0_VAL(mpidr) >= FPGA_MAX_PE_PER_CPU)) {
		ERROR ("Invalid mpidr: 0x%08x\n", (uint32_t)mpidr);
		panic();
	}

	/* Calculate the core position, based on the maximum topology. */
	core_pos = plat_fpga_calc_core_pos(mpidr);

	/* Check whether this core is actually present. */
	if (fpga_valid_mpids[core_pos] != VALID_MPID) {
		return -1;
	}

	return core_pos;
}
