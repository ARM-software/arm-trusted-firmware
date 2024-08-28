/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include "../drivers/arm/gic/v3/gicv3_private.h"

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>

#include <drivers/arm/gic.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <ele_api.h>
#include <imx8_lpuart.h>
#include <imx_plat_common.h>
#include <imx_scmi_client.h>
#include <plat_imx8.h>
#include <platform_def.h>

extern gicv3_driver_data_t gic_data;

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

extern const mmap_region_t imx_mmap[];
extern uintptr_t gpio_base[GPIO_NUM];

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
	static console_t console;

	console_lpuart_register(IMX_LPUART_BASE, IMX_BOOT_UART_CLK_IN_HZ,
		     IMX_CONSOLE_BAUDRATE, &console);

	/* this console is only used for boot stage */
	console_set_scope(&console, CONSOLE_FLAG_BOOT);

	/*
	 * tell bl3-1 where the non-secure software image is located
	 * and the entry state information.
	 */
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
	/* Assign all the GPIO pins to non-secure world by default */
	for (unsigned int i = 0U; i < GPIO_NUM; i++) {
		mmio_write_32(gpio_base[i] + 0x10, 0xffffffff);
		mmio_write_32(gpio_base[i] + 0x14, 0x3);
		mmio_write_32(gpio_base[i] + 0x18, 0xffffffff);
		mmio_write_32(gpio_base[i] + 0x1c, 0x3);
	}

	mmap_add_region(BL31_BASE, BL31_BASE, (BL31_LIMIT - BL31_BASE),
		MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE, (BL_CODE_END - BL_CODE_BASE),
		MT_MEMORY | MT_RO | MT_SECURE);

#ifdef SPD_trusty
	mmap_add_region(BL32_BASE, BL32_BASE, BL32_SIZE, MT_MEMORY | MT_RW);
#endif
	mmap_add(imx_mmap);

	init_xlat_tables();

	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	uint32_t gicr_ctlr;
	uintptr_t gicr_base;

	generic_delay_timer_init();

	/*
	 * In order to apply platform specific gic workaround, the
	 * gicv3_driver_data need to be initialized, the 'USE_GIC_DRIVER'
	 * will init it again, it should be fine.
	 */
	gic_data.gicr_base = PLAT_ARM_GICR_BASE;
	gicv3_driver_init(&gic_data);
	/* Ensure to mark the core as asleep, required for reset case. */
	gic_cpuif_disable(plat_my_core_pos());
	/* Clear LPIs */
	for (unsigned int i = 0U; i < PLATFORM_CORE_COUNT; i++) {
		gicr_base = gicv3_driver_data->rdistif_base_addrs[i];
		gicr_ctlr = gicr_read_ctlr(gicr_base);
		gicr_write_ctlr(gicr_base, gicr_ctlr & ~(GICR_CTLR_EN_LPIS_BIT));
	}

	/* get soc info */
	ele_get_soc_info();

#if HAS_XSPI_SUPPORT
	/* i.MX94 specific */
	ele_release_gmid();
#endif

	plat_imx9_scmi_setup();
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(unsigned int type)
{
	if (type == NON_SECURE) {
		return &bl33_image_ep_info;
	}

	if (type == SECURE) {
		return &bl32_image_ep_info;
	}

	return NULL;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return COUNTER_FREQUENCY;
}

#ifdef SPD_trusty
void plat_trusty_set_boot_args(aapcs64_params_t *args)
{
	args->arg0 = BL32_SIZE;
	args->arg1 = BL32_BASE;
	args->arg2 = TRUSTY_PARAMS_LEN_BYTES;
}
#endif
