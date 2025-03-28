/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/generic_delay_timer.h>
#include <imx_usdhc.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <plat_console.h>
#include <s32cc-clk-drv.h>

#include <plat_io_storage.h>
#include <s32cc-bl-common.h>
#include <s32cc-ncore.h>

#define SIUL20_BASE		UL(0x4009C000)
#define SIUL2_PC09_MSCR		UL(0x4009C2E4)
#define SIUL2_PC10_MSCR		UL(0x4009C2E8)
#define SIUL2_PC10_LIN0_IMCR	UL(0x4009CA40)

#define LIN0_TX_MSCR_CFG	U(0x00214001)
#define LIN0_RX_MSCR_CFG	U(0x00094000)
#define LIN0_RX_IMCR_CFG	U(0x00000002)

struct bl_load_info *plat_get_bl_image_load_info(void)
{
	return get_bl_load_info_from_mem_params_desc();
}

struct bl_params *plat_get_next_bl_params(void)
{
	return get_next_bl_params_from_mem_params_desc();
}

void plat_flush_next_bl_params(void)
{
	flush_bl_params_desc();
}

void bl2_platform_setup(void)
{
	int ret;

	ret = mmap_add_dynamic_region(S32G_FIP_BASE, S32G_FIP_BASE,
				      S32G_FIP_SIZE,
				      MT_MEMORY | MT_RW | MT_SECURE);
	if (ret != 0) {
		panic();
	}
}

static int s32g_mmap_siul2(void)
{
	return mmap_add_dynamic_region(SIUL20_BASE, SIUL20_BASE, PAGE_SIZE,
				       MT_DEVICE | MT_RW | MT_SECURE);
}

static void linflex_config_pinctrl(void)
{
	/* set PC09 - MSCR[41] - for UART0 TXD */
	mmio_write_32(SIUL2_PC09_MSCR, LIN0_TX_MSCR_CFG);
	/* set PC10 - MSCR[42] - for UART0 RXD */
	mmio_write_32(SIUL2_PC10_MSCR, LIN0_RX_MSCR_CFG);
	/* set PC10 - MSCR[512]/IMCR[0] - for UART0 RXD */
	mmio_write_32(SIUL2_PC10_LIN0_IMCR, LIN0_RX_IMCR_CFG);
}

static void init_s32g_usdhc(void)
{
	static struct mmc_device_info sd_device_info = {
		.mmc_dev_type = MMC_IS_SD_HC,
		.ocr_voltage = OCR_3_2_3_3 | OCR_3_3_3_4,
	};
	imx_usdhc_params_t params;

	zeromem(&params, sizeof(imx_usdhc_params_t));

	params.reg_base = S32G_USDHC_BASE;
	params.clk_rate = 25000000;
	params.bus_width = MMC_BUS_WIDTH_4;
	params.flags = MMC_FLAG_SD_CMD6;

	imx_usdhc_init(&params, &sd_device_info);
}

static void plat_s32_mmc_setup(void)
{
	init_s32g_usdhc();
}

void bl2_el3_early_platform_setup(u_register_t arg0, u_register_t arg1,
				  u_register_t arg2, u_register_t arg3)
{
	int ret;

	/* Restore (clear) the CAIUTC[IsolEn] bit for the primary cluster, which
	 * we have manually set during early BL2 boot.
	 */
	ncore_disable_caiu_isolation(A53_CLUSTER0_CAIU);

	ncore_init();
	ncore_caiu_online(A53_CLUSTER0_CAIU);

	ret = s32cc_init_core_clocks();
	if (ret != 0) {
		panic();
	}

	ret = s32cc_bl_mmu_setup();
	if (ret != 0) {
		panic();
	}

	ret = s32cc_init_early_clks();
	if (ret != 0) {
		panic();
	}

	ret = s32g_mmap_siul2();
	if (ret != 0) {
		panic();
	}

	generic_delay_timer_init();

	/* Configure the generic timer frequency to ensure proper operation
	 * of the architectural timer in BL2.
	 */
	write_cntfrq_el0(plat_get_syscnt_freq2());

	linflex_config_pinctrl();
	console_s32g2_register();

	plat_s32_mmc_setup();

	plat_s32g2_io_setup();
}

void bl2_el3_plat_arch_setup(void)
{
}

int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
	const struct bl_mem_params_node *desc = get_bl_mem_params_node(image_id);
	const struct image_info *img_info;
	size_t size;

	if (desc == NULL) {
		return -EINVAL;
	}

	img_info = &desc->image_info;

	if ((img_info == NULL) || (img_info->image_max_size == 0U)) {
		return -EINVAL;
	}

	size = page_align(img_info->image_max_size, UP);

	return mmap_add_dynamic_region(img_info->image_base,
				       img_info->image_base,
				       size,
				       MT_MEMORY | MT_RW | MT_SECURE);
}
