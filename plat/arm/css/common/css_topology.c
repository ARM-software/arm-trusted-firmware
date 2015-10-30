/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#include <plat_arm.h>

/*
 * On ARM CSS platforms, by default, the system power level is treated as the
 * highest. The first entry in the power domain descriptor specifies the
 * number of system power domains i.e. 1.
 */
#define CSS_PWR_DOMAINS_AT_MAX_PWR_LVL	ARM_SYSTEM_COUNT

/*
 * The CSS power domain tree descriptor for dual cluster CSS platforms.
 * The cluster power domains are arranged so that when the PSCI generic
 * code creates the power domain tree, the indices of the CPU power
 * domain nodes it allocates match the linear indices returned by
 * plat_core_pos_by_mpidr() i.e. CLUSTER1 CPUs are allocated indices
 * from 0 to 3 and the higher indices for CLUSTER0 CPUs.
 */
const unsigned char arm_power_domain_tree_desc[] = {
	/* No of root nodes */
	CSS_PWR_DOMAINS_AT_MAX_PWR_LVL,
	/* No of children for the root node */
	ARM_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	PLAT_ARM_CLUSTER1_CORE_COUNT,
	/* No of children for the second cluster node */
	PLAT_ARM_CLUSTER0_CORE_COUNT
};


/******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is
 * returned in case the MPIDR is invalid.
 *****************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	if (arm_check_mpidr(mpidr) == 0)
		return plat_arm_calc_core_pos(mpidr);

	return -1;
}
