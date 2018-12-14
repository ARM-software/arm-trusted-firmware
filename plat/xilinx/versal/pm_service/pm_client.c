/*
 * Copyright (c) 2019, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * APU specific definition of processors in the subsystem as well as functions
 * for getting information about and changing state of the APU.
 */

#include <plat_ipi.h>
#include <platform_def.h>
#include <versal_def.h>
#include <lib/bakery_lock.h>
#include "pm_client.h"

DEFINE_BAKERY_LOCK(pm_client_secure_lock);

static const struct pm_ipi apu_ipi = {
	.local_ipi_id = IPI_ID_APU,
	.remote_ipi_id = IPI_ID_PMC,
	.buffer_base = IPI_BUFFER_APU_BASE,
};

/* Order in pm_procs_all array must match cpu ids */
static const struct pm_proc pm_procs_all[] = {
	{
		.node_id = XPM_DEVID_ACPU_0,
		.ipi = &apu_ipi,
	},
	{
		.node_id = XPM_DEVID_ACPU_1,
		.ipi = &apu_ipi,
	}
};

const struct pm_proc *primary_proc = &pm_procs_all[0];
