/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <arm_gic.h>
#include <assert.h>
#include <bl_common.h>
#include <cci.h>
#include <console.h>
#include <debug.h>
#include <errno.h>
#include <generic_delay_timer.h>
#include <gicv2.h>
#include <hi3660.h>
#include <hisi_ipc.h>
#include <platform_def.h>

#include "hikey960_def.h"
#include "hikey960_private.h"

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL31_RO_BASE	(unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT	(unsigned long)(&__RO_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL31_COHERENT_RAM_BASE	(unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT	(unsigned long)(&__COHERENT_RAM_END__)

static entry_point_info_t bl32_ep_info;
static entry_point_info_t bl33_ep_info;

/******************************************************************************
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 *****************************************************************************/
const unsigned int g0_interrupt_array[] = {
	IRQ_SEC_PHY_TIMER,
	IRQ_SEC_SGI_0
};

const gicv2_driver_data_t hikey960_gic_data = {
	.gicd_base = GICD_REG_BASE,
	.gicc_base = GICC_REG_BASE,
	.g0_interrupt_num = ARRAY_SIZE(g0_interrupt_array),
	.g0_interrupt_array = g0_interrupt_array,
};

static const int cci_map[] = {
	CCI400_SL_IFACE3_CLUSTER_IX,
	CCI400_SL_IFACE4_CLUSTER_IX
};

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ? &bl33_ep_info : &bl32_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	return NULL;
}

#if LOAD_IMAGE_V2
void bl31_early_platform_setup(void *from_bl2,
			       void *plat_params_from_bl2)
#else
void bl31_early_platform_setup(bl31_params_t *from_bl2,
		void *plat_params_from_bl2)
#endif
{
	unsigned int id, uart_base;

	generic_delay_timer_init();
	hikey960_read_boardid(&id);
	if (id == 5300)
		uart_base = PL011_UART5_BASE;
	else
		uart_base = PL011_UART6_BASE;

	/* Initialize the console to provide early debug support */
	console_init(uart_base, PL011_UART_CLK_IN_HZ, PL011_BAUDRATE);

	/* Initialize CCI driver */
	cci_init(CCI400_REG_BASE, cci_map, ARRAY_SIZE(cci_map));
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));

#if LOAD_IMAGE_V2
	/*
	 * Check params passed from BL2 should not be NULL,
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)from_bl2;
	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 and BL32 (if present), entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params) {
		if (bl_params->image_id == BL32_IMAGE_ID)
			bl32_ep_info = *bl_params->ep_info;

		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}

	if (bl33_ep_info.pc == 0)
		panic();

#else /* LOAD_IMAGE_V2 */

	/*
	 * Check params passed from BL2 should not be NULL,
	 */
	assert(from_bl2 != NULL);
	assert(from_bl2->h.type == PARAM_BL31);
	assert(from_bl2->h.version >= VERSION_1);

	/*
	 * Copy BL3-2 and BL3-3 entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	bl32_ep_info = *from_bl2->bl32_ep_info;
	bl33_ep_info = *from_bl2->bl33_ep_info;
#endif /* LOAD_IMAGE_V2 */
}

void bl31_plat_arch_setup(void)
{
	hikey960_init_mmu_el3(BL31_BASE,
			BL31_LIMIT - BL31_BASE,
			BL31_RO_BASE,
			BL31_RO_LIMIT,
			BL31_COHERENT_RAM_BASE,
			BL31_COHERENT_RAM_LIMIT);
}

void bl31_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	gicv2_driver_init(&hikey960_gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();

	hisi_ipc_init();
}

void bl31_plat_runtime_setup(void)
{
}
