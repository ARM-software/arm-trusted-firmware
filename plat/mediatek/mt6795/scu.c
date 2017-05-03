/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <mcucfg.h>
#include <mmio.h>

void disable_scu(unsigned long mpidr)
{
	if (mpidr & MPIDR_CLUSTER_MASK)
		mmio_setbits_32((uintptr_t)&mt6795_mcucfg->mp1_miscdbg,
			MP1_ACINACTM);
	else
		mmio_setbits_32((uintptr_t)&mt6795_mcucfg->mp0_axi_config,
			MP0_ACINACTM);
}

void enable_scu(unsigned long mpidr)
{
	if (mpidr & MPIDR_CLUSTER_MASK)
		mmio_clrbits_32((uintptr_t)&mt6795_mcucfg->mp1_miscdbg,
			MP1_ACINACTM);
	else
		mmio_clrbits_32((uintptr_t)&mt6795_mcucfg->mp0_axi_config,
			MP0_ACINACTM);
}
