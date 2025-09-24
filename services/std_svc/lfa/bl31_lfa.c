/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <services/bl31_lfa.h>
#include <services/lfa_svc.h>

static int lfa_bl31_prime(struct lfa_component_status *activation)
{
	return LFA_WRONG_STATE;
}

static int lfa_bl31_activate(struct lfa_component_status *activation,
		uint64_t ep_address,
		uint64_t context_id)
{
	return LFA_WRONG_STATE;
}

static struct lfa_component_ops bl31_activator = {
	.prime = lfa_bl31_prime,
	.activate = lfa_bl31_activate,
	.may_reset_cpu = false,
	.cpu_rendezvous_required = true,
};

struct lfa_component_ops *get_bl31_activator(void)
{
	return &bl31_activator;
}
