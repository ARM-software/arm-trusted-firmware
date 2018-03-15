/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <plat_ls.h>
#include <tbbr_img_def.h>
#include <cci.h>
#include <debug.h>
#include <mmio.h>
#include <arch_helpers.h>
#include <delay_timer.h>
#include <generic_delay_timer.h>

static const int cci_map[] = {
	PLAT_LS1043_CCI_CLUSTER0_SL_IFACE_IX
};

void bl1_platform_setup(void)
{
	NOTICE(FIRMWARE_WELCOME_STR_LS1043);

	ls_bl1_platform_setup();

	/*
	 * Initialize system level generic timer for Layerscape Socs.
	 */
	ls_delay_timer_init();

	/* TODO: remove these DDR code */
	VERBOSE("CS0_BNDS = %llx\n", mmio_read_32(0x1080000 + 0x000));
	mmio_write_32(0x1080000 + 0x000, 0x7f000000);
	VERBOSE("CS0_BNDS = %llx\n", mmio_read_32(0x1080000 + 0x000));
#ifdef	SD_BOOT
	INFO("loading.... fip from sd\n");
	sd_load_fip_image();
#endif
}

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	ls_bl1_early_platform_setup();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	cci_init(PLAT_LS1043_CCI_BASE, cci_map, ARRAY_SIZE(cci_map));

	/*
	 * Enable coherency in Interconnect for the primary CPU's cluster.
	 */
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr()));

}

unsigned int bl1_plat_get_next_image_id(void)
{
	return BL2_IMAGE_ID;
}
