/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* SDEI configuration for ARM platforms */

#include <platform_def.h>

#include <bl31/ehf.h>
#include <services/sdei.h>

/* Private event mappings */
static sdei_ev_map_t arm_sdei_private[] = {
	PLAT_ARM_PRIVATE_SDEI_EVENTS
};

/* Shared event mappings */
static sdei_ev_map_t arm_sdei_shared[] = {
	PLAT_ARM_SHARED_SDEI_EVENTS
};

/* Export ARM SDEI events */
REGISTER_SDEI_MAP(arm_sdei_private, arm_sdei_shared);
