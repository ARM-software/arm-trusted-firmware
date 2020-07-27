/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* SDEI configuration for ARM platforms */

#include <bl31/ehf.h>
#include <common/debug.h>
#include <services/sdei.h>
#include <lib/utils_def.h>

#include <platform_def.h>

/* Private event mappings */
static sdei_ev_map_t imx_sdei_private[] = {
	SDEI_DEFINE_EVENT_0(PLAT_SDEI_SGI_PRIVATE),
};

/* Shared event mappings */
static sdei_ev_map_t imx_sdei_shared[] = {
};

void plat_sdei_setup(void)
{
	INFO("SDEI platform setup\n");
}

/* Export ARM SDEI events */
REGISTER_SDEI_MAP(imx_sdei_private, imx_sdei_shared);
