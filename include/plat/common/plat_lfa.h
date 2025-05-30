/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_LFA_H
#define PLAT_LFA_H

#include <tools_share/uuid.h>

typedef struct plat_lfa_component_info {
	const uint32_t lfa_component_id;
	const uuid_t uuid;
} plat_lfa_component_info_t;

uint32_t plat_lfa_get_components(plat_lfa_component_info_t **components);

#endif /* PLAT_LFA_H */
