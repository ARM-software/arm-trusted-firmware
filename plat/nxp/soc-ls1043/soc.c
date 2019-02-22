/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <plat_common.h>
#include <io.h>
#include <csu.h>

#include <arch.h>
#include <assert.h>
#include <common/debug.h>
#include <drivers/arm/cci.h>
#include <drivers/arm/tzc380.h>
#include <endian.h>
#include <errata.h>
#include <lib/cassert.h>
#include <lib/mmio.h>

#include <plat_tzc380.h>

const unsigned char _power_domain_tree_desc[] = {1, 1, 4};

CASSERT(NUMBER_OF_CLUSTERS && NUMBER_OF_CLUSTERS <= 256,
		assert_invalid_ls1043_cluster_count);

/******************************************************************************
 * This function returns the SoC topology
 *****************************************************************************/

const unsigned char *plat_get_power_domain_tree_desc(void)
{

	return _power_domain_tree_desc;
}

/******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 *****************************************************************************/
unsigned int plat_ls_get_cluster_core_count(u_register_t mpidr)
{
	return CORES_PER_CLUSTER;
}

/*******************************************************************************
 * This function returns the number of clusters in the SoC
 ******************************************************************************/
static unsigned int get_num_cluster(void)
{
	return NUMBER_OF_CLUSTERS;
}

/******************************************************************************
 *****************************************************************************/
static void bypass_smmu(void)
{
	uint32_t val;

	val = ((mmio_read_32(SMMU_SCR0) | SCR0_CLIENTPD_MASK)) &
						~(SCR0_USFCFG_MASK);
	mmio_write_32(SMMU_SCR0, val);

	val = (mmio_read_32(SMMU_NSCR0) | SCR0_CLIENTPD_MASK) &
						~(SCR0_USFCFG_MASK);
	mmio_write_32(SMMU_NSCR0, val);
}

/******************************************************************************
 * This function implements soc specific erratas
 * This is called before DDR is initialized or MMU is enabled
 *****************************************************************************/
void soc_early_init(void)
{
	uint32_t mode;

	set_base_freq_CNTFID0();

	/* Enable snooping on SEC read and write transactions */
	scfg_setbits32((void *)(NXP_SCFG_ADDR + SCFG_SNPCNFGCR_OFFSET),
			SCFG_SNPCNFGCR_SECRDSNP | SCFG_SNPCNFGCR_SECWRSNP);

	/* For secure boot disable SMMU.
	 * Later when platform security policy comes in picture,
	 * this might get modified based on the policy
	 */
	if (check_boot_mode_secure(&mode) == true)
		bypass_smmu();

	/* Add Erratums here */
	erratum_a008850_early();
	erratum_a009660();
	erratum_a010539();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	cci_init(NXP_CCI_ADDR, cci_map, ARRAY_SIZE(cci_map));

	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 */
	plat_interconnect_enter_coherency(get_num_cluster());
}

/******************************************************************************
 * For LS1043a rev1.0, GIC base address align with 4k.
 * For LS1043a rev1.1, if DCFG_GIC400_ALIGN[GIC_ADDR_BIT]
 * is set, GIC base address align with 4K, or else align
 * with 64k.
 *****************************************************************************/
void get_gic_offset(uint32_t *gicc_base, uint32_t *gicd_base)
{
	uint32_t *ccsr_svr = (uint32_t *)(NXP_DCFG_ADDR + DCFG_SVR_OFFSET);
	uint32_t *gic_align = (uint32_t *)(NXP_SCFG_ADDR +
					   SCFG_GIC400_ADDR_ALIGN_OFFSET);
	uint32_t val;

	val = be32toh(mmio_read_32((uintptr_t)ccsr_svr));

	if ((val & 0xff) == REV1_1) {
		val = be32toh(mmio_read_32((uintptr_t)gic_align));
		if (val & (1 << GIC_ADDR_BIT)) {
			*gicc_base = NXP_GICC_4K_ADDR;
			*gicd_base = NXP_GICD_4K_ADDR;
		} else {
			*gicc_base = NXP_GICC_64K_ADDR;
			*gicd_base = NXP_GICD_64K_ADDR;
		}
	} else {
		*gicc_base = NXP_GICC_4K_ADDR;
		*gicd_base = NXP_GICD_4K_ADDR;
	}
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

	/*
	 * Initialize the interconnect during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	cci_init(NXP_CCI_ADDR, cci_map, ARRAY_SIZE(cci_map));

	/*
	 * Enable coherency in interconnect for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this but we can't
	 * assume so. No harm in executing this code twice.
	 */
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr()));

	/* Init CSU to enable non-secure access to peripherals */
	enable_layerscape_ns_access();

	/* make sure any parallel init tasks are finished */
	soc_init_finish();

}

/******************************************************************************
 * Function to return the SoC SYS CLK
 *****************************************************************************/
unsigned int get_sys_clk(void)
{
	return NXP_SYSCLK_FREQ;
}

/******************************************************************************
 * Function to set the base counter frequency at the
 * the first entry of the Frequency Mode Table,
 * at CNTFID0 (0x20 offset).
 *
 * Set the value of the pirmary core register cntfrq_el0.
 *****************************************************************************/
void set_base_freq_CNTFID0(void)
{
	/*
	 * Below register specifies the base frequency of the system counter.
	 * As per NXP Board Manuals:
	 * The system counter always works with SYS_REF_CLK/4 frequency clock.
	 *
	 */
	unsigned int counter_base_frequency = get_sys_clk()/4;

	/* Setting the frequency in the Frequency modes table.
	 *
	 * Note: The value for ls1046ardb board at this offset
	 *       is not RW as stated. This offset have the
	 *       fixed value of 100000400 Hz.
	 *
	 * The below code line has no effect.
	 * Keeping it for other platforms where it has effect.
	 */
	mmio_write_32(NXP_TIMER_ADDR + CNTFID_OFF, counter_base_frequency);

	write_cntfrq_el0(counter_base_frequency);
}

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
	 *
	 * Not reading the frequency from Frequency modes table.
	 *
	 * Note: The value for ls1046ardb board at this offset is not RW and
	 *       have the fixed value of 100000400 Hz.
	 */
	counter_base_frequency = get_sys_clk()/4;

	return counter_base_frequency;
}

/*******************************************************************************
 * Configure memory access permissions
 *   - Region 0 with no access;
 *   - Region 1 to 4 with ?????
 ******************************************************************************/
static void mem_access_setup(uintptr_t base)

{
	int reg_id = 0;
	unsigned int attr_value;

	INFO("Configuring TZASC-380\n");

	tzc380_init(base);

	tzc380_set_action(TZC_ACTION_NONE);

	for (reg_id = 0; reg_id < MAX_NUM_TZC_REGION; reg_id++) {

		attr_value = (tzc380_reg_list[reg_id].secure) |
			((tzc380_reg_list[reg_id].sub_mask & 0xFF) << 8) |
			((tzc380_reg_list[reg_id].size & 0x3F) << 1) |
			(tzc380_reg_list[reg_id].enabled & 0x1);

		tzc380_configure_region(reg_id, tzc380_reg_list[reg_id].addr,
					attr_value);

	}

	/*
	 * Setting the Error Exception for now when access violation happened.
	 * Later Error Interrupts will be handled.
	 */
	tzc380_set_action(TZC_ACTION_ERR);

	/* Configure CSU secure access register to disable TZASC bypass mux */
	mmio_write_32((uintptr_t)(NXP_CSU_ADDR +
				CSU_SEC_ACCESS_REG_OFFSET),
			bswap32(TZASC_BYPASS_MUX_DISABLE));
}

/*****************************************************************************
 * This function sets up access permissions on memory regions
 ****************************************************************************/
void soc_mem_access(void)
{
	/* Secure Regions on DRAM1 only.*/
	int i = 0;

	/* index 0 is reserved for TZC Region-0 */
	int index = 1;

	struct dram_rgn_info *dram_regions_info = get_dram_regions_info();

	if ((NXP_SP_SHRD_DRAM_SIZE + NXP_SECURE_DRAM_SIZE) !=
		TZC380_REGION_GUARD_SIZE) {
		NOTICE("Error: TZC380 config is in-correct for the board.\n");
		assert(0);
	}

	if (dram_regions_info->region[i].size != 0) {

		/* Region 1: Secure Region on DRAM 1 for  2MB out of  2MB,
		 * excluding 0 sub-region(=256KB).
		 */
		tzc380_reg_list[index].addr =
					dram_regions_info->region[i].addr
					+ dram_regions_info->region[i].size;
		index++;

		/* Region 2: Secure Region on DRAM 1 for 54MB out of 64MB,
		 * excluding 1 sub-rgion(=8MB) of 8MB.
		 */
		tzc380_reg_list[index].addr =
					dram_regions_info->region[i].addr
					+ dram_regions_info->region[i].size
					+ NXP_SP_SHRD_DRAM_SIZE;
		index++;

		/* Region 3: Secure Region on DRAM 1 for  6MB out of  8MB,
		 * excluding 2 sub-rgion(=1MB) of 2MB.
		 */
		tzc380_reg_list[index].addr =
					dram_regions_info->region[i].addr
					+ dram_regions_info->region[i].size
					+ NXP_SECURE_DRAM_SIZE;
		index++;
	}

	mem_access_setup(NXP_TZC_ADDR);
}

/*****************************************************************************
 * This function returns the boot device based on RCW_SRC
 ****************************************************************************/
enum boot_device get_boot_dev(void)
{
	enum boot_device src = BOOT_DEVICE_NONE;
	uint32_t porsr1;
	uint32_t rcw_src, val;

	porsr1 = read_reg_porsr1();

	rcw_src = (porsr1 & PORSR1_RCW_MASK) >> PORSR1_RCW_SHIFT;
	/*
	 * In case of some errata this value might get zeroized
	 * Use the saved porsr1 value in that case
	 */
	if (!rcw_src) {
		porsr1 = read_saved_porsr1();
		rcw_src = (porsr1 & PORSR1_RCW_MASK) >> PORSR1_RCW_SHIFT;
	}

	val = rcw_src & RCW_SRC_NAND_MASK;

	if (val == RCW_SRC_NAND_VAL) {
		val = rcw_src & NAND_RESERVED_MASK;
		if ((val != NAND_RESERVED_1) && (val != NAND_RESERVED_2)) {
			src = BOOT_DEVICE_IFC_NAND;
			INFO("RCW BOOT SRC is IFC NAND\n");
		}
	} else {
		/* RCW SRC NOR */
		val = rcw_src & RCW_SRC_NOR_MASK;
		if (val == NOR_8B_VAL || val == NOR_16B_VAL) {
			src = BOOT_DEVICE_IFC_NOR;
			INFO("RCW BOOT SRC is IFC NOR\n");
		} else {
			switch (rcw_src) {
			case QSPI_VAL1:
			case QSPI_VAL2:
				src = BOOT_DEVICE_QSPI;
				INFO("RCW BOOT SRC is QSPI\n");
				break;
			case SD_VAL:
				src = BOOT_DEVICE_EMMC;
				INFO("RCW BOOT SRC is SD/EMMC\n");
				break;
			default:
				src = BOOT_DEVICE_NONE;
			}
		}
	}

	return src;
}
