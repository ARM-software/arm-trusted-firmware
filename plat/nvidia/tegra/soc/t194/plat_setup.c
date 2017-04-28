/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
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
#include <plat/common/platform.h>
#include <tegra_def.h>
#include <tegra_platform.h>
#include <tegra_private.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

DEFINE_RENAME_SYSREG_RW_FUNCS(l2ctlr_el1, L2CTLR_EL1)
extern uint64_t tegra_enable_l2_ecc_parity_prot;

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

/*
 * Table of regions to map using the MMU.
 */
static const mmap_region_t tegra_mmap[] = {
	MAP_REGION_FLAT(TEGRA_MISC_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_TSA_BASE, 0x20000, /* 128KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_MC_STREAMID_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_MC_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_UARTA_BASE, 0x20000, /* 128KB - UART A, B*/
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_UARTC_BASE, 0x20000, /* 128KB - UART C, G */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_UARTD_BASE, 0x30000, /* 192KB - UART D, E, F */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_FUSE_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_GICD_BASE, 0x20000, /* 128KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SE0_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_PKA1_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_RNG1_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_CAR_RESET_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_PMC_BASE, 0x40000, /* 256KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SCRATCH_BASE, 0x10000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_MMCRAB_BASE, 0x60000, /* 384KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SMMU0_BASE, 0x1000000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SMMU1_BASE, 0x1000000, /* 64KB */
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TEGRA_SMMU2_BASE, 0x1000000, /* 64KB */
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
 * Handler to get the System Counter Frequency
 ******************************************************************************/
unsigned int plat_get_syscnt_freq2(void)
{
	return 31250000;
}

/*******************************************************************************
 * Maximum supported UART controllers
 ******************************************************************************/
#define TEGRA186_MAX_UART_PORTS		7

/*******************************************************************************
 * This variable holds the UART port base addresses
 ******************************************************************************/
static uint32_t tegra186_uart_addresses[TEGRA186_MAX_UART_PORTS + 1] = {
	0,	/* undefined - treated as an error case */
	TEGRA_UARTA_BASE,
	TEGRA_UARTB_BASE,
	TEGRA_UARTC_BASE,
	TEGRA_UARTD_BASE,
	TEGRA_UARTE_BASE,
	TEGRA_UARTF_BASE,
	TEGRA_UARTG_BASE,
};

/*******************************************************************************
 * Retrieve the UART controller base to be used as the console
 ******************************************************************************/
uint32_t plat_get_console_from_id(int id)
{
	if (id > TEGRA186_MAX_UART_PORTS)
		return 0;

	return tegra186_uart_addresses[id];
}

/* represent chip-version as concatenation of major (15:12), minor (11:8) and subrev (7:0) */
#define TEGRA186_VER_A02P	0x1201

/*******************************************************************************
 * Handler for early platform setup
 ******************************************************************************/
void plat_early_platform_setup(void)
{
	int impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;
	uint32_t chip_subrev, val;

	/* sanity check MCE firmware compatibility */
	mce_verify_firmware_version();

	/*
	 * Enable ECC and Parity Protection for Cortex-A57 CPUs
	 * for Tegra A02p SKUs
	 */
	if (impl != DENVER_IMPL) {

		/* get the major, minor and sub-version values */
		chip_subrev = mmio_read_32(TEGRA_FUSE_BASE + OPT_SUBREVISION) &
			      SUBREVISION_MASK;

		/* prepare chip version number */
		val = (tegra_get_chipid_major() << 12) |
		      (tegra_get_chipid_minor() << 8) |
		       chip_subrev;

		/* enable L2 ECC for Tegra186 A02P and beyond */
		if (val >= TEGRA186_VER_A02P) {

			val = read_l2ctlr_el1();
			val |= L2_ECC_PARITY_PROTECTION_BIT;
			write_l2ctlr_el1(val);

			/*
			 * Set the flag to enable ECC/Parity Protection
			 * when we exit System Suspend or Cluster Powerdn
			 */
			tegra_enable_l2_ecc_parity_prot = 1;
		}
	}
}

/* Secure IRQs for Tegra186 */
static const irq_sec_cfg_t tegra186_sec_irqs[] = {
	[0] = {
		TEGRA186_BPMP_WDT_IRQ,
		TEGRA186_SEC_IRQ_TARGET_MASK,
		INTR_TYPE_EL3,
	},
	[1] = {
		TEGRA186_BPMP_WDT_IRQ,
		TEGRA186_SEC_IRQ_TARGET_MASK,
		INTR_TYPE_EL3,
	},
	[2] = {
		TEGRA186_SPE_WDT_IRQ,
		TEGRA186_SEC_IRQ_TARGET_MASK,
		INTR_TYPE_EL3,
	},
	[3] = {
		TEGRA186_SCE_WDT_IRQ,
		TEGRA186_SEC_IRQ_TARGET_MASK,
		INTR_TYPE_EL3,
	},
	[4] = {
		TEGRA186_TOP_WDT_IRQ,
		TEGRA186_SEC_IRQ_TARGET_MASK,
		INTR_TYPE_EL3,
	},
	[5] = {
		TEGRA186_AON_WDT_IRQ,
		TEGRA186_SEC_IRQ_TARGET_MASK,
		INTR_TYPE_EL3,
	},
};

/*******************************************************************************
 * Initialize the GIC and SGIs
 ******************************************************************************/
void plat_gic_setup(void)
{
	tegra_gic_setup(tegra186_sec_irqs,
		sizeof(tegra186_sec_irqs) / sizeof(tegra186_sec_irqs[0]));

	/*
	 * Initialize the FIQ handler only if the platform supports any
	 * FIQ interrupt sources.
	 */
	if (sizeof(tegra186_sec_irqs) > 0)
		tegra_fiq_handler_setup();
}

/*******************************************************************************
 * Return pointer to the BL31 params from previous bootloader
 ******************************************************************************/
struct tegra_bl31_params *plat_get_bl31_params(void)
{
	uint32_t val;

	val = mmio_read_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV53_LO);

	return (struct tegra_bl31_params *)(uintptr_t)val;
}

/*******************************************************************************
 * Return pointer to the BL31 platform params from previous bootloader
 ******************************************************************************/
plat_params_from_bl2_t *plat_get_bl31_plat_params(void)
{
	uint32_t val;

	val = mmio_read_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV53_HI);

	return (plat_params_from_bl2_t *)(uintptr_t)val;
}
