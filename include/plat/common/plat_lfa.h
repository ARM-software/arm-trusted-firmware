/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_LFA_H
#define PLAT_LFA_H

#include <services/lfa_component_desc.h>
#include <tools_share/uuid.h>

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
int plat_lfa_notify_activate(uint32_t lfa_component_id);

#endif /* PLAT_LFA_H */
