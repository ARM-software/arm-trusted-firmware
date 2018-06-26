/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <cortex_a57.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/interrupt_props.h>
#include <drivers/console.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv2.h>
#include <bl31/interrupt_mgmt.h>

#include <bpmp.h>
#include <flowctrl.h>
#include <memctrl.h>
#include <plat/common/platform.h>
#include <security_engine.h>
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
	MAP_REGION_FLAT(TEGRA_IRAM_BASE, 0x40000, /* 256KB */
			MT_DEVICE | MT_RW | MT_SECURE),
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
	/* Add the map region for security engine SE2 */
	if (tegra_chipid_is_t210_b01()) {
		mmap_add_region((uint64_t)TEGRA_SE2_BASE,
				(uint64_t)TEGRA_SE2_BASE,
				(uint64_t)TEGRA_SE2_RANGE_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE);
	}

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
	/* No of CPU cores - cluster0 */
	PLATFORM_MAX_CPUS_PER_CLUSTER,
	/* No of CPU cores - cluster1 */
	PLATFORM_MAX_CPUS_PER_CLUSTER
};

/*******************************************************************************
 * This function returns the Tegra default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return tegra_power_domain_tree_desc;
}

/*******************************************************************************
 * Handler to get the System Counter Frequency
 ******************************************************************************/
unsigned int plat_get_syscnt_freq2(void)
{
	return 19200000;
}

/*******************************************************************************
 * Maximum supported UART controllers
 ******************************************************************************/
#define TEGRA210_MAX_UART_PORTS		5

/*******************************************************************************
 * This variable holds the UART port base addresses
 ******************************************************************************/
static uint32_t tegra210_uart_addresses[TEGRA210_MAX_UART_PORTS + 1] = {
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
	static console_16550_t uart_console;
	uint32_t console_clock;

	if ((id > 0) && (id < TEGRA210_MAX_UART_PORTS)) {
		/*
		 * Reference clock used by the FPGAs is a lot slower.
		 */
		if (tegra_platform_is_fpga()) {
			console_clock = TEGRA_BOOT_UART_CLK_13_MHZ;
		} else {
			console_clock = TEGRA_BOOT_UART_CLK_408_MHZ;
		}

		(void)console_16550_register(tegra210_uart_addresses[id],
					     console_clock,
					     TEGRA_CONSOLE_BAUDRATE,
					     &uart_console);
		console_set_scope(&uart_console.console, CONSOLE_FLAG_BOOT |
			CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);
	}
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
	const plat_params_from_bl2_t *plat_params = bl31_get_plat_params();
	uint64_t val;

	/* platform parameter passed by the previous bootloader */
	if (plat_params->l2_ecc_parity_prot_dis != 1) {
		/* Enable ECC Parity Protection for Cortex-A57 CPUs */
		val = read_l2ctlr_el1();
		val |= (uint64_t)CORTEX_A57_L2_ECC_PARITY_PROTECTION_BIT;
		write_l2ctlr_el1(val);
	}

	/* Initialize security engine driver */
	if (tegra_chipid_is_t210_b01()) {
		tegra_se_init();
	}
}

/* Secure IRQs for Tegra186 */
static const interrupt_prop_t tegra210_interrupt_props[] = {
	INTR_PROP_DESC(TEGRA210_WDT_CPU_LEGACY_FIQ, GIC_HIGHEST_SEC_PRIORITY,
			GICV2_INTR_GROUP0, GIC_INTR_CFG_EDGE),
};

/*******************************************************************************
 * Handler for late platform setup
 ******************************************************************************/
void plat_late_platform_setup(void)
{
	const plat_params_from_bl2_t *plat_params = bl31_get_plat_params();
	uint64_t sc7entry_end, offset;
	int ret;
	uint32_t val;

	/* memmap TZDRAM area containing the SC7 Entry Firmware */
	if (plat_params->sc7entry_fw_base && plat_params->sc7entry_fw_size) {

		assert(plat_params->sc7entry_fw_size <= TEGRA_IRAM_A_SIZE);

		/*
		 * Verify that the SC7 entry firmware resides inside the TZDRAM
		 * aperture, _before_ the BL31 code and the start address is
		 * exactly 1MB from BL31 base.
		 */

		/* sc7entry-fw must be _before_ BL31 base */
		assert(plat_params->tzdram_base > plat_params->sc7entry_fw_base);

		sc7entry_end = plat_params->sc7entry_fw_base +
			       plat_params->sc7entry_fw_size;
		assert(sc7entry_end < plat_params->tzdram_base);

		/* sc7entry-fw start must be exactly 1MB behind BL31 base */
		offset = plat_params->tzdram_base - plat_params->sc7entry_fw_base;
		assert(offset == 0x100000);

		/* secure TZDRAM area */
		tegra_memctrl_tzdram_setup(plat_params->sc7entry_fw_base,
			plat_params->tzdram_size + offset);

		/* power off BPMP processor until SC7 entry */
		tegra_fc_bpmp_off();

		/* memmap SC7 entry firmware code */
		ret = mmap_add_dynamic_region(plat_params->sc7entry_fw_base,
				plat_params->sc7entry_fw_base,
				plat_params->sc7entry_fw_size,
				MT_SECURE | MT_RO_DATA);
		assert(ret == 0);

		/* restrict PMC access to secure world */
		val = mmio_read_32(TEGRA_MISC_BASE + APB_SLAVE_SECURITY_ENABLE);
		val |= PMC_SECURITY_EN_BIT;
		mmio_write_32(TEGRA_MISC_BASE + APB_SLAVE_SECURITY_ENABLE, val);
	}

	if (!tegra_chipid_is_t210_b01()) {
		/* restrict PMC access to secure world */
		val = mmio_read_32(TEGRA_MISC_BASE + APB_SLAVE_SECURITY_ENABLE);
		val |= PMC_SECURITY_EN_BIT;
		mmio_write_32(TEGRA_MISC_BASE + APB_SLAVE_SECURITY_ENABLE, val);
	}
}

/*******************************************************************************
 * Initialize the GIC and SGIs
 ******************************************************************************/
void plat_gic_setup(void)
{
	tegra_gic_setup(tegra210_interrupt_props, ARRAY_SIZE(tegra210_interrupt_props));
	tegra_gic_init();

	/* Enable handling for FIQs */
	tegra_fiq_handler_setup();

	/*
	 * Enable routing watchdog FIQs from the flow controller to
	 * the GICD.
	 */
	tegra_fc_enable_fiq_to_ccplex_routing();
}
/*******************************************************************************
 * Handler to indicate support for System Suspend
 ******************************************************************************/
bool plat_supports_system_suspend(void)
{
	const plat_params_from_bl2_t *plat_params = bl31_get_plat_params();

	/*
	 * sc7entry-fw is only supported by Tegra210 SoCs.
	 */
	if (!tegra_chipid_is_t210_b01() && (plat_params->sc7entry_fw_base != 0U)) {
		return true;
	} else if (tegra_chipid_is_t210_b01()) {
		return true;
	} else {
		return false;
	}
}
