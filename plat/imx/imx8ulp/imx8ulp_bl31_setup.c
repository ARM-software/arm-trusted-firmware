/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <dram.h>
#include <imx8_lpuart.h>
#include <imx8ulp_caam.h>
#include <imx_plat_common.h>
#include <plat_imx8.h>
#include <upower_api.h>
#include <xrdc.h>

#define MAP_BL31_TOTAL										   \
	MAP_REGION_FLAT(BL31_BASE, BL31_LIMIT - BL31_BASE, MT_MEMORY | MT_RW | MT_SECURE)
#define MAP_BL31_RO										   \
	MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE, MT_MEMORY | MT_RO | MT_SECURE)
#define MAP_BL32_TOTAL MAP_REGION_FLAT(BL32_BASE, BL32_SIZE, MT_MEMORY | MT_RW)
#define MAP_COHERENT_MEM									\
	MAP_REGION_FLAT(BL_COHERENT_RAM_BASE, (BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE),	\
			 MT_DEVICE | MT_RW | MT_SECURE)

#define TRUSTY_PARAMS_LEN_BYTES      (4096*2)

static const mmap_region_t imx_mmap[] = {
	DEVICE0_MAP, DEVICE1_MAP, DEVICE2_MAP,
	ELE_MAP, SEC_SIM_MAP, SRAM0_MAP,
	{0}
};

extern uint32_t upower_init(void);
extern void imx8ulp_init_scmi_server(void);

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	static console_t console;

	/* config the TPM5 clock */
	mmio_write_32(IMX_PCC3_BASE + 0xd0, 0x92000000);
	mmio_write_32(IMX_PCC3_BASE + 0xd0, 0xd2000000);

	/* enable the GPIO D,E,F non-secure access by default */
	mmio_write_32(IMX_PCC4_BASE + 0x78, 0xc0000000);
	mmio_write_32(IMX_PCC4_BASE + 0x7c, 0xc0000000);
	mmio_write_32(IMX_PCC5_BASE + 0x114, 0xc0000000);

	mmio_write_32(IMX_GPIOE_BASE + 0x10, 0xffffffff);
	mmio_write_32(IMX_GPIOE_BASE + 0x14, 0x3);
	mmio_write_32(IMX_GPIOE_BASE + 0x18, 0xffffffff);
	mmio_write_32(IMX_GPIOE_BASE + 0x1c, 0x3);

	mmio_write_32(IMX_GPIOF_BASE + 0x10, 0xffffffff);
	mmio_write_32(IMX_GPIOF_BASE + 0x14, 0x3);
	mmio_write_32(IMX_GPIOF_BASE + 0x18, 0xffffffff);
	mmio_write_32(IMX_GPIOF_BASE + 0x1c, 0x3);

	mmio_write_32(IMX_GPIOD_BASE + 0x10, 0xffffffff);
	mmio_write_32(IMX_GPIOD_BASE + 0x14, 0x3);
	mmio_write_32(IMX_GPIOD_BASE + 0x18, 0xffffffff);
	mmio_write_32(IMX_GPIOD_BASE + 0x1c, 0x3);

	console_lpuart_register(IMX_LPUART_BASE, IMX_BOOT_UART_CLK_IN_HZ,
		     IMX_CONSOLE_BAUDRATE, &console);

	/* This console is only used for boot stage */
	console_set_scope(&console, CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME);

	bl33_image_ep_info.pc = PLAT_NS_IMAGE_OFFSET;
	bl33_image_ep_info.spsr = plat_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#if defined(SPD_opteed) || defined(SPD_trusty)
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = 0;

	/* Pass TEE base and size to bl33 */
	bl33_image_ep_info.args.arg1 = BL32_BASE;
	bl33_image_ep_info.args.arg2 = BL32_SIZE;

#ifdef SPD_trusty
	bl32_image_ep_info.args.arg0 = BL32_SIZE;
	bl32_image_ep_info.args.arg1 = BL32_BASE;
#else
	/* Make sure memory is clean */
	mmio_write_32(BL32_FDT_OVERLAY_ADDR, 0);
	bl33_image_ep_info.args.arg3 = BL32_FDT_OVERLAY_ADDR;
	bl32_image_ep_info.args.arg3 = BL32_FDT_OVERLAY_ADDR;
#endif
#endif
}

void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_BL31_TOTAL,
		MAP_BL31_RO,
#if USE_COHERENT_MEM
		MAP_COHERENT_MEM,
#endif
#if defined(SPD_opteed) || defined(SPD_trusty)
		MAP_BL32_TOTAL,
#endif
		{0},
	};

	setup_page_tables(bl_regions, imx_mmap);
	enable_mmu_el3(0);

	/* TODO: Hack, refine this piece, scmi channel free */
	mmio_write_32(SRAM0_BASE + 0x4, 1);

	/* Allow M core to reset A core */
	mmio_clrbits_32(IMX_MU0B_BASE + 0x10, BIT(2));
}

void bl31_platform_setup(void)
{
	/* select the arch timer source */
	mmio_setbits_32(IMX_SIM1_BASE + 0x30, 0x8000000);

	generic_delay_timer_init();

	plat_gic_driver_init();
	plat_gic_init();

	imx8ulp_init_scmi_server();
	upower_init();

	xrdc_apply_apd_config();
	xrdc_apply_lpav_config();
	xrdc_enable();

	imx8ulp_caam_init();

	dram_init();
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(unsigned int type)
{
	if (type == NON_SECURE) {
		return &bl33_image_ep_info;
	} else {
		return &bl32_image_ep_info;
	}
}

unsigned int plat_get_syscnt_freq2(void)
{
	return COUNTER_FREQUENCY;
}

void bl31_plat_runtime_setup(void)
{
}

#ifdef SPD_trusty
void plat_trusty_set_boot_args(aapcs64_params_t *args)
{
	args->arg0 = BL32_SIZE;
	args->arg1 = BL32_BASE;
	args->arg2 = TRUSTY_PARAMS_LEN_BYTES;
}
#endif
