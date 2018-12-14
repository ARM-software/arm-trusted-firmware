/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <drivers/arm/tzc380.h>
#include <drivers/console.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables.h>
#include <plat/common/platform.h>

#include <gpc.h>
#include <imx_uart.h>
#include <plat_imx8.h>

IMPORT_SYM(uintptr_t, __COHERENT_RAM_START__, BL31_COHERENT_RAM_START);
IMPORT_SYM(uintptr_t, __COHERENT_RAM_END__, BL31_COHERENT_RAM_END);
IMPORT_SYM(uintptr_t, __RO_START__, BL31_RO_START);
IMPORT_SYM(uintptr_t, __RO_END__, BL31_RO_END);
IMPORT_SYM(uintptr_t, __RW_START__, BL31_RW_START);
IMPORT_SYM(uintptr_t, __RW_END__, BL31_RW_END);

static const mmap_region_t imx_mmap[] = {
	MAP_REGION_FLAT(GPV_BASE, GPV_SIZE, MT_DEVICE | MT_RW), /* GPV map */
	MAP_REGION_FLAT(IMX_AIPS_BASE, IMX_AIPS_SIZE, MT_DEVICE | MT_RW), /* AIPS map */
	MAP_REGION_FLAT(IMX_GIC_BASE, IMX_GIC_SIZE, MT_DEVICE | MT_RW), /* GIC map */
	{0},
};

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/* get SPSR for BL33 entry */
static uint32_t get_spsr_for_bl33_entry(void)
{
	unsigned long el_status;
	unsigned long mode;
	uint32_t spsr;

	/* figure out what mode we enter the non-secure world */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	mode = (el_status) ? MODE_EL2 : MODE_EL1;

	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

static void bl31_tz380_setup(void)
{
	unsigned int val;

	val = mmio_read_32(IMX_IOMUX_GPR_BASE + IOMUXC_GPR10);
	if ((val & GPR_TZASC_EN) != GPR_TZASC_EN)
		return;

	tzc380_init(IMX_TZASC_BASE);
	/*
	 * Need to substact offset 0x40000000 from CPU address when
	 * programming tzasc region for i.mx8mq. Enable 1G-5G S/NS RW
	 */
	tzc380_configure_region(0, 0x00000000, TZC_ATTR_REGION_SIZE(TZC_REGION_SIZE_4G) |
				TZC_ATTR_REGION_EN_MASK | TZC_ATTR_SP_ALL);
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	int i;
	/* enable CSU NS access permission */
	for (i = 0; i < 64; i++) {
		mmio_write_32(IMX_CSU_BASE + i * 4, 0xffffffff);
	}

#if DEBUG_CONSOLE
	static console_uart_t console;

	console_uart_register(IMX_BOOT_UART_BASE, IMX_BOOT_UART_CLK_IN_HZ,
		IMX_CONSOLE_BAUDRATE, &console);
#endif
	/*
	 * tell BL3-1 where the non-secure software image is located
	 * and the entry state information.
	 */
	bl33_image_ep_info.pc = PLAT_NS_IMAGE_OFFSET;
	bl33_image_ep_info.spsr = get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	bl31_tz380_setup();
}

void bl31_plat_arch_setup(void)
{
	mmap_add_region(BL31_RO_START, BL31_RO_START, (BL31_RO_END - BL31_RO_START),
		MT_MEMORY | MT_RO | MT_SECURE);
	mmap_add_region(BL31_RW_START, BL31_RW_START, (BL31_RW_END - BL31_RW_START),
		MT_MEMORY | MT_RW | MT_SECURE);

	mmap_add(imx_mmap);

#if USE_COHERENT_MEM
	mmap_add_region(BL31_COHERENT_RAM_START, BL31_COHERENT_RAM_START,
		BL31_COHERENT_RAM_END - BL31_COHERENT_RAM_START,
		MT_DEVICE | MT_RW | MT_SECURE);
#endif
	/* setup xlat table */
	init_xlat_tables();
	/* enable the MMU */
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	/* init the GICv3 cpu and distributor interface */
	plat_gic_driver_init();
	plat_gic_init();

	/* gpc init */
	imx_gpc_init();
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(unsigned int type)
{
	if (type == NON_SECURE)
		return &bl33_image_ep_info;
	if (type == SECURE)
		return &bl32_image_ep_info;

	return NULL;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return COUNTER_FREQUENCY;
}

void bl31_plat_runtime_setup(void)
{
	return;
}
