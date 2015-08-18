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
#include <platform_def.h>
#include <psci.h>

unsigned int plat_get_aff_count(unsigned int aff_lvl, unsigned long mpidr)
{
	/* Report 1 (absent) instance at levels higher that the cluster level */
	if (aff_lvl > MPIDR_AFFLVL1)
		return PLATFORM_SYSTEM_COUNT;

	if (aff_lvl == MPIDR_AFFLVL1)
		return PLATFORM_CLUSTER_COUNT;

	return mpidr & 0x100 ? PLATFORM_CLUSTER1_CORE_COUNT :
			       PLATFORM_CLUSTER0_CORE_COUNT;
}

unsigned int plat_get_aff_state(unsigned int aff_lvl, unsigned long mpidr)
{
	return aff_lvl <= MPIDR_AFFLVL2 ? PSCI_AFF_PRESENT : PSCI_AFF_ABSENT;
}

int mt_setup_topology(void)
{
	/* [TODO] Make topology configurable via SCC */
	return 0;
}
