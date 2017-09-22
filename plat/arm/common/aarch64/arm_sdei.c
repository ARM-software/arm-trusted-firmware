/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* SDEI configuration for ARM platforms */

#include <ehf.h>
#include <platform_def.h>
#include <sdei.h>

/* Private event mappings */
static sdei_ev_map_t arm_private_sdei[] = {
	/* Event 0 */
	SDEI_DEFINE_EVENT_0(ARM_SDEI_SGI),

	/* Dynamic private events */
	SDEI_PRIVATE_EVENT(ARM_SDEI_DP_EVENT_0, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),
	SDEI_PRIVATE_EVENT(ARM_SDEI_DP_EVENT_1, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),
	SDEI_PRIVATE_EVENT(ARM_SDEI_DP_EVENT_2, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),
};

/* Shared event mappings */
static sdei_ev_map_t arm_shared_sdei[] = {
	/* Dynamic shared events */
	SDEI_SHARED_EVENT(ARM_SDEI_DS_EVENT_0, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),
	SDEI_SHARED_EVENT(ARM_SDEI_DS_EVENT_1, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),
	SDEI_SHARED_EVENT(ARM_SDEI_DS_EVENT_2, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),
};

/* Export ARM SDEI events */
REGISTER_SDEI_MAP(arm_private_sdei, arm_shared_sdei);
