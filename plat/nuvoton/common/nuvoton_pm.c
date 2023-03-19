/*
 * Copyright (c) 2015-2023, ARM Limited and Contributors. All rights reserved.
 *
 * Copyright (C) 2023 Nuvoton Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>


/* Allow npcm845x to override these functions */
#pragma weak plat_arm_program_trusted_mailbox
#pragma weak plat_setup_psci_ops /* changed to weak */


/*******************************************************************************
 * ARM standard platform handler called to check the validity of the non secure
 * entrypoint. Returns 0 if the entrypoint is valid, or -1 otherwise.
 ******************************************************************************/
int arm_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	if ((entrypoint >= ARM_NS_DRAM1_BASE) &&
		(entrypoint < (ARM_NS_DRAM1_BASE + ARM_NS_DRAM1_SIZE))) {
		return 0;
	}
#ifdef __aarch64__
	if ((entrypoint >= ARM_DRAM2_BASE) &&
		(entrypoint < (ARM_DRAM2_BASE + ARM_DRAM2_SIZE))) {
		return 0;
	}
#endif

	return -1;
}

int arm_validate_psci_entrypoint(uintptr_t entrypoint)
{
	return (arm_validate_ns_entrypoint(entrypoint) == 0) ? PSCI_E_SUCCESS :
		PSCI_E_INVALID_ADDRESS;
}

/******************************************************************************
 * Helper function to save the platform state before a system suspend. Save the
 * state of the system components which are not in the Always ON power domain.
 *****************************************************************************/
void arm_system_pwr_domain_save(void)
{
	/* Assert system power domain is available on the platform */
	assert(PLAT_MAX_PWR_LVL > ARM_PWR_LVL1);

	plat_arm_gic_save();

	/*
	 * Unregister console now so that it is not registered for a second
	 * time during resume.
	 */
	arm_console_runtime_end();

	/*
	 * All the other peripheral which are configured by TF-A are
	 * re-initialized on resume from system suspend. Hence we
	 * don't save their state here.
	 */
}
