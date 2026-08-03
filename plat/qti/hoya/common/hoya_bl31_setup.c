/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <qti_plat.h>
#include <qtiseclib_interface.h>

/*
 * Variable to hold bl31 cold boot status. Default value 0x0 means yet to boot.
 * Any other value means cold booted.
 */
uint32_t g_qti_bl31_cold_booted;

void plat_qti_bl31_setup_post(void)
{
	qtiseclib_bl31_platform_setup();

	/* set boot state to cold boot complete. */
	g_qti_bl31_cold_booted = 0x1;
}
