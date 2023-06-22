/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* SDEI configuration for ARM platforms */

#include <bl31/ehf.h>
#include <common/debug.h>
#include <lib/utils_def.h>
#include <services/sdei.h>
#include <platform_def.h>

/* Private event mappings */
static sdei_ev_map_t qemu_sdei_private[] = {
	SDEI_DEFINE_EVENT_0(PLAT_SDEI_SGI_PRIVATE),
};

/* Shared event mappings */
static sdei_ev_map_t qemu_sdei_shared[] = {
};

void plat_sdei_setup(void)
{
	INFO("SDEI platform setup\n");
}

/* Export Arm SDEI events */
REGISTER_SDEI_MAP(qemu_sdei_private, qemu_sdei_shared);
