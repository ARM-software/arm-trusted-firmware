/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include "spmd_private.h"

struct spmd_pm_secondary_ep_t {
	uintptr_t entry_point;
	uintptr_t context;
	bool locked;
};

static struct spmd_pm_secondary_ep_t spmd_pm_secondary_ep[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * spmd_pm_secondary_core_set_ep
 ******************************************************************************/
int32_t spmd_pm_secondary_core_set_ep(uint64_t mpidr, uintptr_t entry_point,
				      uint64_t context)
{
	int id = plat_core_pos_by_mpidr(mpidr);

	if ((id < 0) || (id >= PLATFORM_CORE_COUNT)) {
		ERROR("%s inconsistent MPIDR (%llx)\n", __func__, mpidr);
		return -EINVAL;
	}

	if (spmd_pm_secondary_ep[id].locked) {
		ERROR("%s entry locked (%llx)\n", __func__, mpidr);
		return -EINVAL;
	}

	/*
	 * Check entry_point address is a PA within
	 * load_address <= entry_point < load_address + binary_size
	 */
	if (!spmd_check_address_in_binary_image(entry_point)) {
		ERROR("%s entry point is not within image boundaries (%llx)\n",
		      __func__, mpidr);
		return -EINVAL;
	}

	/* Fill new entry to corresponding secondary core id and lock it */
	spmd_pm_secondary_ep[id].entry_point = entry_point;
	spmd_pm_secondary_ep[id].context = context;
	spmd_pm_secondary_ep[id].locked = true;

	VERBOSE("%s %d %llx %lx %llx\n",
		__func__, id, mpidr, entry_point, context);

	return 0;
}

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
