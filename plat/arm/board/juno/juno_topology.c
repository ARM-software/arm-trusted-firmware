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

#include <arm_def.h>
#include <plat_arm.h>
#include "juno_def.h"

/*
 * On Juno, the system power level is the highest power level.
 * The first entry in the power domain descriptor specifies the
 * number of system power domains i.e. 1.
 */
#define JUNO_PWR_DOMAINS_AT_MAX_PWR_LVL	 ARM_SYSTEM_COUNT

/*
 * The Juno power domain tree descriptor. The cluster power domains
 * are arranged so that when the PSCI generic code creates the power
 * domain tree, the indices of the CPU power domain nodes it allocates
 * match the linear indices returned by plat_core_pos_by_mpidr()
 * i.e. CLUSTER1 CPUs are allocated indices from 0 to 3 and the higher
 * indices for CLUSTER0 CPUs.
 */
const unsigned char juno_power_domain_tree_desc[] = {
	/* No of root nodes */
	JUNO_PWR_DOMAINS_AT_MAX_PWR_LVL,
	/* No of children for the root node */
	JUNO_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	JUNO_CLUSTER1_CORE_COUNT,
	/* No of children for the second cluster node */
	JUNO_CLUSTER0_CORE_COUNT
};

/*******************************************************************************
 * This function returns the Juno topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return juno_power_domain_tree_desc;
}

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	return (((mpidr) & 0x100) ? JUNO_CLUSTER1_CORE_COUNT :\
				JUNO_CLUSTER0_CORE_COUNT);
}
