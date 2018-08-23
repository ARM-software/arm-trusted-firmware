/********************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <ep_info.h>
#include <bl31.h>
#include <console.h>
#include <coreboot.h>
#include <plat_qti.h>
#include <qti_plat_params.h>
#include <qti_interrupt_svc.h>
#include <qti_uart_console.h>
#include <qtiseclib_interface.h>

/*
 * Below constants identify the extents of the code, RO data region and the
 * limit of the BL31 image.  These addresses are used by the MMU setup code and
 * therefore they must be page-aligned.  It is the responsibility of the linker
 * script to ensure that __RO_START__, __RO_END__ & __BL31_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_CODE_BASE 				(uintptr_t)(&__TEXT_START__)
#define BL31_CODE_LIMIT 			(uintptr_t)(&__TEXT_END__)
#define BL31_RO_DATA_BASE			(uintptr_t)(&__RODATA_START__)
#define BL31_RO_DATA_LIMIT 			(uintptr_t)(&__RODATA_END__)
#define BL31_RW_BASE 				(uintptr_t)(&__DATA_START__)
#define BL31_RW_LIMIT 				(uintptr_t)(&__DATA_END__)
#define BL31_END 				(uintptr_t)(&__BL31_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_COHERENT_RAM_BASE (uintptr_t)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (uintptr_t)(&__COHERENT_RAM_END__)

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl33_image_ep_info;

/*
 * Variable to hold counter frequency for the CPU's generic timer. In this
 * platform coreboot image configure counter frequency for boot core before
 * reaching ATF.
 */
static uint64_t g_qti_cpu_cntfrq;

static void params_early_setup(void *plat_param_from_bl2)
{
	struct bl31_plat_param *bl2_param;

	/* keep plat parameters for later processing if need */
	bl2_param = (struct bl31_plat_param *)plat_param_from_bl2;
	while (bl2_param) {
		switch (bl2_param->type) {
#if COREBOOT
		case PARAM_COREBOOT_TABLE:
			coreboot_table_setup((void *)
				((struct bl31_u64_param *)bl2_param)->value);
			break;
#endif
		default:
			ERROR("not expected type found %lld\n",
			      bl2_param->type);
			break;
		}
		bl2_param = bl2_param->next;
	}
}

/*******************************************************************************
 * Perform any BL31 early platform setup common to ARM standard platforms.
 * Here is an opportunity to copy parameters passed by the calling EL (S-EL1
 * in BL2 & S-EL3 in BL1) before they are lost (potentially). This needs to be
 * done before the MMU is initialized so that the memory layout can be used
 * while creating page tables. BL2 has flushed this information to memory, so
 * we are guaranteed to pick up good data.
 ******************************************************************************/
void bl31_early_platform_setup(bl31_params_t * from_bl2,
			       void *plat_params_from_bl2)
{
	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info = *from_bl2->bl33_ep_info;
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	g_qti_cpu_cntfrq = read_cntfrq_el0();

	params_early_setup(plat_params_from_bl2);

#if COREBOOT
	if (coreboot_serial.baseaddr) {
		static qti_console_uart_t g_qti_console_uart;
		qti_console_uart_register(&g_qti_console_uart,
					  coreboot_serial.baseaddr);
	}
#endif

	qtiseclib_bl31_early_platform_setup();
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	bl31_early_platform_setup((void *)arg0, (void *)arg1);
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	qti_setup_page_tables(BL31_BASE,
			      BL31_END - BL31_BASE,
			      BL31_CODE_BASE,
			      BL31_CODE_LIMIT,
			      BL31_RO_DATA_BASE,
			      BL31_RO_DATA_LIMIT,
			      BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END);
	enable_mmu_el3(0);
}

/*******************************************************************************
 * Perform any BL31 platform setup common to ARM standard platforms
 ******************************************************************************/
void bl31_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_qti_gic_driver_init();
	plat_qti_gic_init();
	qti_interrupt_svc_init();
	qtiseclib_bl31_platform_setup();
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
	assert(NON_SECURE == type);

	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint.
	 */
	if (bl33_image_ep_info.pc)
		return &bl33_image_ep_info;
	else
		return NULL;
}

/*******************************************************************************
 * This function is used by the architecture setup code to retrieve the counter
 * frequency for the CPU's generic timer. This value will be programmed into the
 * CNTFRQ_EL0 register. In ARM standard platforms, it returns the base frequency
 * of the system counter, which is retrieved from the first entry in the
 * frequency modes table.
 ******************************************************************************/
unsigned int plat_get_syscnt_freq2()
{
	assert(0 != g_qti_cpu_cntfrq);
	return g_qti_cpu_cntfrq;
}
