/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LFA_COMPONENT_DESC_H
#define LFA_COMPONENT_DESC_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	PRIME_NONE = 0,
	PRIME_STARTED,
	PRIME_COMPLETE,
} lfa_prime_status_t;

struct lfa_component_status {
	uint32_t component_id;
	lfa_prime_status_t prime_status;
	bool cpu_rendezvous_required;
};

typedef int (*component_prime_fn)(struct lfa_component_status *activation);
typedef int (*component_activate_fn)(struct lfa_component_status *activation,
					 uint64_t ep_address,
					 uint64_t context_id);

struct lfa_component_ops {
	component_prime_fn prime;
	component_activate_fn activate;
	bool may_reset_cpu;
	bool cpu_rendezvous_required;
};

#endif /* LFA_COMPONENT_DESC_H */
