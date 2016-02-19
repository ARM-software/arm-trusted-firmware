/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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
#include <cassert.h>
#include <plat_arm.h>
#include <platform_def.h>
#include "drivers/pwrc/fvp_pwrc.h"

/* The FVP power domain tree descriptor */
unsigned char fvp_power_domain_tree_desc[FVP_CLUSTER_COUNT + 1];


CASSERT(FVP_CLUSTER_COUNT && FVP_CLUSTER_COUNT <= 256, assert_invalid_fvp_cluster_count);

/*******************************************************************************
 * This function dynamically constructs the topology according to
 * FVP_CLUSTER_COUNT and returns it.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	int i;

	/*
	 * The FVP power domain tree does not have a single system level power domain
	 * i.e. a single root node. The first entry in the power domain descriptor
	 * specifies the number of power domains at the highest power level. For the FVP
	 * this is the number of cluster power domains.
	 */
	fvp_power_domain_tree_desc[0] = FVP_CLUSTER_COUNT;

	for (i = 0; i < FVP_CLUSTER_COUNT; i++)
		fvp_power_domain_tree_desc[i + 1] = FVP_MAX_CPUS_PER_CLUSTER;

	return fvp_power_domain_tree_desc;
}

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	return FVP_MAX_CPUS_PER_CLUSTER;
}

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	if (arm_check_mpidr(mpidr) == -1)
		return -1;

	if (fvp_pwrc_read_psysr(mpidr) == PSYSR_INVALID)
		return -1;

	return plat_arm_calc_core_pos(mpidr);
}
