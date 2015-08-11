/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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
#include <plat_arm.h>
#include <platform_def.h>
#include "drivers/pwrc/fvp_pwrc.h"

/*
 * The FVP power domain tree does not have a single system level power domain
 * i.e. a single root node. The first entry in the power domain descriptor
 * specifies the number of power domains at the highest power level. For the FVP
 * this is 2 i.e. the number of cluster power domains.
 */
#define FVP_PWR_DOMAINS_AT_MAX_PWR_LVL	ARM_CLUSTER_COUNT

/* The FVP power domain tree descriptor */
const unsigned char arm_power_domain_tree_desc[] = {
	/* No of root nodes */
	FVP_PWR_DOMAINS_AT_MAX_PWR_LVL,
	/* No of children for the first node */
	PLAT_ARM_CLUSTER0_CORE_COUNT,
	/* No of children for the second node */
	PLAT_ARM_CLUSTER1_CORE_COUNT
};

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int platform_core_pos_by_mpidr(u_register_t mpidr)
{
	if (arm_check_mpidr(mpidr) == -1)
		return -1;

	if (fvp_pwrc_read_psysr(mpidr) == PSYSR_INVALID)
		return -1;

	return plat_arm_calc_core_pos(mpidr);
}
