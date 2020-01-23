/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <errno.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <platform_def.h>
#include <string.h>

#include "s10_memory_controller.h"
#include "socfpga_reset_manager.h"

#define ALT_CCU_NOC_DI_SET_MSK 0x10

#define DDR_READ_LATENCY_DELAY 40
#define MAX_MEM_CAL_RETRY		3
#define PRE_CALIBRATION_DELAY		1
#define POST_CALIBRATION_DELAY		1
#define TIMEOUT_EMIF_CALIBRATION	1000
#define CLEAR_EMIF_DELAY		1000
#define CLEAR_EMIF_TIMEOUT		1000

#define DDR_CONFIG(A, B, C, R)	(((A) << 24) | ((B) << 16) | ((C) << 8) | (R))
#define DDR_CONFIG_ELEMENTS	(sizeof(ddr_config)/sizeof(uint32_t))

/* tWR = Min. 15ns constant, see JEDEC standard eg. DDR4 is JESD79-4.pdf */
#define tWR_IN_NS 15

void configure_hmc_adaptor_regs(void);
void configure_ddr_sched_ctrl_regs(void);

/* The followring are the supported configurations */
uint32_t ddr_config[] = {
	/* DDR_CONFIG(Address order,Bank,Column,Row) */
	/* List for DDR3 or LPDDR3 (pinout order > chip, row, bank, column) */
	DDR_CONFIG(0, 3, 10, 12),
	DDR_CONFIG(0, 3,  9, 13),
	DDR_CONFIG(0, 3, 10, 13),
	DDR_CONFIG(0, 3,  9, 14),
	DDR_CONFIG(0, 3, 10, 14),
	DDR_CONFIG(0, 3, 10, 15),
	DDR_CONFIG(0, 3, 11, 14),
	DDR_CONFIG(0, 3, 11, 15),
	DDR_CONFIG(0, 3, 10, 16),
	DDR_CONFIG(0, 3, 11, 16),
	DDR_CONFIG(0, 3, 12, 15),	/* 0xa */
	/* List for DDR4 only (pinout order > chip, bank, row, column) */
	DDR_CONFIG(1, 3, 10, 14),
	DDR_CONFIG(1, 4, 10, 14),
	DDR_CONFIG(1, 3, 10, 15),
	DDR_CONFIG(1, 4, 10, 15),
	DDR_CONFIG(1, 3, 10, 16),
	DDR_CONFIG(1, 4, 10, 16),
	DDR_CONFIG(1, 3, 10, 17),
	DDR_CONFIG(1, 4, 10, 17),
};

static int match_ddr_conf(uint32_t ddr_conf)
{
	int i;

	for (i = 0; i < DDR_CONFIG_ELEMENTS; i++) {
		if (ddr_conf == ddr_config[i])
			return i;
	}
	return 0;
}

static int check_hmc_clk(void)
{
	unsigned long timeout = 0;
	uint32_t hmc_clk;

	do {
		hmc_clk = mmio_read_32(S10_SYSMGR_CORE_HMC_CLK);
		if (hmc_clk & S10_SYSMGR_CORE_HMC_CLK_STATUS)
			break;
		udelay(1);
	} while (++timeout < 1000);
	if (timeout >= 1000)
		return -ETIMEDOUT;

	return 0;
}

static int clear_emif(void)
{
	uint32_t data;
	unsigned long timeout;

	mmio_write_32(S10_MPFE_HMC_ADP_RSTHANDSHAKECTRL, 0);

	timeout = 0;
	do {
		data = mmio_read_32(S10_MPFE_HMC_ADP_RSTHANDSHAKESTAT);
		if ((data & S10_MPFE_HMC_ADP_RSTHANDSHAKESTAT_SEQ2CORE) == 0)
			break;
		udelay(CLEAR_EMIF_DELAY);
	} while (++timeout < CLEAR_EMIF_TIMEOUT);
	if (timeout >= CLEAR_EMIF_TIMEOUT)
		return -ETIMEDOUT;

	return 0;
}

static int mem_calibration(void)
{
	int status = 0;
	uint32_t data;
	unsigned long timeout;
	unsigned long retry = 0;

	udelay(PRE_CALIBRATION_DELAY);

	do {
		if (retry != 0)
			INFO("DDR: Retrying DRAM calibration\n");

		timeout = 0;
		do {
			data = mmio_read_32(S10_MPFE_HMC_ADP_DDRCALSTAT);
			if (S10_MPFE_HMC_ADP_DDRCALSTAT_CAL(data) == 1)
				break;
			udelay(500);
		} while (++timeout < TIMEOUT_EMIF_CALIBRATION);

		if (S10_MPFE_HMC_ADP_DDRCALSTAT_CAL(data) == 0) {
			status = clear_emif();
			if (status)
				ERROR("Failed to clear Emif\n");
		} else {
			break;
		}
	} while (++retry < MAX_MEM_CAL_RETRY);

	if (S10_MPFE_HMC_ADP_DDRCALSTAT_CAL(data) == 0) {
		ERROR("DDR: DRAM calibration failed.\n");
		status = -EIO;
	} else {
		INFO("DDR: DRAM calibration success.\n");
		status = 0;
	}

	udelay(POST_CALIBRATION_DELAY);

	return status;
}

int init_hard_memory_controller(void)
{
	int status;

	mmio_clrbits_32(S10_CCU_CPU0_MPRT_DDR, S10_CCU_NOC_DI_SET_MSK);
	mmio_clrbits_32(S10_CCU_CPU0_MPRT_MEM0, S10_CCU_NOC_DI_SET_MSK);
	mmio_clrbits_32(S10_CCU_CPU0_MPRT_MEM1A, S10_CCU_NOC_DI_SET_MSK);
	mmio_clrbits_32(S10_CCU_CPU0_MPRT_MEM1B, S10_CCU_NOC_DI_SET_MSK);
	mmio_clrbits_32(S10_CCU_CPU0_MPRT_MEM1C, S10_CCU_NOC_DI_SET_MSK);
	mmio_clrbits_32(S10_CCU_CPU0_MPRT_MEM1D, S10_CCU_NOC_DI_SET_MSK);
	mmio_clrbits_32(S10_CCU_CPU0_MPRT_MEM1E, S10_CCU_NOC_DI_SET_MSK);

	mmio_clrbits_32(S10_CCU_IOM_MPRT_MEM0, S10_CCU_NOC_DI_SET_MSK);
	mmio_clrbits_32(S10_CCU_IOM_MPRT_MEM1A, S10_CCU_NOC_DI_SET_MSK);
	mmio_clrbits_32(S10_CCU_IOM_MPRT_MEM1B, S10_CCU_NOC_DI_SET_MSK);
	mmio_clrbits_32(S10_CCU_IOM_MPRT_MEM1C, S10_CCU_NOC_DI_SET_MSK);
	mmio_clrbits_32(S10_CCU_IOM_MPRT_MEM1D, S10_CCU_NOC_DI_SET_MSK);
	mmio_clrbits_32(S10_CCU_IOM_MPRT_MEM1E, S10_CCU_NOC_DI_SET_MSK);

	mmio_write_32(S10_NOC_FW_DDR_SCR_MPUREGION0ADDR_LIMIT, 0xFFFF0000);
	mmio_write_32(S10_NOC_FW_DDR_SCR_MPUREGION0ADDR_LIMITEXT, 0x1F);

	mmio_write_32(S10_NOC_FW_DDR_SCR_NONMPUREGION0ADDR_LIMIT, 0xFFFF0000);
	mmio_write_32(S10_NOC_FW_DDR_SCR_NONMPUREGION0ADDR_LIMITEXT, 0x1F);
	mmio_write_32(S10_SOC_NOC_FW_DDR_SCR_ENABLE, BIT(0) | BIT(8));

	status = check_hmc_clk();
	if (status) {
		ERROR("DDR: Error, HMC clock not running\n");
		return status;
	}

	mmio_clrbits_32(SOCFPGA_RSTMGR(BRGMODRST), RSTMGR_FIELD(BRG, DDRSCH));

	status = mem_calibration();
	if (status) {
		ERROR("DDR: Memory Calibration Failed\n");
		return status;
	}

	configure_hmc_adaptor_regs();
	configure_ddr_sched_ctrl_regs();

	return 0;
}

void configure_ddr_sched_ctrl_regs(void)
{
	uint32_t data, dram_addr_order, ddr_conf, bank, row, col,
		rd_to_miss, wr_to_miss, burst_len, burst_len_ddr_clk,
		burst_len_sched_clk, act_to_act, rd_to_wr, wr_to_rd, bw_ratio,
		t_rtp, t_rp, t_rcd, rd_latency, tw_rin_clk_cycles,
		bw_ratio_extended, auto_precharge = 0, act_to_act_bank, faw,
		faw_bank, bus_rd_to_rd, bus_rd_to_wr, bus_wr_to_rd;

	INFO("Init HPS NOC's DDR Scheduler.\n");

	data = mmio_read_32(S10_MPFE_IOHMC_CTRLCFG1);
	dram_addr_order = S10_MPFE_IOHMC_CTRLCFG1_CFG_ADDR_ORDER(data);

	data = mmio_read_32(S10_MPFE_IOHMC_DRAMADDRW);

	col  = IOHMC_DRAMADDRW_COL_ADDR_WIDTH(data);
	row  = IOHMC_DRAMADDRW_ROW_ADDR_WIDTH(data);
	bank = IOHMC_DRAMADDRW_BANK_ADDR_WIDTH(data) +
		IOHMC_DRAMADDRW_BANK_GRP_ADDR_WIDTH(data);

	ddr_conf = match_ddr_conf(DDR_CONFIG(dram_addr_order, bank, col, row));

	if (ddr_conf) {
		mmio_clrsetbits_32(
			S10_MPFE_DDR_MAIN_SCHED_DDRCONF,
			S10_MPFE_DDR_MAIN_SCHED_DDRCONF_SET_MSK,
			S10_MPFE_DDR_MAIN_SCHED_DDRCONF_SET(ddr_conf));
	} else {
		ERROR("DDR: Cannot find predefined ddrConf configuration.\n");
	}

	mmio_write_32(S10_MPFE_HMC_ADP(ADP_DRAMADDRWIDTH), data);

	data = mmio_read_32(S10_MPFE_IOHMC_DRAMTIMING0);
	rd_latency = S10_MPFE_IOHMC_REG_DRAMTIMING0_CFG_TCL(data);

	data = mmio_read_32(S10_MPFE_IOHMC_CALTIMING0);
	act_to_act = ACT_TO_ACT(data);
	t_rcd = ACT_TO_RDWR(data);
	act_to_act_bank = ACT_TO_ACT_DIFF_BANK(data);

	data = mmio_read_32(S10_MPFE_IOHMC_CALTIMING1);
	rd_to_wr = RD_TO_WR(data);
	bus_rd_to_rd = RD_TO_RD_DIFF_CHIP(data);
	bus_rd_to_wr = RD_TO_WR_DIFF_CHIP(data);

	data = mmio_read_32(S10_MPFE_IOHMC_CALTIMING2);
	t_rtp = RD_TO_PCH(data);

	data = mmio_read_32(S10_MPFE_IOHMC_CALTIMING3);
	wr_to_rd = CALTIMING3_WR_TO_RD(data);
	bus_wr_to_rd = CALTIMING3_WR_TO_RD_DIFF_CHIP(data);

	data = mmio_read_32(S10_MPFE_IOHMC_CALTIMING4);
	t_rp = PCH_TO_VALID(data);

	data = mmio_read_32(S10_MPFE_HMC_ADP(HMC_ADP_DDRIOCTRL));
	bw_ratio = ((HMC_ADP_DDRIOCTRL_IO_SIZE(data) == 0) ? 0 : 1);

	data = mmio_read_32(S10_MPFE_IOHMC_CTRLCFG0);
	burst_len = HMC_ADP_DDRIOCTRL_CTRL_BURST_LENGTH(data);
	burst_len_ddr_clk = burst_len / 2;
	burst_len_sched_clk = ((burst_len/2) / 2);

	data = mmio_read_32(S10_MPFE_IOHMC_CTRLCFG0);
	switch (S10_MPFE_IOHMC_REG_CTRLCFG0_CFG_MEM_TYPE(data)) {
	case 1:
		/* DDR4 - 1333MHz */
		/* 20 (19.995) clock cycles = 15ns */
		/* Calculate with rounding */
		tw_rin_clk_cycles = (((tWR_IN_NS * 1333) % 1000) >= 500) ?
			((tWR_IN_NS * 1333) / 1000) + 1 :
			((tWR_IN_NS * 1333) / 1000);
		break;
	default:
		/* Others - 1066MHz or slower */
		/* 16 (15.990) clock cycles = 15ns */
		/* Calculate with rounding */
		tw_rin_clk_cycles = (((tWR_IN_NS * 1066) % 1000) >= 500) ?
			((tWR_IN_NS * 1066) / 1000) + 1 :
			((tWR_IN_NS * 1066) / 1000);
		break;
	}

	rd_to_miss = t_rtp + t_rp + t_rcd - burst_len_sched_clk;
	wr_to_miss = ((rd_latency + burst_len_ddr_clk + 2 + tw_rin_clk_cycles)
			/ 2) - rd_to_wr + t_rp + t_rcd;

	mmio_write_32(S10_MPFE_DDR_MAIN_SCHED_DDRTIMING,
		bw_ratio << DDRTIMING_BWRATIO_OFST |
		wr_to_rd << DDRTIMING_WRTORD_OFST|
		rd_to_wr << DDRTIMING_RDTOWR_OFST |
		burst_len_sched_clk << DDRTIMING_BURSTLEN_OFST |
		wr_to_miss << DDRTIMING_WRTOMISS_OFST |
		rd_to_miss << DDRTIMING_RDTOMISS_OFST |
		act_to_act << DDRTIMING_ACTTOACT_OFST);

	data = mmio_read_32(S10_MPFE_HMC_ADP(HMC_ADP_DDRIOCTRL));
	bw_ratio_extended = ((ADP_DDRIOCTRL_IO_SIZE(data) == 0) ? 1 : 0);

	mmio_write_32(S10_MPFE_DDR_MAIN_SCHED_DDRMODE,
		bw_ratio_extended << DDRMODE_BWRATIOEXTENDED_OFST |
		auto_precharge << DDRMODE_AUTOPRECHARGE_OFST);

	mmio_write_32(S10_MPFE_DDR_MAIN_SCHED_READLATENCY,
		(rd_latency / 2) + DDR_READ_LATENCY_DELAY);

	data = mmio_read_32(S10_MPFE_IOHMC_CALTIMING9);
	faw = S10_MPFE_IOHMC_CALTIMING9_ACT_TO_ACT(data);

	faw_bank = 1; // always 1 because we always have 4 bank DDR.

	mmio_write_32(S10_MPFE_DDR_MAIN_SCHED_ACTIVATE,
		faw_bank << S10_MPFE_DDR_MAIN_SCHED_ACTIVATE_FAWBANK_OFST |
		faw << S10_MPFE_DDR_MAIN_SCHED_ACTIVATE_FAW_OFST |
		act_to_act_bank << S10_MPFE_DDR_MAIN_SCHED_ACTIVATE_RRD_OFST);

	mmio_write_32(S10_MPFE_DDR_MAIN_SCHED_DEVTODEV,
		((bus_rd_to_rd
			<< S10_MPFE_DDR_MAIN_SCHED_DEVTODEV_BUSRDTORD_OFST)
			& S10_MPFE_DDR_MAIN_SCHED_DEVTODEV_BUSRDTORD_MSK) |
		((bus_rd_to_wr
			<< S10_MPFE_DDR_MAIN_SCHED_DEVTODEV_BUSRDTOWR_OFST)
			& S10_MPFE_DDR_MAIN_SCHED_DEVTODEV_BUSRDTOWR_MSK) |
		((bus_wr_to_rd
			<< S10_MPFE_DDR_MAIN_SCHED_DEVTODEV_BUSWRTORD_OFST)
			& S10_MPFE_DDR_MAIN_SCHED_DEVTODEV_BUSWRTORD_MSK));

}

unsigned long get_physical_dram_size(void)
{
	uint32_t data;
	unsigned long ram_addr_width, ram_ext_if_io_width;

	data = mmio_read_32(S10_MPFE_HMC_ADP_DDRIOCTRL);
	switch (S10_MPFE_HMC_ADP_DDRIOCTRL_IO_SIZE(data)) {
	case 0:
		ram_ext_if_io_width = 16;
		break;
	case 1:
		ram_ext_if_io_width = 32;
		break;
	case 2:
		ram_ext_if_io_width = 64;
		break;
	default:
		ram_ext_if_io_width = 0;
		break;
	}

	data = mmio_read_32(S10_MPFE_IOHMC_REG_DRAMADDRW);
	ram_addr_width = IOHMC_DRAMADDRW_CFG_COL_ADDR_WIDTH(data) +
		IOHMC_DRAMADDRW_CFG_ROW_ADDR_WIDTH(data) +
		IOHMC_DRAMADDRW_CFG_BANK_ADDR_WIDTH(data) +
		IOHMC_DRAMADDRW_CFG_BANK_GROUP_ADDR_WIDTH(data) +
		IOHMC_DRAMADDRW_CFG_CS_ADDR_WIDTH(data);

	return (1 << ram_addr_width) * (ram_ext_if_io_width / 8);
}



void configure_hmc_adaptor_regs(void)
{
	uint32_t data;
	uint32_t dram_io_width;

	dram_io_width = S10_MPFE_IOHMC_NIOSRESERVE0_NIOS_RESERVE0(
		mmio_read_32(S10_MPFE_IOHMC_REG_NIOSRESERVE0_OFST));

	dram_io_width = (dram_io_width & 0xFF) >> 5;

	mmio_clrsetbits_32(S10_MPFE_HMC_ADP_DDRIOCTRL,
		S10_MPFE_HMC_ADP_DDRIOCTRL_IO_SIZE_MSK,
		dram_io_width << S10_MPFE_HMC_ADP_DDRIOCTRL_IO_SIZE_OFST);

	mmio_write_32(S10_MPFE_HMC_ADP_HPSINTFCSEL,
		S10_MPFE_HMC_ADP_HPSINTFCSEL_ENABLE);

	data = mmio_read_32(S10_MPFE_IOHMC_REG_CTRLCFG1);
	if (data & (1 << S10_IOHMC_CTRLCFG1_ENABLE_ECC_OFST)) {
		mmio_clrsetbits_32(S10_MPFE_HMC_ADP_ECCCTRL1,
			S10_MPFE_HMC_ADP_ECCCTRL1_AUTOWB_CNT_RST_SET_MSK |
			S10_MPFE_HMC_ADP_ECCCTRL1_CNT_RST_SET_MSK |
			S10_MPFE_HMC_ADP_ECCCTRL1_ECC_EN_SET_MSK,
			S10_MPFE_HMC_ADP_ECCCTRL1_AUTOWB_CNT_RST_SET_MSK |
			S10_MPFE_HMC_ADP_ECCCTRL1_CNT_RST_SET_MSK);

		mmio_clrsetbits_32(S10_MPFE_HMC_ADP_ECCCTRL2,
			S10_MPFE_HMC_ADP_ECCCTRL2_OVRW_RB_ECC_EN_SET_MSK |
			S10_MPFE_HMC_ADP_ECCCTRL2_RMW_EN_SET_MSK |
			S10_MPFE_HMC_ADP_ECCCTRL2_AUTOWB_EN_SET_MSK,
			S10_MPFE_HMC_ADP_ECCCTRL2_RMW_EN_SET_MSK |
			S10_MPFE_HMC_ADP_ECCCTRL2_AUTOWB_EN_SET_MSK);

		mmio_clrsetbits_32(S10_MPFE_HMC_ADP_ECCCTRL1,
			S10_MPFE_HMC_ADP_ECCCTRL1_AUTOWB_CNT_RST_SET_MSK |
			S10_MPFE_HMC_ADP_ECCCTRL1_CNT_RST_SET_MSK |
			S10_MPFE_HMC_ADP_ECCCTRL1_ECC_EN_SET_MSK,
			S10_MPFE_HMC_ADP_ECCCTRL1_ECC_EN_SET_MSK);
		INFO("Scrubbing ECC\n");

		/* ECC Scrubbing */
		zeromem(DRAM_BASE, DRAM_SIZE);
	} else {
		INFO("ECC is disabled.\n");
	}
}

