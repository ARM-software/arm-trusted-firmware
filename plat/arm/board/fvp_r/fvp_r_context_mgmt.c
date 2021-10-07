/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>

/************************************************************
 * For R-class everything is in secure world.
 * Prepare the CPU system registers for first entry into EL1
 ************************************************************/
void cm_prepare_el2_exit(void)
{
	uint64_t hcr_el2 = 0U;

	/*
	 * The use of ARMv8.3 pointer authentication (PAuth) is governed
	 * by fields in HCR_EL2, which trigger a 'trap to EL2' if not
	 * enabled. This register initialized at boot up, update PAuth
	 * bits.
	 *
	 * HCR_API_BIT: Set to one to disable traps to EL2 if lower ELs
	 * access PAuth registers
	 *
	 * HCR_APK_BIT: Set to one to disable traps to EL2 if lower ELs
	 * access PAuth instructions
	 */
	hcr_el2 = read_hcr_el2();
	write_hcr_el2(hcr_el2 | HCR_API_BIT | HCR_APK_BIT);

	/*
	 * Initialise CNTHCTL_EL2. All fields are architecturally UNKNOWN
	 * on reset and are set to zero except for field(s) listed below.
	 *
	 * CNTHCTL_EL2.EL1PCEN: Set to one to disable traps to EL2
	 * if lower ELs accesses to the physical timer registers.
	 *
	 * CNTHCTL_EL2.EL1PCTEN: Set to one to disable traps to EL2
	 * if lower ELs access to the physical counter registers.
	 */
	write_cnthctl_el2(CNTHCTL_RESET_VAL | EL1PCEN_BIT | EL1PCTEN_BIT);

	/*
	 * On Armv8-R, the EL1&0 memory system architecture is configurable
	 * as a VMSA or PMSA. All the fields architecturally UNKNOWN on reset
	* and are set to zero except for field listed below.
	*
	* VCTR_EL2.MSA: Set to one to ensure the VMSA is enabled so that
	* rich OS can boot.
	*/
	write_vtcr_el2(VTCR_RESET_VAL | VTCR_EL2_MSA);
}
