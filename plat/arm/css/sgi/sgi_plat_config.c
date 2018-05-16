/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <plat_arm.h>
#include <platform_def.h>
#include <sgi_variant.h>
#include <sgi_plat_config.h>
#include <string.h>

static css_plat_config_t *css_plat_info;

/* GIC */
/* The GICv3 driver only needs to be initialized in EL3 */
uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

/* Interconnect configuration for SGI575 */
const css_inteconn_config_t sgi575_inteconn = {
	.ip_type = ARM_CMN,
	.plat_inteconn_desc = NULL
};

/* Configuration structure for SGI575 */
css_plat_config_t sgi575_config = {
	.inteconn = &sgi575_inteconn,
};

/*******************************************************************************
 * This function initializes the platform sturcture.
 ******************************************************************************/
void plat_config_init(void)
{
	/* Get the platform configurations */
	switch (GET_SGI_PART_NUM) {
	case SGI575_SSC_VER_PART_NUM:
		css_plat_info = &sgi575_config;
		break;
	default:
		ERROR("Not a valid sgi variant!\n");
		panic();
	}
}

/*******************************************************************************
 * This function returns the platform structure pointer.
 ******************************************************************************/
css_plat_config_t *get_plat_config(void)
{
	assert(css_plat_info != NULL);
	return css_plat_info;
}
