/*
 * Copyright 2022-2024, 2026 NXP
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
#include <drivers/nxp/trdc/imx_trdc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <dram.h>
#include <ele_api.h>
#include <imx8_lpuart.h>
#include <imx_cpu_off.h>
#include <imx_intercore_svc.h>
#include <imx_interrupt_mgmt.h>
#include <imx_plat_common.h>
#include <plat_common.h>
#include <plat_imx8.h>
#include <platform_def.h>

CASSERT(SAVED_DRAM_TIMING_BASE + SAVED_DRAM_TIMING_SIZE <= OCRAM_BASE + OCRAM_SIZE,
	assert_saved_dram_timing_fits_ocram);

#define MAP_BL31_TOTAL										   \
	MAP_REGION_FLAT(BL31_BASE, BL31_SIZE, MT_MEMORY | MT_RW | MT_SECURE)
#define MAP_BL31_RO										   \
	MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE, MT_MEMORY | MT_RO | MT_SECURE)

static const uintptr_t gicr_base_addrs[] = {
	PLAT_ARM_GICR_BASE,
	0U
};

static const mmap_region_t imx_mmap[] = {
	AIPS1_MAP, AIPS2_MAP, AIPS4_MAP, GIC_MAP,
	TRDC_A_MAP, TRDC_W_MAP, TRDC_M_MAP,
	TRDC_N_MAP, DDRMIX_MAP, GPIO_MAP, S400_MU_MAP,
	FSB_MAP, NIC_GPV_MAP, AIPS3_MAP,
	{0},
};

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
	static console_t console;

	console_lpuart_register(IMX_LPUART_BASE, IMX_BOOT_UART_CLK_IN_HZ,
		     IMX_CONSOLE_BAUDRATE, &console);

	/* This console is only used for boot stage */
	console_set_scope(&console, CONSOLE_FLAG_BOOT);

	/*
	 * tell BL3-1 where the non-secure software image is located
	 * and the entry state information.
	 */
	bl33_image_ep_info.pc = PLAT_NS_IMAGE_OFFSET;
	bl33_image_ep_info.spsr = plat_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#if defined(SPD_opteed)
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = 0;

	/* Pass TEE base and size to bl33 */
	bl33_image_ep_info.args.arg1 = BL32_BASE;
	bl33_image_ep_info.args.arg2 = BL32_SIZE;

	/* Make sure memory is clean */
	mmio_write_32(BL32_FDT_OVERLAY_ADDR, 0);
	bl33_image_ep_info.args.arg3 = BL32_FDT_OVERLAY_ADDR;
	bl32_image_ep_info.args.arg3 = BL32_FDT_OVERLAY_ADDR;
#endif

	imx_bl31_params_parse(arg0, OCRAM_BASE, OCRAM_SIZE,
				    &bl32_image_ep_info, &bl33_image_ep_info);
}

void bl31_plat_arch_setup(void)
{
	/* no coherence memory support on i.MX9 */
	const mmap_region_t bl_regions[] = {
		MAP_BL31_TOTAL,
		MAP_BL31_RO,
		{0}
	};

	/* Assign all the GPIO pins to non-secure world by default */
	mmio_write_32(GPIO2_BASE + 0x10, 0xffffffff);
	mmio_write_32(GPIO2_BASE + 0x14, 0x3);
	mmio_write_32(GPIO2_BASE + 0x18, 0xffffffff);
	mmio_write_32(GPIO2_BASE + 0x1c, 0x3);

	mmio_write_32(GPIO3_BASE + 0x10, 0xffffffff);
	mmio_write_32(GPIO3_BASE + 0x14, 0x3);
	mmio_write_32(GPIO3_BASE + 0x18, 0xffffffff);
	mmio_write_32(GPIO3_BASE + 0x1c, 0x3);

	mmio_write_32(GPIO4_BASE + 0x10, 0xffffffff);
	mmio_write_32(GPIO4_BASE + 0x14, 0x3);
	mmio_write_32(GPIO4_BASE + 0x18, 0xffffffff);
	mmio_write_32(GPIO4_BASE + 0x1c, 0x3);

	mmio_write_32(GPIO1_BASE + 0x10, 0xffffffff);
	mmio_write_32(GPIO1_BASE + 0x14, 0x3);
	mmio_write_32(GPIO1_BASE + 0x18, 0xffffffff);
	mmio_write_32(GPIO1_BASE + 0x1c, 0x3);

	setup_page_tables(bl_regions, imx_mmap);
	enable_mmu_el3(0);

	/* trdc must be initialized */
	trdc_config();

	gic_set_gicr_frames(gicr_base_addrs);
}

void plat_gic_pre_pcpu_init(unsigned int cpu_idx)
{
}

void bl31_platform_setup(void)
{
	generic_delay_timer_init();

	/* Retrieve SoC info from EdgeLock Enclave */
	ele_get_soc_info();

	imx_el3_interrupt_config();
	imx_intercore_svc_init();
	imx_cpuoff_init();

	/* Init the dram info */
	dram_info_init(SAVED_DRAM_TIMING_BASE);
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
