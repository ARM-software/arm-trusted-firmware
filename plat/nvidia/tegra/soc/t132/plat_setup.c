/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <drivers/console.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <memctrl.h>
#include <plat/common/platform.h>
#include <tegra_def.h>
#include <tegra_platform.h>
#include <tegra_private.h>

/* sets of MMIO ranges setup */
#define MMIO_RANGE_0_ADDR	0x50000000
#define MMIO_RANGE_1_ADDR	0x60000000
#define MMIO_RANGE_2_ADDR	0x70000000
#define MMIO_RANGE_SIZE		0x200000

/*
 * Table of regions to map using the MMU.
 */
static const mmap_region_t tegra_mmap[] = {
	MAP_REGION_FLAT(MMIO_RANGE_0_ADDR, MMIO_RANGE_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MMIO_RANGE_1_ADDR, MMIO_RANGE_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MMIO_RANGE_2_ADDR, MMIO_RANGE_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	{0}
};

/*******************************************************************************
 * Set up the pagetables as per the platform memory map & initialize the MMU
 ******************************************************************************/
const mmap_region_t *plat_get_mmio_map(void)
{
	/* MMIO space */
	return tegra_mmap;
}

/*******************************************************************************
 * The Tegra power domain tree has a single system level power domain i.e. a
 * single root node. The first entry in the power domain descriptor specifies
 * the number of power domains at the highest power level.
 *******************************************************************************
 */
const unsigned char tegra_power_domain_tree_desc[] = {
	/* No of root nodes */
	1,
	/* No of clusters */
	PLATFORM_CLUSTER_COUNT,
	/* No of CPU cores */
	PLATFORM_CORE_COUNT,
};

/*******************************************************************************
 * This function returns the Tegra default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return tegra_power_domain_tree_desc;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return 12000000;
}

/*******************************************************************************
 * Maximum supported UART controllers
 ******************************************************************************/
#define TEGRA132_MAX_UART_PORTS		5

/*******************************************************************************
 * This variable holds the UART port base addresses
 ******************************************************************************/
static uint32_t tegra132_uart_addresses[TEGRA132_MAX_UART_PORTS + 1] = {
	0,	/* undefined - treated as an error case */
	TEGRA_UARTA_BASE,
	TEGRA_UARTB_BASE,
	TEGRA_UARTC_BASE,
	TEGRA_UARTD_BASE,
	TEGRA_UARTE_BASE,
};

/*******************************************************************************
 * Enable console corresponding to the console ID
 ******************************************************************************/
void plat_enable_console(int32_t id)
{
	static console_t uart_console;
	uint32_t console_clock;

	if ((id > 0) && (id < TEGRA132_MAX_UART_PORTS)) {
		/*
		 * Reference clock used by the FPGAs is a lot slower.
		 */
		if (tegra_platform_is_fpga()) {
			console_clock = TEGRA_BOOT_UART_CLK_13_MHZ;
		} else {
			console_clock = TEGRA_BOOT_UART_CLK_408_MHZ;
		}

		(void)console_16550_register(tegra132_uart_addresses[id],
					     console_clock,
					     TEGRA_CONSOLE_BAUDRATE,
					     &uart_console);
		console_set_scope(&uart_console, CONSOLE_FLAG_BOOT |
			CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);
	}
}

/*******************************************************************************
 * Initialize the GIC and SGIs
 ******************************************************************************/
void plat_gic_setup(void)
{
	tegra_gic_setup(NULL, 0);
	tegra_gic_init();
}

/*******************************************************************************
 * Return pointer to the BL31 params from previous bootloader
 ******************************************************************************/
struct tegra_bl31_params *plat_get_bl31_params(void)
{
	return NULL;
}

/*******************************************************************************
 * Return pointer to the BL31 platform params from previous bootloader
 ******************************************************************************/
plat_params_from_bl2_t *plat_get_bl31_plat_params(void)
{
	return NULL;
}

/*******************************************************************************
 * Handler for early platform setup
 ******************************************************************************/
void plat_early_platform_setup(void)
{
	plat_params_from_bl2_t *plat_params = bl31_get_plat_params();

	/* Verify chip id is t132 */
	assert(tegra_chipid_is_t132());

	/*
	 * Do initial security configuration to allow DRAM/device access.
	 */
	tegra_memctrl_tzdram_setup(plat_params->tzdram_base,
			(uint32_t)plat_params->tzdram_size);
}

/*******************************************************************************
 * Handler for late platform setup
 ******************************************************************************/
void plat_late_platform_setup(void)
{
	; /* do nothing */
}

/*******************************************************************************
 * Handler to indicate support for System Suspend
 ******************************************************************************/
bool plat_supports_system_suspend(void)
{
	return true;
}

/*******************************************************************************
 * Platform specific runtime setup.
 ******************************************************************************/
void plat_runtime_setup(void)
{
	/*
	 * During cold boot, it is observed that the arbitration
	 * bit is set in the Memory controller leading to false
	 * error interrupts in the non-secure world. To avoid
	 * this, clean the interrupt status register before
	 * booting into the non-secure world
	 */
	tegra_memctrl_clear_pending_interrupts();

	/*
	 * During boot, USB3 and flash media (SDMMC/SATA) devices need
	 * access to IRAM. Because these clients connect to the MC and
	 * do not have a direct path to the IRAM, the MC implements AHB
	 * redirection during boot to allow path to IRAM. In this mode
	 * accesses to a programmed memory address aperture are directed
	 * to the AHB bus, allowing access to the IRAM. This mode must be
	 * disabled before we jump to the non-secure world.
	 */
	tegra_memctrl_disable_ahb_redirection();
}
