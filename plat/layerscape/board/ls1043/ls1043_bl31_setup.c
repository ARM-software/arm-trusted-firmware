/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/cci.h>

#include "plat_ls.h"
#include "fsl_csu.h"

/* slave interfaces according to the RM */
static const int cci_map[] = {
	4,
};

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
#ifdef LS_BL2_IN_OCRAM
	unsigned long romem_base = (unsigned long)(&__TEXT_START__);
	unsigned long romem_size = (unsigned long)(&__RODATA_END__)
					- romem_base;

	/* Check the Text and RO-Data region size */
	if (romem_size > BL31_TEXT_RODATA_SIZE) {
		ERROR("BL31 Text and RO-Data region size exceed reserved memory size\n");
		panic();
	}
#endif

	/*
	 * Initialize system level generic timer for Layerscape Socs.
	 */
	ls_delay_timer_init();

	ls_bl31_early_platform_setup((void *)arg0, (void *)arg3);

	/*
	 * Initialize the correct interconnect for this cluster during cold
	 * boot. No need for locks as no other CPU is active.
	 */
	cci_init(PLAT_LS1043_CCI_BASE, cci_map, ARRAY_SIZE(cci_map));

	/*
	 * Enable coherency in interconnect for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this (e.g. Trusted
	 * Firmware's BL1 does it) but we can't assume so. There is no harm in
	 * executing this code twice anyway.
	 */
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr()));

	/* Init CSU to enable non-secure access to peripherals */
	enable_layerscape_ns_access();
}
