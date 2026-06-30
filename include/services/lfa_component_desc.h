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

/*
 * This structure contains the status of a component that is currently in the
 * process of being activated.
 *
 * component_id         ID of the activation component currently in progress.
 *                      Can be either RMM or BL31.
 * prime_status         Current prime status of this component.
 * image_size           Size of the image file to be activated.
 * image_address        Address of the image file to be activated.
 * reset                Flag indicating that the core will warm reset after the
 *                      activation.
 * cpu_rendezvous	Flag indicating that active CPUs will rendezvous prior
 *                      to activation. This is set to true by default and can
 *                      only be cleared if the caller requests it and the flag
 *                      cpu_rendezvous_required in the component ops structure
 *                      is clear.
 */
struct lfa_component_status {
	uint32_t component_id;
	lfa_prime_status_t prime_status;
	size_t image_size;
	uintptr_t image_address;
	bool reset;
	bool cpu_rendezvous;
};

typedef int (*component_prime_fn)(struct lfa_component_status *activation);
typedef int (*component_activate_fn)(struct lfa_component_status *activation,
					 uint64_t ep_address,
					 uint64_t context_id);
typedef int (*component_cancel_fn)(struct lfa_component_status *activation);

/*
 * This structure is defined per-component and contains pointers to the prime
 * and activate functions as well as some basic requirements for activation of
 * that component.
 *
 * prime                   Pointer to prime function for a component.
 * activate		   Pointer to activate function for a component.
 * may_reset_cpu	   Flag letting the caller know to expect a CPU warm
 *                         reset after activation.
 * cpu_rendezvous_required Flag indicating that a CPU reset is not optional. If
 *                         this flag is set, the caller cannot request skipping
 *                         the CPU rendezvous.
 */
struct lfa_component_ops {
	component_prime_fn prime;
	component_activate_fn activate;
	component_cancel_fn cancel;
	bool may_reset_cpu;
	bool cpu_rendezvous_required;
};

#endif /* LFA_COMPONENT_DESC_H */
