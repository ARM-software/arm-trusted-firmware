/*
 * Copyright (c) 2021-2023, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "msm8916_gicv2.h"
#include <msm8916_mmap.h>
#include "msm8916_setup.h"
#include <uartdm_console.h>

static const mmap_region_t msm8916_mmap[] = {
	MAP_REGION_FLAT(PCNOC_BASE, PCNOC_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE | MT_EXECUTE_NEVER),
	MAP_REGION_FLAT(APCS_BASE, APCS_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE | MT_EXECUTE_NEVER),
	{},
};

static console_t console;

unsigned int plat_get_syscnt_freq2(void)
{
	return PLAT_SYSCNT_FREQ;
}

#define GPIO_BLSP_UART2_TX		4
#define GPIO_BLSP_UART2_RX		5
#define GPIO_CFG_FUNC_BLSP_UART2	(U(0x2) << 2)
#define GPIO_CFG_DRV_STRENGTH_16MA	(U(0x7) << 6)

#define CLK_ENABLE			BIT_32(0)
#define CLK_OFF				BIT_32(31)
#define GCC_BLSP1_AHB_CBCR		(GCC_BASE + 0x01008)
#define GCC_BLSP1_UART2_APPS_CBCR	(GCC_BASE + 0x0302c)
#define GCC_APCS_CLOCK_BRANCH_ENA_VOTE	(GCC_BASE + 0x45004)
#define BLSP1_AHB_CLK_ENA		BIT_32(10)

/*
 * The previous boot stage seems to disable most of the UART setup before exit
 * so it must be enabled here again before the UART console can be used.
 */
static void msm8916_enable_blsp_uart2(void)
{
	/* Route GPIOs to BLSP UART2 */
	mmio_write_32(TLMM_GPIO_CFG(GPIO_BLSP_UART2_TX),
		      GPIO_CFG_FUNC_BLSP_UART2 | GPIO_CFG_DRV_STRENGTH_16MA);
	mmio_write_32(TLMM_GPIO_CFG(GPIO_BLSP_UART2_RX),
		      GPIO_CFG_FUNC_BLSP_UART2 | GPIO_CFG_DRV_STRENGTH_16MA);

	/* Enable AHB clock */
	mmio_setbits_32(GCC_APCS_CLOCK_BRANCH_ENA_VOTE, BLSP1_AHB_CLK_ENA);
	while (mmio_read_32(GCC_BLSP1_AHB_CBCR) & CLK_OFF)
		;

	/* Enable BLSP UART2 clock */
	mmio_setbits_32(GCC_BLSP1_UART2_APPS_CBCR, CLK_ENABLE);
	while (mmio_read_32(GCC_BLSP1_UART2_APPS_CBCR) & CLK_OFF)
		;
}

void msm8916_early_platform_setup(void)
{
	/* Initialize the debug console as early as possible */
	msm8916_enable_blsp_uart2();
	console_uartdm_register(&console, BLSP_UART2_BASE);
}

void msm8916_plat_arch_setup(uintptr_t base, size_t size)
{
	mmap_add_region(base, base, size, MT_RW_DATA | MT_SECURE);
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);
	mmap_add_region(BL_RO_DATA_BASE, BL_RO_DATA_BASE,
			BL_RO_DATA_END - BL_RO_DATA_BASE,
			MT_RO_DATA | MT_SECURE);
	mmap_add_region(BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
			MT_DEVICE | MT_RW | MT_SECURE | MT_EXECUTE_NEVER);

	mmap_add(msm8916_mmap);
	init_xlat_tables();
}

void msm8916_platform_setup(void)
{
	generic_delay_timer_init();
	msm8916_gicv2_init();
}
