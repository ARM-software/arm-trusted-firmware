/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <bl31/bl31.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <lib/bl_aux_params/bl_aux_params.h>
#include <lib/coreboot.h>
#include <lib/spinlock.h>

#include <platform.h>
#include <qti_interrupt_svc.h>
#include <qti_plat.h>
#include <qti_uart_console.h>
#include <qtiseclib_interface.h>

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl33_image_ep_info;

/*
 * Variable to hold counter frequency for the CPU's generic timer. In this
 * platform coreboot image configure counter frequency for boot core before
 * reaching TF-A.
 */
static uint64_t g_qti_cpu_cntfrq;

/*
 * Variable to hold bl31 cold boot status. Default value 0x0 means yet to boot.
 * Any other value means cold booted.
 */
uint32_t g_qti_bl31_cold_booted;

/*******************************************************************************
 * Perform any BL31 early platform setup common to ARM standard platforms.
 * Here is an opportunity to copy parameters passed by the calling EL (S-EL1
 * in BL2 & S-EL3 in BL1) before they are lost (potentially). This needs to be
 * done before the MMU is initialized so that the memory layout can be used
 * while creating page tables. BL2 has flushed this information to memory, so
 * we are guaranteed to pick up good data.
 ******************************************************************************/
void bl31_early_platform_setup(u_register_t from_bl2,
			       u_register_t plat_params_from_bl2)
{

	g_qti_cpu_cntfrq = read_cntfrq_el0();

	bl_aux_params_parse(plat_params_from_bl2, NULL);

#if COREBOOT
	if (coreboot_serial.baseaddr != 0) {
		static console_t g_qti_console_uart;

		qti_console_uart_register(&g_qti_console_uart,
					  coreboot_serial.baseaddr);
	}
#endif

	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl31_params_parse_helper(from_bl2, NULL, &bl33_image_ep_info);
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	bl31_early_platform_setup(arg0, arg1);
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	qti_setup_page_tables(
			      BL31_START,
			      BL31_END-BL31_START,
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END
			     );
	enable_mmu_el3(0);
}

/*******************************************************************************
 * Perform any BL31 platform setup common to ARM standard platforms
 ******************************************************************************/
void bl31_platform_setup(void)
{
	generic_delay_timer_init();
	/* Initialize the GIC driver, CPU and distributor interfaces */
	plat_qti_gic_driver_init();
	plat_qti_gic_init();
	qti_interrupt_svc_init();
	qtiseclib_bl31_platform_setup();

	/* set boot state to cold boot complete. */
	g_qti_bl31_cold_booted = 0x1;
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	/* QTI platform don't have BL32 implementation. */
	assert(type == NON_SECURE);
	assert(bl33_image_ep_info.h.type == PARAM_EP);
	assert(bl33_image_ep_info.h.attr == NON_SECURE);
	/*
	 * None of the images on the platforms can have 0x0
	 * as the entrypoint.
	 */
	if (bl33_image_ep_info.pc) {
		return &bl33_image_ep_info;
	} else {
		return NULL;
	}
}

/*******************************************************************************
 * This function is used by the architecture setup code to retrieve the counter
 * frequency for the CPU's generic timer. This value will be programmed into the
 * CNTFRQ_EL0 register. In Arm standard platforms, it returns the base frequency
 * of the system counter, which is retrieved from the first entry in the
 * frequency modes table. This will be used later in warm boot (psci_arch_setup)
 * of CPUs to set when CPU frequency.
 ******************************************************************************/
unsigned int plat_get_syscnt_freq2(void)
{
	assert(g_qti_cpu_cntfrq != 0);
	return g_qti_cpu_cntfrq;
}
