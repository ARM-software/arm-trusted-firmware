/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/css/common/css_pm.h>

#include "n1sdp_private.h"

/*******************************************************************************
 * N1SDP specific function called when turning off a power domain. Additionally
 * disables the GIC redistributor interface as cores are disabled to
 * let cluster-PPU state transition to completion when a cluster is powered
 * down.
 ******************************************************************************/
void n1sdp_pwr_domain_off(const psci_power_state_t *target_state)
{
	css_pwr_domain_off(target_state);
	plat_arm_gic_redistif_off();
}
