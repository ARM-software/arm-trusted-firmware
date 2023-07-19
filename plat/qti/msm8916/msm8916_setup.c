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

#define GPIO_CFG_FUNC(n)		((n) << 2)
#define GPIO_CFG_DRV_STRENGTH_MA(ma)	(((ma) / 2 - 1) << 6)

#define CLK_ENABLE			BIT_32(0)
#define CLK_OFF				BIT_32(31)
#define GCC_BLSP1_AHB_CBCR		(GCC_BASE + 0x01008)
#define GCC_BLSP1_UART_APPS_CBCR(n)	(GCC_BASE + \
	(((n) == 2) ? (0x0302c) : (0x0203c + (((n) - 1) * 0x1000))))
#define GCC_APCS_CLOCK_BRANCH_ENA_VOTE	(GCC_BASE + 0x45004)
#define BLSP1_AHB_CLK_ENA		BIT_32(10)

struct uartdm_gpios {
	unsigned int tx, rx, func;
};

static const struct uartdm_gpios uartdm_gpio_map[] = {
#if defined(PLAT_msm8909)
	{4, 5, 0x2}, {20, 21, 0x3},
#elif defined(PLAT_msm8916) || defined(PLAT_msm8939)
	{0, 1, 0x2}, {4, 5, 0x2},
#elif defined(PLAT_mdm9607)
	{12, 13, 0x2}, {4, 5, 0x2}, {0, 1, 0x1},
	{16, 17, 0x2}, {8, 9, 0x2}, {20, 21, 0x2},
#endif
};

/*
 * The previous boot stage seems to disable most of the UART setup before exit
 * so it must be enabled here again before the UART console can be used.
 */
static void msm8916_enable_blsp_uart(void)
{
	const struct uartdm_gpios *gpios = &uartdm_gpio_map[QTI_UART_NUM - 1];

	CASSERT(QTI_UART_NUM > 0 && QTI_UART_NUM <= ARRAY_SIZE(uartdm_gpio_map),
		assert_qti_blsp_uart_valid);

	/* Route GPIOs to BLSP UART */
	mmio_write_32(TLMM_GPIO_CFG(gpios->tx), GPIO_CFG_FUNC(gpios->func) |
		      GPIO_CFG_DRV_STRENGTH_MA(8));
	mmio_write_32(TLMM_GPIO_CFG(gpios->rx), GPIO_CFG_FUNC(gpios->func) |
		      GPIO_CFG_DRV_STRENGTH_MA(8));

	/* Enable AHB clock */
	mmio_setbits_32(GCC_APCS_CLOCK_BRANCH_ENA_VOTE, BLSP1_AHB_CLK_ENA);
	while (mmio_read_32(GCC_BLSP1_AHB_CBCR) & CLK_OFF) {
	}

	/* Enable BLSP UART clock */
	mmio_setbits_32(GCC_BLSP1_UART_APPS_CBCR(QTI_UART_NUM), CLK_ENABLE);
	while (mmio_read_32(GCC_BLSP1_UART_APPS_CBCR(QTI_UART_NUM)) & CLK_OFF) {
	}
}

void msm8916_early_platform_setup(void)
{
	/* Initialize the debug console as early as possible */
	msm8916_enable_blsp_uart();
	console_uartdm_register(&console, BLSP_UART_BASE);

	if (QTI_RUNTIME_UART) {
		/* Mark UART as runtime usable */
		console_set_scope(&console, CONSOLE_FLAG_BOOT |
				  CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);
	}
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
