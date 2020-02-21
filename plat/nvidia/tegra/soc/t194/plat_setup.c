/*
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl31/bl31.h>
#include <common/bl_common.h>
#include <common/interrupt_props.h>
#include <drivers/console.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <cortex_a57.h>
#include <common/debug.h>
#include <denver.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv2.h>
#include <bl31/interrupt_mgmt.h>
#include <mce.h>
#include <mce_private.h>
#include <plat/common/platform.h>
#include <spe.h>
#include <tegra_def.h>
#include <tegra_mc_def.h>
#include <tegra_platform.h>
#include <tegra_private.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

/* ID for spe-console */
#define TEGRA_CONSOLE_SPE_ID		0xFE

/*******************************************************************************
 * The Tegra power domain tree has a single system level power domain i.e. a
 * single root node. The first entry in the power domain descriptor specifies
 * the number of power domains at the highest power level.
 *******************************************************************************
 */
static const uint8_t tegra_power_domain_tree_desc[] = {
	/* No of root nodes */
	1,
	/* No of clusters */
	PLATFORM_CLUSTER_COUNT,
	/* No of CPU cores - cluster0 */
	PLATFORM_MAX_CPUS_PER_CLUSTER,
	/* No of CPU cores - cluster1 */
	PLATFORM_MAX_CPUS_PER_CLUSTER,
	/* No of CPU cores - cluster2 */
	PLATFORM_MAX_CPUS_PER_CLUSTER,
	/* No of CPU cores - cluster3 */
	PLATFORM_MAX_CPUS_PER_CLUSTER
};

/*******************************************************************************
 * This function returns the Tegra default topology tree information.
 ******************************************************************************/
const uint8_t *plat_get_power_domain_tree_desc(void)
{
	return tegra_power_domain_tree_desc;
}

/*
 * Table of regions to map using the MMU.
 */
static const mmap_region_t tegra_mmap[] = {
	MAP_REGION_FLAT(TEGRA_MISC_BASE, 0x4000U, /* 16KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_TSA_BASE, 0x20000U, /* 128KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_GPCDMA_BASE, 0x10000U, /* 64KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_MC_STREAMID_BASE, 0x8000U, /* 32KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_MC_BASE, 0x8000U, /* 32KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
#if !ENABLE_CONSOLE_SPE
	MAP_REGION_FLAT(TEGRA_UARTA_BASE, 0x20000U, /* 128KB - UART A, B*/
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_UARTC_BASE, 0x20000U, /* 128KB - UART C, G */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_UARTD_BASE, 0x30000U, /* 192KB - UART D, E, F */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
#endif
	MAP_REGION_FLAT(TEGRA_XUSB_PADCTL_BASE, 0x2000U, /* 8KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_GICD_BASE, 0x1000, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_GICC_BASE, 0x1000, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SE0_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_PKA1_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_RNG1_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_HSP_DBELL_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
#if ENABLE_CONSOLE_SPE
	MAP_REGION_FLAT(TEGRA_CONSOLE_SPE_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
#endif
	MAP_REGION_FLAT(TEGRA_TMRUS_BASE, TEGRA_TMRUS_SIZE, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SCRATCH_BASE, 0x1000U, /* 4KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SMMU2_BASE, 0x800000U, /* 8MB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SMMU1_BASE, 0x800000U, /* 8MB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SMMU0_BASE, 0x800000U, /* 8MB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_BPMP_IPC_TX_PHYS_BASE, 0x10000U, /* 64KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
	MAP_REGION_FLAT(TEGRA_CAR_RESET_BASE, 0x10000U, /* 64KB */
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE),
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
 * Handler to get the System Counter Frequency
 ******************************************************************************/
uint32_t plat_get_syscnt_freq2(void)
{
	return 31250000;
}

#if !ENABLE_CONSOLE_SPE
/*******************************************************************************
 * Maximum supported UART controllers
 ******************************************************************************/
#define TEGRA194_MAX_UART_PORTS		7

/*******************************************************************************
 * This variable holds the UART port base addresses
 ******************************************************************************/
static uint32_t tegra194_uart_addresses[TEGRA194_MAX_UART_PORTS + 1] = {
	0,	/* undefined - treated as an error case */
	TEGRA_UARTA_BASE,
	TEGRA_UARTB_BASE,
	TEGRA_UARTC_BASE,
	TEGRA_UARTD_BASE,
	TEGRA_UARTE_BASE,
	TEGRA_UARTF_BASE,
	TEGRA_UARTG_BASE
};
#endif

/*******************************************************************************
 * Enable console corresponding to the console ID
 ******************************************************************************/
void plat_enable_console(int32_t id)
{
	uint32_t console_clock = 0U;

#if ENABLE_CONSOLE_SPE
	static console_spe_t spe_console;

	if (id == TEGRA_CONSOLE_SPE_ID) {
		(void)console_spe_register(TEGRA_CONSOLE_SPE_BASE,
					   console_clock,
					   TEGRA_CONSOLE_BAUDRATE,
					   &spe_console);
		console_set_scope(&spe_console.console, CONSOLE_FLAG_BOOT |
			CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);
	}
#else
	static console_16550_t uart_console;

	if ((id > 0) && (id < TEGRA194_MAX_UART_PORTS)) {
		/*
		 * Reference clock used by the FPGAs is a lot slower.
		 */
		if (tegra_platform_is_fpga()) {
			console_clock = TEGRA_BOOT_UART_CLK_13_MHZ;
		} else {
			console_clock = TEGRA_BOOT_UART_CLK_408_MHZ;
		}

		(void)console_16550_register(tegra194_uart_addresses[id],
					     console_clock,
					     TEGRA_CONSOLE_BAUDRATE,
					     &uart_console);
		console_set_scope(&uart_console.console, CONSOLE_FLAG_BOOT |
			CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);
	}
#endif
}

/*******************************************************************************
 * Handler for early platform setup
 ******************************************************************************/
void plat_early_platform_setup(void)
{
	/* sanity check MCE firmware compatibility */
	mce_verify_firmware_version();

	/*
	 * Program XUSB STREAMIDs
	 * ======================
	 * T19x XUSB has support for XUSB virtualization. It will have one
	 * physical function (PF) and four Virtual function (VF)
	 *
	 * There were below two SIDs for XUSB until T186.
	 * 1) #define TEGRA_SID_XUSB_HOST    0x1bU
	 * 2) #define TEGRA_SID_XUSB_DEV    0x1cU
	 *
	 * We have below four new SIDs added for VF(s)
	 * 3) #define TEGRA_SID_XUSB_VF0    0x5dU
	 * 4) #define TEGRA_SID_XUSB_VF1    0x5eU
	 * 5) #define TEGRA_SID_XUSB_VF2    0x5fU
	 * 6) #define TEGRA_SID_XUSB_VF3    0x60U
	 *
	 * When virtualization is enabled then we have to disable SID override
	 * and program above SIDs in below newly added SID registers in XUSB
	 * PADCTL MMIO space. These registers are TZ protected and so need to
	 * be done in ATF.
	 * a) #define XUSB_PADCTL_HOST_AXI_STREAMID_PF_0 (0x136cU)
	 * b) #define XUSB_PADCTL_DEV_AXI_STREAMID_PF_0  (0x139cU)
	 * c) #define XUSB_PADCTL_HOST_AXI_STREAMID_VF_0 (0x1370U)
	 * d) #define XUSB_PADCTL_HOST_AXI_STREAMID_VF_1 (0x1374U)
	 * e) #define XUSB_PADCTL_HOST_AXI_STREAMID_VF_2 (0x1378U)
	 * f) #define XUSB_PADCTL_HOST_AXI_STREAMID_VF_3 (0x137cU)
	 *
	 * This change disables SID override and programs XUSB SIDs in
	 * above registers to support both virtualization and
	 * non-virtualization platforms
	 */
	if (tegra_platform_is_silicon() || tegra_platform_is_fpga()) {

		mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
			XUSB_PADCTL_HOST_AXI_STREAMID_PF_0, TEGRA_SID_XUSB_HOST);
		mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
			XUSB_PADCTL_HOST_AXI_STREAMID_VF_0, TEGRA_SID_XUSB_VF0);
		mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
			XUSB_PADCTL_HOST_AXI_STREAMID_VF_1, TEGRA_SID_XUSB_VF1);
		mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
			XUSB_PADCTL_HOST_AXI_STREAMID_VF_2, TEGRA_SID_XUSB_VF2);
		mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
			XUSB_PADCTL_HOST_AXI_STREAMID_VF_3, TEGRA_SID_XUSB_VF3);
		mmio_write_32(TEGRA_XUSB_PADCTL_BASE +
			XUSB_PADCTL_DEV_AXI_STREAMID_PF_0, TEGRA_SID_XUSB_DEV);
	}
}

/* Secure IRQs for Tegra194 */
static const interrupt_prop_t tegra194_interrupt_props[] = {
	INTR_PROP_DESC(TEGRA194_TOP_WDT_IRQ, GIC_HIGHEST_SEC_PRIORITY,
			GICV2_INTR_GROUP0, GIC_INTR_CFG_EDGE),
	INTR_PROP_DESC(TEGRA194_AON_WDT_IRQ, GIC_HIGHEST_SEC_PRIORITY,
			GICV2_INTR_GROUP0, GIC_INTR_CFG_EDGE)
};

/*******************************************************************************
 * Initialize the GIC and SGIs
 ******************************************************************************/
void plat_gic_setup(void)
{
	tegra_gic_setup(tegra194_interrupt_props, ARRAY_SIZE(tegra194_interrupt_props));
	tegra_gic_init();

	/*
	 * Initialize the FIQ handler
	 */
	tegra_fiq_handler_setup();
}

/*******************************************************************************
 * Return pointer to the BL31 params from previous bootloader
 ******************************************************************************/
struct tegra_bl31_params *plat_get_bl31_params(void)
{
	uint64_t val;

	val = (mmio_read_32(TEGRA_SCRATCH_BASE + SCRATCH_BL31_PARAMS_HI_ADDR) &
		SCRATCH_BL31_PARAMS_HI_ADDR_MASK) >> SCRATCH_BL31_PARAMS_HI_ADDR_SHIFT;
	val <<= 32;
	val |= mmio_read_32(TEGRA_SCRATCH_BASE + SCRATCH_BL31_PARAMS_LO_ADDR);

	return (struct tegra_bl31_params *)(uintptr_t)val;
}

/*******************************************************************************
 * Return pointer to the BL31 platform params from previous bootloader
 ******************************************************************************/
plat_params_from_bl2_t *plat_get_bl31_plat_params(void)
{
	uint64_t val;

	val = (mmio_read_32(TEGRA_SCRATCH_BASE + SCRATCH_BL31_PLAT_PARAMS_HI_ADDR) &
		SCRATCH_BL31_PLAT_PARAMS_HI_ADDR_MASK) >> SCRATCH_BL31_PLAT_PARAMS_HI_ADDR_SHIFT;
	val <<= 32;
	val |= mmio_read_32(TEGRA_SCRATCH_BASE + SCRATCH_BL31_PLAT_PARAMS_LO_ADDR);

	return (plat_params_from_bl2_t *)(uintptr_t)val;
}

/*******************************************************************************
 * Handler for late platform setup
 ******************************************************************************/
void plat_late_platform_setup(void)
{
#if ENABLE_STRICT_CHECKING_MODE
	/*
	 * Enable strict checking after programming the GSC for
	 * enabling TZSRAM and TZDRAM
	 */
	mce_enable_strict_checking();
#endif
}

/*******************************************************************************
 * Handler to indicate support for System Suspend
 ******************************************************************************/
bool plat_supports_system_suspend(void)
{
	return true;
}
