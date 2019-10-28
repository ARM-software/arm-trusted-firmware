/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include "spmd_private.h"

/*******************************************************************************
 * This CPU has been turned on. Enter SPMC to initialise S-EL1 or S-EL2. As part
 * of the SPMC initialization path, they will initialize any SPs that they
 * manage. Entry into SPMC is done after initialising minimal architectural
 * state that guarantees safe execution.
 ******************************************************************************/
static void spmd_cpu_on_finish_handler(u_register_t unused)
{
	unsigned int linear_id = plat_my_core_pos();
	spmd_spm_core_context_t *ctx = spmd_get_context();
	int rc;

	assert(ctx->state != SPMC_STATE_ON);

	rc = spmd_spm_core_sync_entry(ctx);
	if (rc != 0) {
		ERROR("SPMC initialisation failed (%d) on CPU%u\n", rc,
		      linear_id);
		ctx->state = SPMC_STATE_OFF;
		return;
	}

	ctx->state = SPMC_STATE_ON;
}

/*******************************************************************************
 * Structure populated by the SPM Dispatcher to perform any bookkeeping before
 * PSCI executes a power mgmt. operation.
 ******************************************************************************/
const spd_pm_ops_t spmd_pm = {
	.svc_on_finish = spmd_cpu_on_finish_handler,
};
