/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <debug.h>
#include <sgi_plat_config.h>

/*
 * For SGI575 which support FCM (with automatic interconnect enter/exit),
 * we should not do anything in these interface functions.
 * They are used to override the weak functions in cci drivers.
 */

/******************************************************************************
 * Helper function to initialize ARM interconnect driver.
 *****************************************************************************/
void plat_arm_interconnect_init(void)
{
}

/******************************************************************************
 * Helper function to place current master into coherency
 *****************************************************************************/
void plat_arm_interconnect_enter_coherency(void)
{
}

/******************************************************************************
 * Helper function to remove current master from coherency
 *****************************************************************************/
void plat_arm_interconnect_exit_coherency(void)
{
}
