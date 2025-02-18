/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <board_def.h>
#include <ddr.h>
#include <lib/mmio.h>
#include <lpm_trace.h>

#define DENALI_CTL_167__SFR_OFFS			0x29c
#define DENALI_CTL_276__SFR_OFFS			0x450
#define DENALI_CTL_277__SFR_OFFS			0x454
#define DENALI_CTL_158__SFR_OFFS			0x278
#define DENALI_CTL_353__SFR_OFFS			0x584
#define DENALI_CTL_345__SFR_OFFS			0x564
#define DENALI_CTL_337__SFR_OFFS			0x544
#define LP_MODE_LONG_SELF_REFRESH			0x31
#define LP_MODE_LONG_SELF_REFRESH_PHY_CTRL		0x51
#define LP_MODE_LONG_SELF_REFRESH_EXIT			0x2
#define LPDDR4_DRAM_CLASS_REG_VALUE			0xB

#define CSL_DDR16SS0_REGS_SS_CFG_SSCFG_BASE		(0xf300000UL)
#define WKUP_CTRL_MMR_SEC_4_BASE			(0x43040000UL)
#define MAIN_PLL_MMR_BASE				(0x04060000UL)
#define DDRSS0_CTRL_BASE				(0xF308000UL)

#define CHNG_DDR4_FSP_REQ				(0x0U)
#define CHNG_DDR4_FSP_ACK				(0x4U)
#define DDR4_FSP_CLKCHNG_REQ				(0x80U)
#define DDR4_FSP_CLKCHNG_ACK				(0x84U)
#define DDR32SS_PMCTRL					(0x1000U)
#define CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL		(0x00001000U)

#define NUM_DDR_CTL_REG					423 /* 0-422 */
#define NUM_DDR_PI_REG					345 /* 0-344 */
#define NUM_DDR_DATA_0_REG				126 /* 0-125 */
#define NUM_DDR_DATA_1_REG				126 /* 256-381 */
#define NUM_DDR_ADDR_0_REG				43 /* 512-554 */
#define NUM_DDR_ADDR_1_REG				43 /* 768-810 */
#define NUM_DDR_ADDR_2_REG				43 /* 1024-1066 */
#define NUM_DDR_PHY_REG					126 /* 1280-1405 */
#define NUM_ALL_DDR_REG (NUM_DDR_CTL_REG + NUM_DDR_PI_REG + NUM_DDR_DATA_0_REG + NUM_DDR_DATA_1_REG + NUM_DDR_ADDR_0_REG + NUM_DDR_ADDR_1_REG + NUM_DDR_ADDR_2_REG + NUM_DDR_PHY_REG)
#define DENALI_CTL_00_DATA				0x00000B00
#define DENALI_PI_00_DATA				0x00000B00

#define DDRSS_PHY_Core_REGISTER_BLOCK__OFFS		0x5400
/* Macros for register block Address_Slice_2 */
#define DDRSS_Address_Slice_2_REGISTER_BLOCK__OFFS	0x5000
/* Macros for register block Address_Slice_1 */
#define DDRSS_Address_Slice_1_REGISTER_BLOCK__OFFS	0x4c00
/* Macros for register block Address_Slice_0 */
#define DDRSS_Address_Slice_0_REGISTER_BLOCK__OFFS	0x4800
/* Macros for register block Data_Slice_1 */
#define DDRSS_Data_Slice_1_REGISTER_BLOCK__OFFS		0x4400
/* Macros for register block Data_Slice_0 */
#define DDRSS_Data_Slice_0_REGISTER_BLOCK__OFFS		0x4000
/* Macros for register block PI */
#define DDRSS_PI_REGISTER_BLOCK__OFFS			0x2000
#define DENALI_PI_83__SFR_OFFS				0x14c
#define DENALI_CTL_342__SFR_OFFS			0x558
#define DENALI_PI_0__SFR_OFFS				0x0
#define DENALI_CTL_0__SFR_OFFS				0x0
#define SDRAM_IDX					0xf
#define REGION_IDX					0xf

#define CSL_EMIF_SSCFG_V2A_CTL_REG			(0x00000020U)
#define CSL_EMIF_CTLCFG_DENALI_PHY_1306			(0x00005468U)
#define CSL_EMIF_CTLCFG_DENALI_CTL_21			(0x00000054U)
#define CSL_EMIF_CTLCFG_DENALI_CTL_20			(0x00000050U)
#define CSL_EMIF_CTLCFG_DENALI_CTL_106			(0x000001A8U)
#define CSL_EMIF_CTLCFG_DENALI_PI_4			(0x00002010U)
#define CSL_EMIF_CTLCFG_DENALI_PI_6			(0x00002018U)
#define CSL_EMIF_CTLCFG_DENALI_PI_23			(0x0000205CU)
#define CSL_EMIF_CTLCFG_DENALI_PI_33			(0x00002084U)
#define CSL_EMIF_CTLCFG_DENALI_PI_67			(0x0000210CU)
#define CSL_EMIF_CTLCFG_DENALI_PI_133			(0x00002214U)
#define CSL_EMIF_CTLCFG_DENALI_PI_134			(0x00002218U)
#define CSL_EMIF_CTLCFG_DENALI_PI_138			(0x00002228U)
#define CSL_EMIF_CTLCFG_DENALI_PI_181			(0x000022D4U)
#define CSL_EMIF_CTLCFG_DENALI_PI_182			(0x000022D8U)
#define CSL_EMIF_CTLCFG_DENALI_PI_188			(0x000022F0U)
#define CSL_EMIF_CTLCFG_DENALI_PI_189			(0x000022F4U)
#define CSL_EMIF_CTLCFG_DENALI_PI_190			(0x000022F8U)
#define CSL_EMIF_CTLCFG_DENALI_PI_191			(0x000022FCU)
#define CSL_EMIF_CTLCFG_DENALI_PI_192			(0x00002300U)
#define CSL_EMIF_CTLCFG_DENALI_PI_193			(0x00002304U)
#define CSL_EMIF_CTLCFG_DENALI_PI_199			(0x0000231CU)
#define CSL_EMIF_CTLCFG_DENALI_PI_223			(0x0000237CU)
#define CSL_EMIF_CTLCFG_DENALI_PI_226			(0x00002388U)
#define CSL_EMIF_CTLCFG_DENALI_PI_229			(0x00002394U)
typedef struct emif_handle_s {
	uint64_t		   ss_cfg_base_addr;
	uint64_t		   ctl_cfg_base_addr;
} emif_handle_t;

__wkupsramdata emif_handle_t Emifhandle;
__wkupsramdata uint32_t ddrss_save_restore[NUM_ALL_DDR_REG];

/* poll_for_init_completion - Sub-routine to poll for init completion */
__wkupsramfunc void poll_for_init_completion(struct emif_handle_s *h)
{
	/* Poll for PI Init completion */
	while (((mmio_read_32(h->ctl_cfg_base_addr + DDRSS_PI_REGISTER_BLOCK__OFFS + DENALI_PI_83__SFR_OFFS)) & 0x1) != 0x1) {
	}
	/* Poll for CTL Init completion */
	while (((mmio_read_32(h->ctl_cfg_base_addr + DENALI_CTL_342__SFR_OFFS)) & 0x02000000) != 0x02000000) {
	}
}

/* Write to a specific field in an MMR. */
__wkupsramfunc void write_mmr_field(uint32_t mmr_address, uint32_t field_value, uint32_t width, uint32_t leftshift)
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

__wkupsramfunc void configure_sdram_region_idx(struct emif_handle_s *h, uint32_t sdram_idx, uint32_t region_idx)
{
	uint32_t rd_val;

	rd_val = mmio_read_32(h->ss_cfg_base_addr + CSL_EMIF_SSCFG_V2A_CTL_REG);
	rd_val = (rd_val & 0xFFFFFC00);
	rd_val = rd_val | (sdram_idx << 5) | (region_idx);
	/* Programming the region_idx and sdram_idx fields for address mapping */
	mmio_write_32((h->ss_cfg_base_addr + CSL_EMIF_SSCFG_V2A_CTL_REG), rd_val);
}

__wkupsramfunc void sdram_region_idx_cfg(struct emif_handle_s *h)
{
	/* Programming the region_idx and sdram_idx fields for address mapping */
	configure_sdram_region_idx(h, SDRAM_IDX, REGION_IDX);
}

__wkupsramfunc void put_ddr_in_sr(bool enable)
{
	if (enable) {
		uint32_t lp_status = 0U;

		/* Program Self Refresh mode  */
		write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_158__SFR_OFFS, LP_MODE_LONG_SELF_REFRESH_PHY_CTRL, 7U, 8U);
		/* Poll for Self Refresh Mode change */
		write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_353__SFR_OFFS, 0x0U, 16U, 16U);
		lp_status = (mmio_read_32(DDRSS0_CTRL_BASE + DENALI_CTL_337__SFR_OFFS) & 0x10000U);
		while (lp_status != 0x10000U) {
			lp_status = (mmio_read_32(DDRSS0_CTRL_BASE + DENALI_CTL_337__SFR_OFFS) & 0x10000U);
		}
		/* clear low power complete */
		write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_345__SFR_OFFS, 0x1U, 16U, 16U);
		lp_status = ((mmio_read_32(DDRSS0_CTRL_BASE + DENALI_CTL_167__SFR_OFFS) & 0x7F00U) >> 8U);
		while (lp_status != 0x4FU) {
			lp_status = ((mmio_read_32(DDRSS0_CTRL_BASE + DENALI_CTL_167__SFR_OFFS) & 0x7F00U) >> 8U);
		}
	} else {
		uint32_t lp_status = 0U;

		/* Program Self Refresh mode */
		write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_158__SFR_OFFS, LP_MODE_LONG_SELF_REFRESH_EXIT, 7U, 8U);
		/* Poll for Self Refresh Mode change */
		write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_353__SFR_OFFS, 0x0U, 16U, 16U);
		lp_status = (mmio_read_32(DDRSS0_CTRL_BASE + DENALI_CTL_337__SFR_OFFS) & 0x10000U);
		while (lp_status != 0x10000U) {
			lp_status = (mmio_read_32(DDRSS0_CTRL_BASE + DENALI_CTL_337__SFR_OFFS) & 0x10000U);
		}
		write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_345__SFR_OFFS, 0x1U, 16U, 16U);
		lp_status = ((mmio_read_32(DDRSS0_CTRL_BASE + DENALI_CTL_167__SFR_OFFS) & 0x7F00U) >> 8U);
		while (lp_status != 0x40U) {
			lp_status = ((mmio_read_32(DDRSS0_CTRL_BASE + DENALI_CTL_167__SFR_OFFS) & 0x7F00U) >> 8U);
		}
	}
}

__wkupsramfunc int32_t put_ddr_in_rtc_lpm(void)
{

	uint32_t req, req_type;
	uint32_t lp_status = 0U;

	/* disable auto entry / exit */
	write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_167__SFR_OFFS, 0U, 4U, 16U);
	write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_167__SFR_OFFS, 0U, 4U, 24U);
	write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_276__SFR_OFFS, 1U, 1U, 24U);
	write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_277__SFR_OFFS, 1U, 1U, 8U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_REQ), 0x0U, 2U, 0U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_REQ), 0x1U, 1U, 8U);
	req = (mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR4_FSP_CLKCHNG_REQ)) & 0x80U);
	while (req == 0x0U) {
		req = (mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR4_FSP_CLKCHNG_REQ)) & 0x80U);
	}
	req_type = (req & 0x03U);
	if (req_type == 0U) {
		write_mmr_field((MAIN_PLL_MMR_BASE + (0U * 0x1000U) + ((2U * 0x4U) + 0x80U)), 0x4FU, 7U, 0U);
	} else {
		lpm_seq_trace_fail(0xF3);
		return -1;
	}
	mmio_write_32(((WKUP_CTRL_MMR_SEC_4_BASE + DDR4_FSP_CLKCHNG_ACK)), 0x1U);
	while (((mmio_read_32(((WKUP_CTRL_MMR_SEC_4_BASE + DDR4_FSP_CLKCHNG_REQ)))) & 0x80U) == 0x80U) {
	}
	mmio_write_32(((WKUP_CTRL_MMR_SEC_4_BASE + DDR4_FSP_CLKCHNG_ACK)), 0x0U);
	req = (mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_ACK)) & 0x80U);
	while (req == 0x0U) {
		req = (mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_ACK)) & 0x80U);
	}
	req_type = (mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_ACK)) & 0x01U);
	if (req_type == 0U) {
	} else {
		lpm_seq_trace_fail(0xF4);
		return -2;
	}
	req = mmio_read_32(WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_REQ);
	req &= ~0x100U;
	mmio_write_32((WKUP_CTRL_MMR_SEC_4_BASE + CHNG_DDR4_FSP_REQ), req);
	/* Program Self Refresh mode */
	write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_158__SFR_OFFS, LP_MODE_LONG_SELF_REFRESH, 7U, 8U);
	/* Poll for Self Refresh Mode change */
	write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_353__SFR_OFFS, 0x0U, 16U, 16U);
	lp_status = (mmio_read_32(DDRSS0_CTRL_BASE + DENALI_CTL_337__SFR_OFFS) & 0x10000U);
	while (lp_status != 0x10000U) {
		lp_status = (mmio_read_32(DDRSS0_CTRL_BASE + DENALI_CTL_337__SFR_OFFS) & 0x10000U);
	}
	write_mmr_field(DDRSS0_CTRL_BASE + DENALI_CTL_345__SFR_OFFS, 0x1U, 16U, 16U);
	lp_status = ((mmio_read_32(DDRSS0_CTRL_BASE + DENALI_CTL_167__SFR_OFFS) & 0x7F00U) >> 8U);
	while (lp_status != 0x4EU) {
		lp_status = ((mmio_read_32(DDRSS0_CTRL_BASE + DENALI_CTL_167__SFR_OFFS) & 0x7F00U) >> 8U);
	}

	/* Enable DDR data retention by writing b0110 to WKUP_CTRL_MMR. DDR32SS_PMCTRL.data_retention */
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x6U, 4U, 0U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x1U, 1U, 31U);
	lp_status = mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL));
	while (lp_status != ((1U << 31) | 0x6U)) {
		lp_status = mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL));
	}
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x0U, 1U, 31U);

	return 0;
}

__wkupsramfunc void emif_instance_select(struct emif_handle_s *h)
{
	/* Config Addresses */
	h->ss_cfg_base_addr = (uint64_t)(CSL_DDR16SS0_REGS_SS_CFG_SSCFG_BASE);
	h->ctl_cfg_base_addr = (uint64_t)(DDRSS0_CTRL_BASE);
}

__wkupsramfunc void start_PI_CTL_init(struct emif_handle_s *h)
{
	uint32_t wr_init_val;

	wr_init_val = ((LPDDR4_DRAM_CLASS_REG_VALUE << 8) | 0x1);
	/* Set START bit in register for PI module */
	mmio_write_32(h->ctl_cfg_base_addr + DDRSS_PI_REGISTER_BLOCK__OFFS + DENALI_PI_0__SFR_OFFS, wr_init_val);
	volatile int i = 0;

	for (i = 0; i < 1000; i++) {
	}
	/* Set START bit in register for controller */
	mmio_write_32(h->ctl_cfg_base_addr + DENALI_CTL_0__SFR_OFFS, wr_init_val);
}

__wkupsramfunc void save_ddr_registers(struct emif_handle_s *h)
{
	int i, j;

	/* DDRSS Memory Base */
	uint32_t DDR_CTL_REG_BASE = h->ctl_cfg_base_addr;
	uint32_t DDR_PI_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_PI_REGISTER_BLOCK__OFFS;
	uint32_t DDR_PHY_DATA_SLICE_0_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_Data_Slice_0_REGISTER_BLOCK__OFFS;
	uint32_t DDR_PHY_DATA_SLICE_1_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_Data_Slice_1_REGISTER_BLOCK__OFFS;
	uint32_t DDR_PHY_ADDR_SLICE_0_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_Address_Slice_0_REGISTER_BLOCK__OFFS;
	uint32_t DDR_PHY_ADDR_SLICE_1_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_Address_Slice_1_REGISTER_BLOCK__OFFS;
	uint32_t DDR_PHY_ADDR_SLICE_2_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_Address_Slice_2_REGISTER_BLOCK__OFFS;
	uint32_t DDR_PHY_CORE_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_PHY_Core_REGISTER_BLOCK__OFFS;

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

__wkupsramfunc void restore_ddr_registers(struct emif_handle_s *h)
{
	int j;

	/* DDRSS Memory Base */
	uint32_t DDR_CTL_REG_BASE = h->ctl_cfg_base_addr;
	uint32_t DDR_PI_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_PI_REGISTER_BLOCK__OFFS;
	uint32_t DDR_PHY_DATA_SLICE_0_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_Data_Slice_0_REGISTER_BLOCK__OFFS;
	uint32_t DDR_PHY_DATA_SLICE_1_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_Data_Slice_1_REGISTER_BLOCK__OFFS;
	uint32_t DDR_PHY_ADDR_SLICE_0_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_Address_Slice_0_REGISTER_BLOCK__OFFS;
	uint32_t DDR_PHY_ADDR_SLICE_1_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_Address_Slice_1_REGISTER_BLOCK__OFFS;
	uint32_t DDR_PHY_ADDR_SLICE_2_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_Address_Slice_2_REGISTER_BLOCK__OFFS;
	uint32_t DDR_PHY_CORE_REG_BASE = (h->ctl_cfg_base_addr) + DDRSS_PHY_Core_REGISTER_BLOCK__OFFS;

	mmio_write_32(DDR_CTL_REG_BASE + DENALI_CTL_0__SFR_OFFS, DENALI_CTL_00_DATA);
	/* Skip the first CTL register write */
	j = 1;
	for (int i = 1; i < NUM_DDR_CTL_REG; i++, j++) {
		mmio_write_32(DDR_CTL_REG_BASE + i * 4, ddrss_save_restore[j]);
	}
	mmio_write_32(DDR_PI_REG_BASE + DENALI_PI_0__SFR_OFFS, DENALI_PI_00_DATA);
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

__wkupsramfunc void ddr_save_restore_exit_sequence(struct emif_handle_s *h)
{

	uint32_t lp_status;

	/* Restore the default values from the reg_config file */
	sdram_region_idx_cfg(h);

	/* Write back the copied registers */
	restore_ddr_registers(h);

	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PHY_1306, 0x1, 1, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_4, 0x0, 1, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_CTL_20, 0x1, 1, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_CTL_21, 0x1, 1, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_138, 0x1, 1, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_CTL_106, 0x0, 1, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_134, 0x1, 1, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_138, 0x1, 1, 8);

	/* De-asserting data retention pin and wake Control bits */
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL), 0x0U, 1U, 31U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL), 0x0U, 4U, 0U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL), 0x1U, 1U, 31U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL), 0x0U, 4U, 0U);
	lp_status = mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL));
	while (lp_status != ((1U << 31U))) {
		lp_status = mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL));
	}
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL), 0x0, 1, 31);

	/* Start Initialization [PI_START=1 and START=1] */
	start_PI_CTL_init(h);

	/* Wait for INIT_DONE interrupt */
	poll_for_init_completion(h);

	/* dfi_phymstr_cs_state_r = 0, dfi_phymstr_state_sel_r = 0, and PI_SELF_REFRESH_EN = 1 to enable self-refresh
	 * during training since PI does not send refresh commands during CA leveling
	 */
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_6, 0x1, 1, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_6, 0x1, 1, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_133, 0x1, 1, 24);

	/* LPDDR4 PI sequence */
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_23, 0x1, 1, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_33, 0x1, 1, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_33, 0x1, 1, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_67, 0x1, 1, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_181, 0x0, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_181, 0x1, 2, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_182, 0x1, 2, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_188, 0x0, 2, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_189, 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_189, 0x1, 2, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_188, 0x0, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_189, 0x0, 2, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_189, 0x1, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_190, 0x0, 2, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_191, 0x0, 2, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_192, 0x0, 2, 24);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_191, 0x0, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_192, 0x0, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_193, 0x0, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_191, 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_192, 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_193, 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_191, 0x0, 2, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_192, 0x0, 2, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_193, 0x0, 2, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_199, 0x0, 2, 0);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_199, 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_199, 0x0, 2, 16);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_223, 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_226, 0x0, 2, 8);
	write_mmr_field(h->ctl_cfg_base_addr + CSL_EMIF_CTLCFG_DENALI_PI_229, 0x1, 2, 8);
}

__wkupsramfunc void enter_lpm_self_refresh(struct emif_handle_s *h)
{
	uint32_t lp_status = 0;
	/* Program Self Refresh mode */
	mmio_write_32(h->ctl_cfg_base_addr + DENALI_CTL_158__SFR_OFFS, (LP_MODE_LONG_SELF_REFRESH << 8));

	/* Poll for Self Refresh Mode change */
	while (lp_status != 0x4E) {
		lp_status = ((mmio_read_32(h->ctl_cfg_base_addr + DENALI_CTL_167__SFR_OFFS) & 0x7F00) >> 8);
	}
}

__wkupsramfunc int32_t save_ddr_reg_configs(void)
{
	uint32_t lp_status;
	/* Save DDR register context in WKUP SRAM, Put the DDR in self refresh */
	emif_instance_select(&Emifhandle);
	save_ddr_registers(&Emifhandle);
	enter_lpm_self_refresh(&Emifhandle);
	/* Enable DDR data retention by writing b0110 to WKUP_CTRL_MMR. DDR32SS_PMCTRL.data_retention */
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL), 0x6U, 4U, 0U);
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL), 0x1U, 1U, 31U);
	lp_status = mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL));
	while (lp_status != ((1U << 31U) | 0x6U)) {
		lp_status = mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL));
	}
	write_mmr_field((WKUP_CTRL_MMR_SEC_4_BASE + CSL_WKUP_CTRL_MMR_CFG4_DDR32SS_PMCTRL), 0x0, 1, 31);

	return 0;
}

__wkupsramfunc int32_t restore_ddr_reg_configs(void)
{
	/* Restore DDR Controller Context & Take DDR out of self refresh, retaining of DDR & Remove DDR data retention */
	emif_instance_select(&Emifhandle);
	ddr_save_restore_exit_sequence(&Emifhandle);

	return 0;
}
