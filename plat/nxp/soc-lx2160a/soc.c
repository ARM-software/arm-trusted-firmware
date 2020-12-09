/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>

#include <arch.h>
#include <bl31/interrupt_mgmt.h>
#include <caam.h>
#include <cassert.h>
#include <ccn.h>
#include <common/debug.h>
#include <dcfg.h>
#ifdef I2C_INIT
#include <i2c.h>
#endif
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <ls_interconnect.h>
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
#if defined(NXP_SFP_ENABLED)
#include <sfp.h>
#endif

#include <errata.h>
#include <ls_interrupt_mgmt.h>
#include "plat_common.h"
#ifdef NXP_NV_SW_MAINT_LAST_EXEC_DATA
#include <plat_nv_storage.h>
#endif
#ifdef NXP_WARM_BOOT
#include <plat_warm_rst.h>
#endif
#include "platform_def.h"
#include "soc.h"

static struct soc_type soc_list[] =  {
	SOC_ENTRY(LX2160A, LX2160A, 8, 2),
	SOC_ENTRY(LX2080A, LX2080A, 8, 1),
	SOC_ENTRY(LX2120A, LX2120A, 6, 2),
};

static dcfg_init_info_t dcfg_init_data = {
			.g_nxp_dcfg_addr = NXP_DCFG_ADDR,
			.nxp_sysclk_freq = NXP_SYSCLK_FREQ,
			.nxp_ddrclk_freq = NXP_DDRCLK_FREQ,
			.nxp_plat_clk_divider = NXP_PLATFORM_CLK_DIVIDER,
		};
static const unsigned char master_to_6rn_id_map[] = {
	PLAT_6CLUSTER_TO_CCN_ID_MAP
};

static const unsigned char master_to_rn_id_map[] = {
	PLAT_CLUSTER_TO_CCN_ID_MAP
};

CASSERT(ARRAY_SIZE(master_to_rn_id_map) == NUMBER_OF_CLUSTERS,
		assert_invalid_cluster_count_for_ccn_variant);

static const ccn_desc_t plat_six_cluster_ccn_desc = {
	.periphbase = NXP_CCN_ADDR,
	.num_masters = ARRAY_SIZE(master_to_6rn_id_map),
	.master_to_rn_id_map = master_to_6rn_id_map
};

static const ccn_desc_t plat_ccn_desc = {
	.periphbase = NXP_CCN_ADDR,
	.num_masters = ARRAY_SIZE(master_to_rn_id_map),
	.master_to_rn_id_map = master_to_rn_id_map
};

/*******************************************************************************
 * This function returns the number of clusters in the SoC
 ******************************************************************************/
static unsigned int get_num_cluster(void)
{
	const soc_info_t *soc_info = get_soc_info();
	uint32_t num_clusters = NUMBER_OF_CLUSTERS;
	unsigned int i;

	for (i = 0U; i < ARRAY_SIZE(soc_list); i++) {
		if (soc_list[i].personality == soc_info->personality) {
			num_clusters = soc_list[i].num_clusters;
			break;
		}
	}

	VERBOSE("NUM of cluster = 0x%x\n", num_clusters);

	return num_clusters;
}


/******************************************************************************
 * Function returns the base counter frequency
 * after reading the first entry at CNTFID0 (0x20 offset).
 *
 * Function is used by:
 *   1. ARM common code for PSCI management.
 *   2. ARM Generic Timer init.
 *
 *****************************************************************************/
unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int counter_base_frequency;
	/*
	 * Below register specifies the base frequency of the system counter.
	 * As per NXP Board Manuals:
	 * The system counter always works with SYS_REF_CLK/4 frequency clock.
	 *
	 *
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
	.gpio4_base_addr = NXP_GPIO4_ADDR,
};
#endif

static void soc_interconnect_config(void)
{
	unsigned long long val = 0x0U;

	uint32_t num_clusters = get_num_cluster();

	if (num_clusters == 6U) {
		ccn_init(&plat_six_cluster_ccn_desc);
	} else {
		ccn_init(&plat_ccn_desc);
	}

	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 */
	plat_ls_interconnect_enter_coherency(num_clusters);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 13, PCIeRC_RN_I_NODE_ID_OFFSET);
	val |= (1 << 17);
	ccn_write_node_reg(NODE_TYPE_HNI, 13, PCIeRC_RN_I_NODE_ID_OFFSET, val);

	/* PCIe is Connected to RN-I 17 which is connected to HN-I 13. */
	val = ccn_read_node_reg(NODE_TYPE_HNI, 30, PCIeRC_RN_I_NODE_ID_OFFSET);
	val |= (1 << 17);
	ccn_write_node_reg(NODE_TYPE_HNI, 30, PCIeRC_RN_I_NODE_ID_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 13, SA_AUX_CTRL_REG_OFFSET);
	val |= SERIALIZE_DEV_nGnRnE_WRITES;
	ccn_write_node_reg(NODE_TYPE_HNI, 13, SA_AUX_CTRL_REG_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 30, SA_AUX_CTRL_REG_OFFSET);
	val &= ~(ENABLE_RESERVE_BIT53);
	val |= SERIALIZE_DEV_nGnRnE_WRITES;
	ccn_write_node_reg(NODE_TYPE_HNI, 30, SA_AUX_CTRL_REG_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 13, PoS_CONTROL_REG_OFFSET);
	val &= ~(HNI_POS_EN);
	ccn_write_node_reg(NODE_TYPE_HNI, 13, PoS_CONTROL_REG_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 30, PoS_CONTROL_REG_OFFSET);
	val &= ~(HNI_POS_EN);
	ccn_write_node_reg(NODE_TYPE_HNI, 30, PoS_CONTROL_REG_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 13, SA_AUX_CTRL_REG_OFFSET);
	val &= ~(POS_EARLY_WR_COMP_EN);
	ccn_write_node_reg(NODE_TYPE_HNI, 13, SA_AUX_CTRL_REG_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 30, SA_AUX_CTRL_REG_OFFSET);
	val &= ~(POS_EARLY_WR_COMP_EN);
	ccn_write_node_reg(NODE_TYPE_HNI, 30, SA_AUX_CTRL_REG_OFFSET, val);

#if POLICY_PERF_WRIOP
	uint16_t wriop_rni = 0U;

	if (POLICY_PERF_WRIOP == 1) {
		wriop_rni = 7U;
	} else if (POLICY_PERF_WRIOP == 2) {
		wriop_rni = 23U;
	} else {
		ERROR("Incorrect WRIOP selected.\n");
		panic();
	}

	val = ccn_read_node_reg(NODE_TYPE_RNI, wriop_rni,
				SA_AUX_CTRL_REG_OFFSET);
	val |= ENABLE_WUO;
	ccn_write_node_reg(NODE_TYPE_HNI, wriop_rni, SA_AUX_CTRL_REG_OFFSET,
			   val);
#else
	val = ccn_read_node_reg(NODE_TYPE_RNI, 17, SA_AUX_CTRL_REG_OFFSET);
	val |= ENABLE_WUO;
	ccn_write_node_reg(NODE_TYPE_RNI, 17, SA_AUX_CTRL_REG_OFFSET, val);
#endif
}


void soc_preload_setup(void)
{
	dram_regions_info_t *info_dram_regions = get_dram_regions_info();
#if defined(NXP_WARM_BOOT)
	bool warm_reset = is_warm_boot();
#endif
	info_dram_regions->total_dram_size =
#if defined(NXP_WARM_BOOT)
						init_ddr(warm_reset);
#else
						init_ddr();
#endif
}

/*******************************************************************************
 * This function implements soc specific erratas
 * This is called before DDR is initialized or MMU is enabled
 ******************************************************************************/
void soc_early_init(void)
{
	dcfg_init(&dcfg_init_data);
#ifdef POLICY_FUSE_PROVISION
	gpio_init(&gpio_init_data);
	sec_init(NXP_CAAM_ADDR);
#endif
#if LOG_LEVEL > 0
	/* Initialize the console to provide early debug support */
	plat_console_init(NXP_CONSOLE_ADDR,
				NXP_UART_CLK_DIVIDER, NXP_CONSOLE_BAUDRATE);
#endif

	enable_timer_base_to_cluster(NXP_PMU_ADDR);
	soc_interconnect_config();

	enum  boot_device dev = get_boot_dev();
	/* Mark the buffer for SD in OCRAM as non secure.
	 * The buffer is assumed to be at end of OCRAM for
	 * the logic below to calculate TZPC programming
	 */
	if (dev == BOOT_DEVICE_EMMC || dev == BOOT_DEVICE_SDHC2_EMMC) {
		/* Calculate the region in OCRAM which is secure
		 * The buffer for SD needs to be marked non-secure
		 * to allow SD to do DMA operations on it
		 */
		uint32_t secure_region = (NXP_OCRAM_SIZE
						- NXP_SD_BLOCK_BUF_SIZE);
		uint32_t mask = secure_region/TZPC_BLOCK_SIZE;

		mmio_write_32(NXP_OCRAM_TZPC_ADDR, mask);

		/* Add the entry for buffer in MMU Table */
		mmap_add_region(NXP_SD_BLOCK_BUF_ADDR, NXP_SD_BLOCK_BUF_ADDR,
				NXP_SD_BLOCK_BUF_SIZE,
				MT_DEVICE | MT_RW | MT_NS);
	}

#ifdef ERRATA_SOC_A050426
	erratum_a050426();
#endif

#if (TRUSTED_BOARD_BOOT) || defined(POLICY_FUSE_PROVISION)
	sfp_init(NXP_SFP_ADDR);
#endif

#if TRUSTED_BOARD_BOOT
	uint32_t mode;

	/* For secure boot disable SMMU.
	 * Later when platform security policy comes in picture,
	 * this might get modified based on the policy
	 */
	if (check_boot_mode_secure(&mode) == true) {
		bypass_smmu(NXP_SMMU_ADDR);
	}

	/* For Mbedtls currently crypto is not supported via CAAM
	 * enable it when that support is there. In tbbr.mk
	 * the CAAM_INTEG is set as 0.
	 */

#ifndef MBEDTLS_X509
	/* Initialize the crypto accelerator if enabled */
	if (is_sec_enabled() == false)
		INFO("SEC is disabled.\n");
	else
		sec_init(NXP_CAAM_ADDR);
#endif
#endif

	/*
	 * Initialize system level generic timer for Layerscape Socs.
	 */
	delay_timer_init(NXP_TIMER_ADDR);
	i2c_init(NXP_I2C_ADDR);
}

void soc_bl2_prepare_exit(void)
{
#if defined(NXP_SFP_ENABLED) && defined(DISABLE_FUSE_WRITE)
	set_sfp_wr_disable();
#endif
}

/*****************************************************************************
 * This function returns the boot device based on RCW_SRC
 ****************************************************************************/
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


void soc_mem_access(void)
{
	const devdisr5_info_t *devdisr5_info = get_devdisr5_info();
	dram_regions_info_t *info_dram_regions = get_dram_regions_info();
	struct tzc400_reg tzc400_reg_list[MAX_NUM_TZC_REGION];
	int dram_idx, index = 0U;

	for (dram_idx = 0U; dram_idx < info_dram_regions->num_dram_regions;
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

	if (devdisr5_info->ddrc1_present != 0) {
		INFO("DDR Controller 1.\n");
		mem_access_setup(NXP_TZC_ADDR, index,
				tzc400_reg_list);
		mem_access_setup(NXP_TZC3_ADDR, index,
				tzc400_reg_list);
	}
	if (devdisr5_info->ddrc2_present != 0) {
		INFO("DDR Controller 2.\n");
		mem_access_setup(NXP_TZC2_ADDR, index,
				tzc400_reg_list);
		mem_access_setup(NXP_TZC4_ADDR, index,
				tzc400_reg_list);
	}
}

#else
const unsigned char _power_domain_tree_desc[] = {1, 8, 2, 2, 2, 2, 2, 2, 2, 2};

CASSERT(NUMBER_OF_CLUSTERS && NUMBER_OF_CLUSTERS <= 256,
		assert_invalid_lx2160a_cluster_count);

/******************************************************************************
 * This function returns the SoC topology
 ****************************************************************************/

const unsigned char *plat_get_power_domain_tree_desc(void)
{

	return _power_domain_tree_desc;
}

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_ls_get_cluster_core_count(u_register_t mpidr)
{
	return CORES_PER_CLUSTER;
}


void soc_early_platform_setup2(void)
{
	dcfg_init(&dcfg_init_data);
	/*
	 * Initialize system level generic timer for Socs
	 */
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
#ifdef LS_SYS_TIMCTL_BASE
	ls_configure_sys_timer(LS_SYS_TIMCTL_BASE,
			       LS_CONFIG_CNTACR,
			       PLAT_LS_NSTIMER_FRAME_ID);
#endif
}

/*******************************************************************************
 * This function initializes the soc from the BL31 module
 ******************************************************************************/
void soc_init(void)
{
	 /* low-level init of the soc */
	soc_init_start();
	soc_init_percpu();
	_init_global_data();
	_initialize_psci();

	if (ccn_get_part0_id(NXP_CCN_ADDR) != CCN_508_PART0_ID) {
		ERROR("Unrecognized CCN variant detected.");
		ERROR("Only CCN-508 is supported\n");
		panic();
	}

	uint32_t num_clusters = get_num_cluster();

	if (num_clusters == 6U) {
		ccn_init(&plat_six_cluster_ccn_desc);
	} else {
		ccn_init(&plat_ccn_desc);
	}

	plat_ls_interconnect_enter_coherency(num_clusters);

	/* Set platform security policies */
	_set_platform_security();

	 /* make sure any parallel init tasks are finished */
	soc_init_finish();

	/* Initialize the crypto accelerator if enabled */
	if (is_sec_enabled() == false) {
		INFO("SEC is disabled.\n");
	} else {
		sec_init(NXP_CAAM_ADDR);
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
#endif
