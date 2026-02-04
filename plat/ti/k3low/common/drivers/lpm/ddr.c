/*
 * Copyright (c) 2024-2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <board_def.h>
#include <ddr.h>
#include <lib/mmio.h>

/* DDR Subsystem configuration base address and field values */
#define DDRSS0_SSCFG_BASE				(0xF300000UL)
#define CSL_EMIF_SSCFG_V2A_CTL_REG			(0x00000020U)
#define SDRAM_IDX					0xFU
#define REGION_IDX					0xFU

/* DDR Control system base address and field values */
#define DDRSS0_CTRL_BASE				(0xF308000UL)
/* Register block CTL (CTL_0-CTL_422) offset, total and field values */
#define CTLCFG_DENALI_CTL_(x)				((x) << 2U)
#define NUM_DDR_CTL_REG					423U
#define DENALI_CTL_00_DATA				0x00000B00U
/* Register block PI (PI_0-PI_344) offset, total and field values */
#define DDRSS_PI_REGISTER_BLOCK_OFFS			0x2000U
#define CTLCFG_DENALI_PI_(x) \
	(((x) << 2U) + DDRSS_PI_REGISTER_BLOCK_OFFS)
#define NUM_DDR_PI_REG					345U
#define DENALI_PI_00_DATA				0x00000B00U
/* Register block Data_Slice_0 (or PHY Register block offset PHY_0-PHY_125) */
#define DDRSS_DATA_SLICE_0_REGISTER_BLOCK_OFFS		0x4000U
#define CTLCFG_DENALI_PHY_(x) \
	(((x) << 2U) + DDRSS_DATA_SLICE_0_REGISTER_BLOCK_OFFS)
#define NUM_DDR_DATA_0_REG				126U
/* Register block Data_Slice_1 (PHY_256-PHY_381) offset and total */
#define DDRSS_DATA_SLICE_1_REGISTER_BLOCK_OFFS		0x4400U
#define NUM_DDR_DATA_1_REG				126U
/* Register block Address_Slice_0 (PHY_512-PHY_554) offset and total  */
#define DDRSS_ADDRESS_SLICE_0_REGISTER_BLOCK_OFFS	0x4800U
#define NUM_DDR_ADDR_0_REG				43U
/* Register block Address_Slice_1 (PHY_768-PHY_810) offset and total  */
#define DDRSS_ADDRESS_SLICE_1_REGISTER_BLOCK_OFFS	0x4c00U
#define NUM_DDR_ADDR_1_REG				43U
/* Register block Address_Slice_2 (PHY_1024-PHY_1066) offset and total */
#define DDRSS_ADDRESS_SLICE_2_REGISTER_BLOCK_OFFS	0x5000U
#define NUM_DDR_ADDR_2_REG				43U
/* Register block core (PHY_1280-PHY_1405) offset and total */
#define DDRSS_PHY_CORE_REGISTER_BLOCK_OFFS		0x5400U
#define NUM_DDR_PHY_REG					126U
#define NUM_ALL_PHY_REG \
	(NUM_DDR_DATA_0_REG + NUM_DDR_DATA_1_REG + \
	 NUM_DDR_ADDR_0_REG + NUM_DDR_ADDR_1_REG + \
	 NUM_DDR_ADDR_2_REG + NUM_DDR_PHY_REG)
#define NUM_ALL_DDR_REG \
	(NUM_DDR_CTL_REG + NUM_DDR_PI_REG + NUM_ALL_PHY_REG)
#define LP_MODE_LONG_SELF_REFRESH			0x31U
#define LP_MODE_LONG_SELF_REFRESH_PHY_CTRL		0x51U
#define LP_MODE_LONG_SELF_REFRESH_EXIT			0x2U
#define LPDDR4_DRAM_CLASS_REG_VALUE			0xBU

/* WKUP CTRL MMR Base and register configuration values */
#define WKUP_CTRL_MMR_SEC_4_BASE			(0x43040000UL)
#define CHNG_DDR4_FSP_REQ				(0x0U)
#define CHNG_DDR4_FSP_ACK				(0x4U)
#define DDR4_FSP_CLKCHNG_REQ				(0x80U)
#define DDR4_FSP_CLKCHNG_ACK				(0x84U)
#define DDR32SS_PMCTRL					(0x1000U)

/* MAIN PLL MMR Base */
#define MAIN_PLL_MMR_BASE				(0x04060000UL)

typedef struct emif_handle_s {
	uint64_t		   ss_cfg_base_addr;
	uint64_t		   ctl_cfg_base_addr;
} emif_handle_t;

__wkupsramdata emif_handle_t Emifhandle;
__wkupsramdata uint32_t ddrss_save_restore[NUM_ALL_DDR_REG];

/**
 * @brief Poll for init completion
 *
 * @param h Pointer to EMIF handle structure
 */
__wkupsramfunc static void poll_for_init_completion(struct emif_handle_s *h)
{
	/* Poll for PI Init completion */
	while (((mmio_read_32(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(83))) & 0x1) != 0x1) {
	}
	/* Poll for CTL Init completion */
	while (((mmio_read_32(h->ctl_cfg_base_addr +
			      CTLCFG_DENALI_CTL_(342))) &
		0x02000000) != 0x02000000) {
	}
}

/**
 * @brief Write to a specific field in an MMR
 *
 * @param mmr_address Address of the MMR register
 * @param field_value Value to write to the field
 * @param width Bit width of the field
 * @param leftshift Left shift position of the field
 */
__wkupsramfunc static void write_mmr_field(uint32_t mmr_address, uint32_t field_value,
				     uint32_t width, uint32_t leftshift)
{
	uint32_t val;
	uint32_t mask;

	val = mmio_read_32(mmr_address);
	mask = (((1 << width) - 1) << leftshift);
	mask = (~(mask));
	val &= mask;
	val |= (field_value << leftshift);
	mmio_write_32(mmr_address, val);
}

/**
 * @brief Enable DDR data retention mode
 */
__wkupsramfunc static void enable_ddr_data_retention(void)
{
	uint32_t val;

	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x6U, 4U, 0U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x1U, 1U, 31U);
	val = mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL));
	while (val != ((1U << 31) | 0x6U)) {
		val = mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL));
	}
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x0U, 1U, 31U);
}

/**
 * @brief Configure SDRAM region index
 *
 * @param h Pointer to EMIF handle structure
 * @param sdram_idx SDRAM index value
 * @param region_idx Region index value
 */
__wkupsramfunc static void configure_sdram_region_idx(struct emif_handle_s *h,
						uint32_t sdram_idx,
						uint32_t region_idx)
{
	uint32_t rd_val;

	rd_val = mmio_read_32(h->ss_cfg_base_addr + CSL_EMIF_SSCFG_V2A_CTL_REG);
	rd_val = (rd_val & 0xFFFFFC00);
	rd_val = rd_val | (sdram_idx << 5) | (region_idx);
	/* Programming the region_idx and sdram_idx fields for address mapping */
	mmio_write_32((h->ss_cfg_base_addr + CSL_EMIF_SSCFG_V2A_CTL_REG), rd_val);
}

/**
 * @brief Configure SDRAM region index with default values
 *
 * @param h Pointer to EMIF handle structure
 */
__wkupsramfunc static void sdram_region_idx_cfg(struct emif_handle_s *h)
{
	/* Programming the region_idx and sdram_idx fields for address mapping */
	configure_sdram_region_idx(h, SDRAM_IDX, REGION_IDX);
}

__wkupsramfunc int32_t k3low_put_ddr_in_rtc_lpm(void)
{

	uint32_t req, req_type;
	uint32_t lp_status = 0U;

	/* disable auto entry / exit */
	write_mmr_field(DDRSS0_CTRL_BASE + CTLCFG_DENALI_CTL_(167), 0U, 4U, 16U);
	write_mmr_field(DDRSS0_CTRL_BASE + CTLCFG_DENALI_CTL_(167), 0U, 4U, 24U);
	write_mmr_field(DDRSS0_CTRL_BASE + CTLCFG_DENALI_CTL_(276), 1U, 1U, 24U);
	write_mmr_field(DDRSS0_CTRL_BASE + CTLCFG_DENALI_CTL_(277), 1U, 1U, 8U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_REQ), 0x0U, 2U, 0U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_REQ), 0x1U, 1U, 8U);
	req = (mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR4_FSP_CLKCHNG_REQ)) & 0x80U);
	while (req == 0x0U) {
		req = (mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR4_FSP_CLKCHNG_REQ)) & 0x80U);
	}
	req_type = (req & 0x03U);
	if (req_type == 0U) {
		write_mmr_field((MAIN_PLL_MMR_BASE + (0U * 0x1000U) +
				 ((2U * 0x4U) + 0x80U)), 0x4FU, 7U, 0U);
	} else {
		return -1;
	}
	mmio_write_32(((WKUP_CTRL_MMR_SEC_4_BASE + DDR4_FSP_CLKCHNG_ACK)),
		      0x1U);
	while (((mmio_read_32(((WKUP_CTRL_MMR_SEC_4_BASE +
				DDR4_FSP_CLKCHNG_REQ)))) &
		0x80U) == 0x80U) {
	}
	mmio_write_32(((WKUP_CTRL_MMR_SEC_4_BASE + DDR4_FSP_CLKCHNG_ACK)), 0x0U);
	req = (mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_ACK)) & 0x80U);
	while (req == 0x0U) {
		req = (mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_ACK)) & 0x80U);
	}
	req_type = (mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_ACK)) & 0x01U);
	if (req_type == 0U) {
	} else {
		return -2;
	}
	req = mmio_read_32(WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_REQ);
	req &= ~0x100U;
	mmio_write_32((WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_REQ), req);
	/* Program Self Refresh mode */
	write_mmr_field(DDRSS0_CTRL_BASE + CTLCFG_DENALI_CTL_(158),
			LP_MODE_LONG_SELF_REFRESH, 7U, 8U);
	/* Poll for Self Refresh Mode change */
	write_mmr_field(DDRSS0_CTRL_BASE + CTLCFG_DENALI_CTL_(353),
			0x0U, 16U, 16U);
	lp_status = (mmio_read_32(DDRSS0_CTRL_BASE +
				  CTLCFG_DENALI_CTL_(337)) & 0x10000U);
	while (lp_status != 0x10000U) {
		lp_status = (mmio_read_32(DDRSS0_CTRL_BASE +
					  CTLCFG_DENALI_CTL_(337)) &
			     0x10000U);
	}
	write_mmr_field(DDRSS0_CTRL_BASE + CTLCFG_DENALI_CTL_(345),
			0x1U, 16U, 16U);
	lp_status = ((mmio_read_32(DDRSS0_CTRL_BASE +
				   CTLCFG_DENALI_CTL_(167)) &
		      0x7F00U) >> 8U);
	while (lp_status != 0x4EU) {
		lp_status = ((mmio_read_32(DDRSS0_CTRL_BASE +
					   CTLCFG_DENALI_CTL_(167)) &
			      0x7F00U) >> 8U);
	}

	/*
	 * Enable DDR data retention by writing b0110 to WKUP_CTRL_MMR.
	 * DDR32SS_PMCTRL.data_retention
	 */
	enable_ddr_data_retention();

	return 0;
}

/**
 * @brief Select EMIF instance and configure base addresses
 *
 * @param h Pointer to EMIF handle structure
 */
__wkupsramfunc static void emif_instance_select(struct emif_handle_s *h)
{
	/* Config Addresses */
	h->ss_cfg_base_addr = (uint64_t)(DDRSS0_SSCFG_BASE);
	h->ctl_cfg_base_addr = (uint64_t)(DDRSS0_CTRL_BASE);
}

/**
 * @brief Start PI and CTL initialization
 *
 * @param h Pointer to EMIF handle structure
 */
__wkupsramfunc static void start_PI_CTL_init(struct emif_handle_s *h)
{
	uint32_t wr_init_val;

	wr_init_val = ((LPDDR4_DRAM_CLASS_REG_VALUE << 8) | 0x1);
	/* Set START bit in register for PI module */
	mmio_write_32(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(0), wr_init_val);
	volatile int i = 0;

	for (i = 0; i < 1000; i++) {
	}
	/* Set START bit in register for controller */
	mmio_write_32(h->ctl_cfg_base_addr + CTLCFG_DENALI_CTL_(0), wr_init_val);
}

/**
 * @brief Save DDR registers to memory
 *
 * @param h Pointer to EMIF handle structure
 */
__wkupsramfunc static void save_ddr_registers(struct emif_handle_s *h)
{
	int i, j;

	/* DDRSS Memory Base */
	uint32_t DDR_CTL_REG_BASE = h->ctl_cfg_base_addr;
	uint32_t DDR_PI_REG_BASE = (h->ctl_cfg_base_addr) +
				   DDRSS_PI_REGISTER_BLOCK_OFFS;
	uint32_t DDR_PHY_DATA_SLICE_0_REG_BASE =
		(h->ctl_cfg_base_addr) + DDRSS_DATA_SLICE_0_REGISTER_BLOCK_OFFS;
	uint32_t DDR_PHY_DATA_SLICE_1_REG_BASE =
		(h->ctl_cfg_base_addr) + DDRSS_DATA_SLICE_1_REGISTER_BLOCK_OFFS;
	uint32_t DDR_PHY_ADDR_SLICE_0_REG_BASE =
		(h->ctl_cfg_base_addr) + DDRSS_ADDRESS_SLICE_0_REGISTER_BLOCK_OFFS;
	uint32_t DDR_PHY_ADDR_SLICE_1_REG_BASE =
		(h->ctl_cfg_base_addr) + DDRSS_ADDRESS_SLICE_1_REGISTER_BLOCK_OFFS;
	uint32_t DDR_PHY_ADDR_SLICE_2_REG_BASE =
		(h->ctl_cfg_base_addr) + DDRSS_ADDRESS_SLICE_2_REGISTER_BLOCK_OFFS;
	uint32_t DDR_PHY_CORE_REG_BASE =
		(h->ctl_cfg_base_addr) + DDRSS_PHY_CORE_REGISTER_BLOCK_OFFS;

	j = 0;
	for (i = 0; i < NUM_DDR_CTL_REG; i++, j++) {
		ddrss_save_restore[j] = mmio_read_32(DDR_CTL_REG_BASE + i * 4);
	}
	for (i = 0; i < NUM_DDR_PI_REG; i++, j++) {
		ddrss_save_restore[j] = mmio_read_32(DDR_PI_REG_BASE + i * 4);
	}
	for (i = 0; i < NUM_DDR_DATA_0_REG; i++, j++) {
		ddrss_save_restore[j] = mmio_read_32(DDR_PHY_DATA_SLICE_0_REG_BASE + i * 4);
	}
	for (i = 0; i < NUM_DDR_DATA_1_REG; i++, j++) {
		ddrss_save_restore[j] = mmio_read_32(DDR_PHY_DATA_SLICE_1_REG_BASE + i * 4);
	}
	for (i = 0; i < NUM_DDR_ADDR_0_REG; i++, j++) {
		ddrss_save_restore[j] = mmio_read_32(DDR_PHY_ADDR_SLICE_0_REG_BASE + i * 4);
	}
	for (i = 0; i < NUM_DDR_ADDR_1_REG; i++, j++) {
		ddrss_save_restore[j] = mmio_read_32(DDR_PHY_ADDR_SLICE_1_REG_BASE + i * 4);
	}
	for (i = 0; i < NUM_DDR_ADDR_2_REG; i++, j++) {
		ddrss_save_restore[j] = mmio_read_32(DDR_PHY_ADDR_SLICE_2_REG_BASE + i * 4);
	}
	for (i = 0; i < NUM_DDR_PHY_REG; i++, j++) {
		ddrss_save_restore[j] = mmio_read_32(DDR_PHY_CORE_REG_BASE + i * 4);
	}
}

/**
 * @brief Restore DDR registers from memory
 *
 * @param h Pointer to EMIF handle structure
 */
__wkupsramfunc static void restore_ddr_registers(struct emif_handle_s *h)
{
	int j;

	/* DDRSS Memory Base */
	uint32_t DDR_CTL_REG_BASE = h->ctl_cfg_base_addr;
	uint32_t DDR_PI_REG_BASE = (h->ctl_cfg_base_addr) +
				   DDRSS_PI_REGISTER_BLOCK_OFFS;
	uint32_t DDR_PHY_DATA_SLICE_0_REG_BASE =
		(h->ctl_cfg_base_addr) + DDRSS_DATA_SLICE_0_REGISTER_BLOCK_OFFS;
	uint32_t DDR_PHY_DATA_SLICE_1_REG_BASE =
		(h->ctl_cfg_base_addr) + DDRSS_DATA_SLICE_1_REGISTER_BLOCK_OFFS;
	uint32_t DDR_PHY_ADDR_SLICE_0_REG_BASE =
		(h->ctl_cfg_base_addr) + DDRSS_ADDRESS_SLICE_0_REGISTER_BLOCK_OFFS;
	uint32_t DDR_PHY_ADDR_SLICE_1_REG_BASE =
		(h->ctl_cfg_base_addr) + DDRSS_ADDRESS_SLICE_1_REGISTER_BLOCK_OFFS;
	uint32_t DDR_PHY_ADDR_SLICE_2_REG_BASE =
		(h->ctl_cfg_base_addr) + DDRSS_ADDRESS_SLICE_2_REGISTER_BLOCK_OFFS;
	uint32_t DDR_PHY_CORE_REG_BASE =
		(h->ctl_cfg_base_addr) + DDRSS_PHY_CORE_REGISTER_BLOCK_OFFS;

	mmio_write_32(DDR_CTL_REG_BASE + CTLCFG_DENALI_CTL_(0), DENALI_CTL_00_DATA);
	/* Skip the first CTL register write */
	j = 1;
	for (int i = 1; i < NUM_DDR_CTL_REG; i++, j++) {
		mmio_write_32(DDR_CTL_REG_BASE + i * 4, ddrss_save_restore[j]);
	}
	mmio_write_32(DDR_CTL_REG_BASE + CTLCFG_DENALI_PI_(0), DENALI_PI_00_DATA);
	/* Skip the first PI register write */
	j++;
	for (int i = 1; i < NUM_DDR_PI_REG; i++, j++) {
		mmio_write_32(DDR_PI_REG_BASE + i * 4, ddrss_save_restore[j]);
	}
	for (int i = 0; i < NUM_DDR_DATA_0_REG; i++, j++) {
		mmio_write_32(DDR_PHY_DATA_SLICE_0_REG_BASE + i * 4, ddrss_save_restore[j]);
	}
	for (int i = 0; i < NUM_DDR_DATA_1_REG; i++, j++) {
		mmio_write_32(DDR_PHY_DATA_SLICE_1_REG_BASE + i * 4, ddrss_save_restore[j]);
	}
	for (int i = 0; i < NUM_DDR_ADDR_0_REG; i++, j++) {
		mmio_write_32(DDR_PHY_ADDR_SLICE_0_REG_BASE + i * 4, ddrss_save_restore[j]);
	}
	for (int i = 0; i < NUM_DDR_ADDR_1_REG; i++, j++) {
		mmio_write_32(DDR_PHY_ADDR_SLICE_1_REG_BASE + i * 4, ddrss_save_restore[j]);
	}
	for (int i = 0; i < NUM_DDR_ADDR_2_REG; i++, j++) {
		mmio_write_32(DDR_PHY_ADDR_SLICE_2_REG_BASE + i * 4, ddrss_save_restore[j]);
	}
	for (int i = 0; i < NUM_DDR_PHY_REG; i++, j++) {
		mmio_write_32(DDR_PHY_CORE_REG_BASE + i * 4, ddrss_save_restore[j]);
	}
}

/**
 * @brief DDR deep sleep resume sequence
 *
 * @param h Pointer to EMIF handle structure
 */
__wkupsramfunc static void ddr_deep_sleep_resume_sequence(struct emif_handle_s *h)
{
	uint32_t lp_status;

	/* Restore the default values from the reg_config file */
	sdram_region_idx_cfg(h);

	/* Write back the copied registers */
	restore_ddr_registers(h);

	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PHY_(1306), 0x1, 1, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(4), 0x0, 1, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_CTL_(20), 0x1, 1, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_CTL_(21), 0x1, 1, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(138), 0x1, 1, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_CTL_(106), 0x0, 1, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(134), 0x1, 1, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(138), 0x1, 1, 8);

	/* De-asserting data retention pin and wake Control bits */
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x0U, 1U, 31U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x0U, 4U, 0U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x1U, 1U, 31U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x0U, 4U, 0U);
	lp_status = mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL));
	while (lp_status != ((1U << 31U))) {
		lp_status = mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL));
	}
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x0, 1, 31);

	/* Start Initialization [PI_START=1 and START=1] */
	start_PI_CTL_init(h);

	/* Wait for INIT_DONE interrupt */
	poll_for_init_completion(h);

	/* dfi_phymstr_cs_state_r = 0, dfi_phymstr_state_sel_r = 0, and
	 * PI_SELF_REFRESH_EN = 1 to enable self-refresh during training
	 * since PI does not send refresh commands during CA leveling
	 */
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(6), 0x1, 1, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(6), 0x1, 1, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(133), 0x1, 1, 24);

	/* LPDDR4 PI sequence */
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(23), 0x1, 1, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(33), 0x1, 1, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(33), 0x1, 1, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(67), 0x1, 1, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(181), 0x0, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(181), 0x1, 2, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(182), 0x1, 2, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(188), 0x0, 2, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(189), 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(189), 0x1, 2, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(188), 0x0, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(189), 0x0, 2, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(189), 0x1, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(190), 0x0, 2, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(191), 0x0, 2, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(192), 0x0, 2, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(191), 0x0, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(192), 0x0, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(193), 0x0, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(191), 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(192), 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(193), 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(191), 0x0, 2, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(192), 0x0, 2, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(193), 0x0, 2, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(199), 0x0, 2, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(199), 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(199), 0x0, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(223), 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(226), 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CTLCFG_DENALI_PI_(229), 0x1, 2, 8);
}

/**
 * @brief Enter LPM self refresh mode
 *
 * @param h Pointer to EMIF handle structure
 */
__wkupsramfunc static void enter_lpm_self_refresh(struct emif_handle_s *h)
{
	uint32_t lp_status = 0;
	/* Program Self Refresh mode */
	mmio_write_32(h->ctl_cfg_base_addr + CTLCFG_DENALI_CTL_(158),
		      (LP_MODE_LONG_SELF_REFRESH << 8));

	/* Poll for Self Refresh Mode change */
	while (lp_status != 0x4E) {
		lp_status = ((mmio_read_32(h->ctl_cfg_base_addr +
					    CTLCFG_DENALI_CTL_(167)) &
			      0x7F00) >> 8);
	}
}

__wkupsramfunc int32_t k3low_ddr_deep_sleep_suspend_sequence(void)
{
	/* Save DDR register context in WKUP SRAM, Put the DDR in self refresh */
	emif_instance_select(&Emifhandle);
	save_ddr_registers(&Emifhandle);
	enter_lpm_self_refresh(&Emifhandle);
	/* Enable DDR data retention by writing b0110 to
	 * WKUP_CTRL_MMR.DDR32SS_PMCTRL.data_retention
	 */
	enable_ddr_data_retention();

	return 0;
}

__wkupsramfunc int32_t k3low_ddr_deep_sleep_resume_sequence(void)
{
	/* Restore DDR Controller Context & Take DDR out of self refresh,
	 * retaining of DDR & Remove DDR data retention
	 */
	emif_instance_select(&Emifhandle);
	ddr_deep_sleep_resume_sequence(&Emifhandle);

	return 0;
}
