/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <device_wrapper.h>
#include <drivers/scmi-msg.h>
#include <lpm_stub.h>
#include <plat_private.h>
#include <plat_scmi_def.h>
#include <rtc.h>
#include <ti_sci.h>
#include <ti_sci_transport.h>

/* Table of regions to map using the MMU */
/* TODO: Add AM62L specific mapping such that K3 devices don't break */
const mmap_region_t plat_k3_mmap[] = {
	MAP_REGION_FLAT(0x0, 0x80000000, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(K3_FUSE_WRITEBUFF_BASE, K3_FUSE_WRITEBUFF_SIZE, MT_MEMORY | MT_RW | MT_NS),
#ifdef K3_AM62L_LPM
	MAP_REGION_FLAT(DEVICE_WKUP_SRAM_BASE, DEVICE_WKUP_SRAM_SIZE, MT_MEMORY | MT_RW | MT_SECURE),
#endif
	{ /* sentinel */ }
};

/*
 * HACK: ADC clock can not be controlled by linux due to known bug,
 * where 0_ADC0 is being registered as it's own parent, which makes it
 * unable to reparent. Force the clock parent from here till proper
 * fix is implemented in linux
 */
static void ti_force_adc_parent(void)
{
	INFO("0_ADC0's parent is %d\n", plat_scmi_clock_get_parent(0, 0));
	if (!plat_scmi_clock_set_parent(0, 0, 2)) {
		INFO("0_ADC0's parent (after set_parent) is %d\n",
		       plat_scmi_clock_get_parent(0, 0));
	} else {
		WARN("ADC set_parent failed!\n");
	}
}

int ti_soc_init(void)
{
	struct ti_sci_msg_version version;
	int ret;

	generic_delay_timer_init();
	ti_init_scmi_server();
#ifdef K3_AM62L_LPM
	if (k3_lpm_stub_copy_to_sram()) {
		WARN("A53 stub copy failed!\n");
	} else {
		INFO("A53 stub copy passed\n");
	}
	rtc_init();
#endif
	ret = ti_sci_get_revision(&version);
	if (ret) {
		ERROR("Unable to communicate with the control firmware (%d)\n", ret);
		return ret;
	}

	INFO("SYSFW ABI: %d.%d (firmware rev 0x%04x '%s')\n",
	     version.abi_major, version.abi_minor,
	     version.firmware_revision,
	     version.firmware_description);

	ti_force_adc_parent();

	return 0;
}
