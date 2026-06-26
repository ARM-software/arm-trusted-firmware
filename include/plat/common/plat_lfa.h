/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_LFA_H
#define PLAT_LFA_H

#include <services/lfa_component_desc.h>
#include <tools_share/uuid.h>

/*
 * This data structure contains component info retrieved from the platform-
 * implemented API plat_lfa_get_components.
 *
 * lfa_component_id    Component described by the data structure.
 * uuid                UUID of this component.
 * activator           Activator data structure used by this component.
 * activation_pending  Flag indicating whether this component can be activated.
 */
typedef struct plat_lfa_component_info {
	const uint32_t lfa_component_id;
	const uuid_t uuid;
	struct lfa_component_ops *activator;
	bool activation_pending;
} plat_lfa_component_info_t;

uint32_t plat_lfa_get_components(plat_lfa_component_info_t **components);
bool is_plat_lfa_activation_pending(uint32_t lfa_component_id);
int plat_lfa_cancel(uint32_t lfa_component_id);
int plat_lfa_load_auth_image(uint32_t lfa_component_id);
int plat_lfa_get_image_info(uint32_t lfa_component_id, uintptr_t *image_address,
			    size_t *image_size);
int plat_lfa_notify_activate(uint32_t lfa_component_id);
uint64_t plat_lfa_mailbox_base(void);

#endif /* PLAT_LFA_H */
