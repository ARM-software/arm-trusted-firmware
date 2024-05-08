/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/desc_image_load.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <plat_console.h>
#include <plat_io_storage.h>

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

void bl2_el3_early_platform_setup(u_register_t arg0, u_register_t arg1,
				  u_register_t arg2, u_register_t arg3)
{
	linflex_config_pinctrl();
	console_s32g2_register();

	plat_s32g2_io_setup();
}

void bl2_el3_plat_arch_setup(void)
{
}

