/*
 * Copyright 2018-2021, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <endian.h>

#include <arch.h>
#include <caam.h>
#include <cassert.h>
#include <cci.h>
#include <common/debug.h>
#include <dcfg.h>
#include <i2c.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <ls_interconnect.h>
#include <mmio.h>
#ifdef POLICY_FUSE_PROVISION
#include <nxp_gpio.h>
#endif
#if TRUSTED_BOARD_BOOT
#include <nxp_smmu.h>
#endif
#include <nxp_timer.h>
#include <plat_console.h>
#include <plat_gic.h>
#include <plat_tzc400.h>
#include <pmu.h>
#include <scfg.h>
#if defined(NXP_SFP_ENABLED)
#include <sfp.h>
#endif
#if TRUSTED_BOARD_BOOT
#include <snvs.h>
#endif

#include <errata.h>
#ifdef CONFIG_OCRAM_ECC_EN
#include <ocram.h>
#endif
#include "plat_common.h"
#include "platform_def.h"
#include "soc.h"

static dcfg_init_info_t dcfg_init_data = {
	.g_nxp_dcfg_addr = NXP_DCFG_ADDR,
	.nxp_sysclk_freq = NXP_SYSCLK_FREQ,
	.nxp_ddrclk_freq = NXP_DDRCLK_FREQ,
	.nxp_plat_clk_divider = NXP_PLATFORM_CLK_DIVIDER,
};

static struct soc_type soc_list[] =  {
	SOC_ENTRY(LS1017AN, LS1017AN, 1, 1),
	SOC_ENTRY(LS1017AE, LS1017AE, 1, 1),
	SOC_ENTRY(LS1018AN, LS1018AN, 1, 1),
	SOC_ENTRY(LS1018AE, LS1018AE, 1, 1),
	SOC_ENTRY(LS1027AN, LS1027AN, 1, 2),
	SOC_ENTRY(LS1027AE, LS1027AE, 1, 2),
	SOC_ENTRY(LS1028AN, LS1028AN, 1, 2),
	SOC_ENTRY(LS1028AE, LS1028AE, 1, 2),
};

CASSERT(NUMBER_OF_CLUSTERS && NUMBER_OF_CLUSTERS <= 256,
	assert_invalid_ls1028a_cluster_count);

/*
 * Function returns the base counter frequency
 * after reading the first entry at CNTFID0 (0x20 offset).
 *
 * Function is used by:
 *   1. ARM common code for PSCI management.
 *   2. ARM Generic Timer init.
 *
 */
unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int counter_base_frequency;
	/*
	 * Below register specifies the base frequency of the system counter.
	 * As per NXP Board Manuals:
	 * The system counter always works with SYS_REF_CLK/4 frequency clock.
	 */
	counter_base_frequency = mmio_read_32(NXP_TIMER_ADDR + CNTFID_OFF);

	return counter_base_frequency;
}

#ifdef IMAGE_BL2

#ifdef POLICY_FUSE_PROVISION
static gpio_init_info_t gpio_init_data = {
	.gpio1_base_addr = NXP_GPIO1_ADDR,
	.gpio2_base_addr = NXP_GPIO2_ADDR,
	.gpio3_base_addr = NXP_GPIO3_ADDR,
};
#endif

void soc_preload_setup(void)
{
}

void soc_early_init(void)
{
	uint8_t num_clusters, cores_per_cluster;

#if TRUSTED_BOARD_BOOT
	snvs_init(NXP_SNVS_ADDR);
#endif

#ifdef CONFIG_OCRAM_ECC_EN
	ocram_init(NXP_OCRAM_ADDR, NXP_OCRAM_SIZE);
#endif
	dcfg_init(&dcfg_init_data);
	enable_timer_base_to_cluster(NXP_PMU_ADDR);
	enable_core_tb(NXP_PMU_ADDR);
	dram_regions_info_t *dram_regions_info = get_dram_regions_info();

#ifdef POLICY_FUSE_PROVISION
	gpio_init(&gpio_init_data);
	sec_init(NXP_CAAM_ADDR);
#endif

#if LOG_LEVEL > 0
	/* Initialize the console to provide early debug support */
	plat_console_init(NXP_CONSOLE_ADDR,
				NXP_UART_CLK_DIVIDER, NXP_CONSOLE_BAUDRATE);
#endif
	enum  boot_device dev = get_boot_dev();
	/*
	 * Mark the buffer for SD in OCRAM as non secure.
	 * The buffer is assumed to be at end of OCRAM for
	 * the logic below to calculate TZPC programming
	 */
	if (dev == BOOT_DEVICE_EMMC || dev == BOOT_DEVICE_SDHC2_EMMC) {
		/*
		 * Calculate the region in OCRAM which is secure
		 * The buffer for SD needs to be marked non-secure
		 * to allow SD to do DMA operations on it
		 */
		uint32_t secure_region = (NXP_OCRAM_SIZE - NXP_SD_BLOCK_BUF_SIZE);
		uint32_t mask = secure_region/TZPC_BLOCK_SIZE;

		mmio_write_32(NXP_OCRAM_TZPC_ADDR, mask);

		/* Add the entry for buffer in MMU Table */
		mmap_add_region(NXP_SD_BLOCK_BUF_ADDR, NXP_SD_BLOCK_BUF_ADDR,
				NXP_SD_BLOCK_BUF_SIZE, MT_DEVICE | MT_RW | MT_NS);
	}

#if TRUSTED_BOARD_BOOT
	uint32_t mode;

	sfp_init(NXP_SFP_ADDR);

	/*
	 * For secure boot disable SMMU.
	 * Later when platform security policy comes in picture,
	 * this might get modified based on the policy
	 */
	if (check_boot_mode_secure(&mode) == true) {
		bypass_smmu(NXP_SMMU_ADDR);
	}

	/*
	 * For Mbedtls currently crypto is not supported via CAAM
	 * enable it when that support is there. In tbbr.mk
	 * the CAAM_INTEG is set as 0.
	 */
#ifndef MBEDTLS_X509
	/* Initialize the crypto accelerator if enabled */
	if (is_sec_enabled()) {
		sec_init(NXP_CAAM_ADDR);
	} else {
		INFO("SEC is disabled.\n");
	}
#endif
#endif

	/* Set eDDRTQ for DDR performance */
	scfg_setbits32((void *)(NXP_SCFG_ADDR + 0x210), 0x1f1f1f1f);

	soc_errata();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	cci_init(NXP_CCI_ADDR, cci_map, ARRAY_SIZE(cci_map));

	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 */
	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters, &cores_per_cluster);
	plat_ls_interconnect_enter_coherency(num_clusters);

	delay_timer_init(NXP_TIMER_ADDR);
	i2c_init(NXP_I2C_ADDR);
	dram_regions_info->total_dram_size = init_ddr();
}

void soc_bl2_prepare_exit(void)
{
#if defined(NXP_SFP_ENABLED) && defined(DISABLE_FUSE_WRITE)
	set_sfp_wr_disable();
#endif
}

/*
 * This function returns the boot device based on RCW_SRC
 */
enum boot_device get_boot_dev(void)
{
	enum boot_device src = BOOT_DEVICE_NONE;
	uint32_t porsr1;
	uint32_t rcw_src;

	porsr1 = read_reg_porsr1();

	rcw_src = (porsr1 & PORSR1_RCW_MASK) >> PORSR1_RCW_SHIFT;
	switch (rcw_src) {
	case FLEXSPI_NOR:
		src = BOOT_DEVICE_FLEXSPI_NOR;
		INFO("RCW BOOT SRC is FLEXSPI NOR\n");
		break;
	case FLEXSPI_NAND2K_VAL:
	case FLEXSPI_NAND4K_VAL:
		INFO("RCW BOOT SRC is FLEXSPI NAND\n");
		src = BOOT_DEVICE_FLEXSPI_NAND;
		break;
	case SDHC1_VAL:
		src = BOOT_DEVICE_EMMC;
		INFO("RCW BOOT SRC is SD\n");
		break;
	case SDHC2_VAL:
		src = BOOT_DEVICE_SDHC2_EMMC;
		INFO("RCW BOOT SRC is EMMC\n");
		break;
	default:
		break;
	}

	return src;
}

/*
 * This function sets up access permissions on memory regions
 ****************************************************************************/
void soc_mem_access(void)
{
	dram_regions_info_t *info_dram_regions = get_dram_regions_info();
	struct tzc400_reg tzc400_reg_list[MAX_NUM_TZC_REGION];
	int dram_idx = 0;
	/* index 0 is reserved for region-0 */
	int index = 1;

	for (dram_idx = 0; dram_idx < info_dram_regions->num_dram_regions;
	     dram_idx++) {
		if (info_dram_regions->region[dram_idx].size == 0) {
			ERROR("DDR init failure, or");
			ERROR("DRAM regions not populated correctly.\n");
			break;
		}

		index = populate_tzc400_reg_list(tzc400_reg_list,
				dram_idx, index,
				info_dram_regions->region[dram_idx].addr,
				info_dram_regions->region[dram_idx].size,
				NXP_SECURE_DRAM_SIZE, NXP_SP_SHRD_DRAM_SIZE);
	}

	mem_access_setup(NXP_TZC_ADDR, index, tzc400_reg_list);
}

#else

static unsigned char _power_domain_tree_desc[NUMBER_OF_CLUSTERS + 2];
/*
 * This function dynamically constructs the topology according to
 *  SoC Flavor and returns it.
 */
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	uint8_t num_clusters, cores_per_cluster;
	unsigned int i;

	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters, &cores_per_cluster);
	/*
	 * The highest level is the system level. The next level is constituted
	 * by clusters and then cores in clusters.
	 */
	_power_domain_tree_desc[0] = 1;
	_power_domain_tree_desc[1] = num_clusters;

	for (i = 0; i < _power_domain_tree_desc[1]; i++)
		_power_domain_tree_desc[i + 2] = cores_per_cluster;

	return _power_domain_tree_desc;
}

/*
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 */
unsigned int plat_ls_get_cluster_core_count(u_register_t mpidr)
{
	uint8_t num_clusters, cores_per_cluster;

	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters, &cores_per_cluster);
	return num_clusters;
}

void soc_early_platform_setup2(void)
{
	dcfg_init(&dcfg_init_data);
	/* Initialize system level generic timer for Socs */
	delay_timer_init(NXP_TIMER_ADDR);

#if LOG_LEVEL > 0
	/* Initialize the console to provide early debug support */
	plat_console_init(NXP_CONSOLE_ADDR,
				NXP_UART_CLK_DIVIDER, NXP_CONSOLE_BAUDRATE);
#endif
}

void soc_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	static uintptr_t target_mask_array[PLATFORM_CORE_COUNT];
	static interrupt_prop_t ls_interrupt_props[] = {
		PLAT_LS_G1S_IRQ_PROPS(INTR_GROUP1S),
		PLAT_LS_G0_IRQ_PROPS(INTR_GROUP0)
	};

	plat_ls_gic_driver_init(NXP_GICD_ADDR, NXP_GICR_ADDR,
				PLATFORM_CORE_COUNT,
				ls_interrupt_props,
				ARRAY_SIZE(ls_interrupt_props),
				target_mask_array,
				plat_core_pos);

	plat_ls_gic_init();
	enable_init_timer();
}

/* This function initializes the soc from the BL31 module */
void soc_init(void)
{
	uint8_t num_clusters, cores_per_cluster;

	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters, &cores_per_cluster);

	/* Low-level init of the soc */
	soc_init_lowlevel();
	_init_global_data();
	soc_init_percpu();
	_initialize_psci();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	cci_init(NXP_CCI_ADDR, cci_map, ARRAY_SIZE(cci_map));

	/* Enable Interconnect coherency for the primary CPU's cluster. */
	plat_ls_interconnect_enter_coherency(num_clusters);

	/* Set platform security policies */
	_set_platform_security();

	/* Init SEC Engine which will be used by SiP */
	if (is_sec_enabled()) {
		sec_init(NXP_CAAM_ADDR);
	} else {
		INFO("SEC is disabled.\n");
	}
}

#ifdef NXP_WDOG_RESTART
static uint64_t wdog_interrupt_handler(uint32_t id, uint32_t flags,
					  void *handle, void *cookie)
{
	uint8_t data = WDOG_RESET_FLAG;

	wr_nv_app_data(WDT_RESET_FLAG_OFFSET,
			(uint8_t *)&data, sizeof(data));

	mmio_write_32(NXP_RST_ADDR + RSTCNTL_OFFSET, SW_RST_REQ_INIT);

	return 0;
}
#endif

void soc_runtime_setup(void)
{
#ifdef NXP_WDOG_RESTART
	request_intr_type_el3(BL31_NS_WDOG_WS1, wdog_interrupt_handler);
#endif
}

/* This function returns the total number of cores in the SoC. */
unsigned int get_tot_num_cores(void)
{
	uint8_t num_clusters, cores_per_cluster;

	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters, &cores_per_cluster);
	return (num_clusters * cores_per_cluster);
}

/* This function returns the PMU IDLE Cluster mask. */
unsigned int get_pmu_idle_cluster_mask(void)
{
	uint8_t num_clusters, cores_per_cluster;

	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters, &cores_per_cluster);
	return ((1 << num_clusters) - 2);
}

/* This function returns the PMU Flush Cluster mask. */
unsigned int get_pmu_flush_cluster_mask(void)
{
	uint8_t num_clusters, cores_per_cluster;

	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters, &cores_per_cluster);
	return ((1 << num_clusters) - 2);
}

/* This function returns the PMU idle core mask. */
unsigned int get_pmu_idle_core_mask(void)
{
	return ((1 << get_tot_num_cores()) - 2);
}

/* Function to return the SoC SYS CLK */
unsigned int get_sys_clk(void)
{
	return NXP_SYSCLK_FREQ;
}
#endif
