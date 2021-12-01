/*
 * Copyright (c) 2021, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <msm8916_mmap.h>

static void __dead2 msm8916_system_reset(void)
{
	mmio_write_32(MPM_PS_HOLD, 0);
	mdelay(1000);

	ERROR("PSCI: System reset failed\n");
	panic();
}

static const plat_psci_ops_t msm8916_psci_ops = {
	.system_off			= msm8916_system_reset,
	.system_reset			= msm8916_system_reset,
};

/* Defined and used in msm8916_helpers.S */
extern uintptr_t msm8916_entry_point;

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	msm8916_entry_point = sec_entrypoint;
	*psci_ops = &msm8916_psci_ops;
	return 0;
}
