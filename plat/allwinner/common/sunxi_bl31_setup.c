/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <assert.h>
#include <console.h>
#include <debug.h>
#include <generic_delay_timer.h>
#include <gicv2.h>
#include <platform.h>
#include <platform_def.h>
#include <sunxi_def.h>
#include <sunxi_mmap.h>
#include <uart_16550.h>

#include "sunxi_private.h"

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

static console_16550_t console;

static const gicv2_driver_data_t sunxi_gic_data = {
	.gicd_base = SUNXI_GICD_BASE,
	.gicc_base = SUNXI_GICC_BASE,
};

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Initialize the debug console as soon as possible */
	console_16550_register(SUNXI_UART0_BASE, SUNXI_UART0_CLK_IN_HZ,
			       SUNXI_UART0_BAUDRATE, &console);

#ifdef BL32_BASE
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
#endif

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info, PARAM_EP, VERSION_1, 0);
	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
					  DISABLE_ALL_EXCEPTIONS);
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	/* Turn off all secondary CPUs */
	sunxi_disable_secondary_cpus(plat_my_core_pos());
}

void bl31_plat_arch_setup(void)
{
	sunxi_configure_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	const char *soc_name;
	uint16_t soc_id = sunxi_read_soc_id();

	switch (soc_id) {
	case 0x1689:
		soc_name = "A64/H64/R18";
		break;
	case 0x1718:
		soc_name = "H5";
		break;
	case 0x1728:
		soc_name = "H6";
		break;
	default:
		soc_name = "unknown";
		break;
	}
	NOTICE("BL31: Detected Allwinner %s SoC (%04x)\n", soc_name, soc_id);

	generic_delay_timer_init();

	/* Configure the interrupt controller */
	gicv2_driver_init(&sunxi_gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();

	sunxi_security_setup();

	sunxi_pmic_setup();

	INFO("BL31: Platform setup done\n");
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	assert(sec_state_is_valid(type) != 0);

	if (type == NON_SECURE)
		return &bl33_image_ep_info;

	if ((type == SECURE) && bl32_image_ep_info.pc)
		return &bl32_image_ep_info;

	return NULL;
}
