/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <services/lfa_svc.h>
#include <services/rmmd_rmm_lfa.h>

static int32_t lfa_rmm_prime(struct lfa_component_status *activation)
{
	return LFA_WRONG_STATE;
}

static int32_t lfa_rmm_activate(struct lfa_component_status *activation,
				uint64_t ep_address, uint64_t context_id)
{
	return LFA_WRONG_STATE;
}

static struct lfa_component_ops rmm_activator = {
	.prime = lfa_rmm_prime,
	.activate = lfa_rmm_activate,
	.may_reset_cpu = false,
	.cpu_rendezvous_required = true,
};

struct lfa_component_ops *get_rmm_activator(void)
{
	return &rmm_activator;
}
