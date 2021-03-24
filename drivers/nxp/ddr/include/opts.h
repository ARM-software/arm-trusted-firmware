/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DDR_OPTS_H
#define DDR_OPTS_H

#define SDRAM_TYPE_DDR4		5	/* sdram_cfg register */

#define DDR_BC4			4	/* burst chop */
#define DDR_OTF			6	/* on-the-fly BC4 and BL8 */
#define DDR_BL8			8	/* burst length 8 */

#define DDR4_RTT_OFF		0
#define DDR4_RTT_60_OHM		1	/* RZQ/4 */
#define DDR4_RTT_120_OHM	2	/* RZQ/2 */
#define DDR4_RTT_40_OHM		3	/* RZQ/6 */
#define DDR4_RTT_240_OHM	4	/* RZQ/1 */
#define DDR4_RTT_48_OHM		5	/* RZQ/5 */
#define DDR4_RTT_80_OHM		6	/* RZQ/3 */
#define DDR4_RTT_34_OHM		7	/* RZQ/7 */
#define DDR4_RTT_WR_OFF		0
#define DDR4_RTT_WR_120_OHM	1
#define DDR4_RTT_WR_240_OHM	2
#define DDR4_RTT_WR_HZ		3
#define DDR4_RTT_WR_80_OHM	4
#define DDR_ODT_NEVER		0x0
#define DDR_ODT_CS		0x1
#define DDR_ODT_ALL_OTHER_CS	0x2
#define DDR_ODT_OTHER_DIMM	0x3
#define DDR_ODT_ALL		0x4
#define DDR_ODT_SAME_DIMM	0x5
#define DDR_ODT_CS_AND_OTHER_DIMM 0x6
#define DDR_ODT_OTHER_CS_ONSAMEDIMM 0x7
#define DDR_BA_INTLV_CS01	0x40
#define DDR_BA_INTLV_CS0123	0x64
#define DDR_BA_NONE		0
#define DDR_256B_INTLV		0x8

struct memctl_opt {
	int rdimm;
	unsigned int dbw_cap_shift;
	struct local_opts_s {
		unsigned int auto_precharge;
		unsigned int odt_rd_cfg;
		unsigned int odt_wr_cfg;
		unsigned int odt_rtt_norm;
		unsigned int odt_rtt_wr;
	} cs_odt[DDRC_NUM_CS];
	int ctlr_intlv;
	unsigned int ctlr_intlv_mode;
	unsigned int ba_intlv;
	int addr_hash;
	int ecc_mode;
	int ctlr_init_ecc;
	int self_refresh_in_sleep;
	int self_refresh_irq_en;
	int dynamic_power;
	/* memory data width 0 = 64-bit, 1 = 32-bit, 2 = 16-bit */
	unsigned int data_bus_dimm;
	unsigned int data_bus_used;	/* on individual board */
	unsigned int burst_length;	/* BC4, OTF and BL8 */
	int otf_burst_chop_en;
	int mirrored_dimm;
	int quad_rank_present;
	int output_driver_impedance;
	int ap_en;
	int x4_en;

	int caslat_override;
	unsigned int caslat_override_value;
	int addt_lat_override;
	unsigned int addt_lat_override_value;

	unsigned int clk_adj;
	unsigned int cpo_sample;
	unsigned int wr_data_delay;

	unsigned int cswl_override;
	unsigned int wrlvl_override;
	unsigned int wrlvl_sample;
	unsigned int wrlvl_start;
	unsigned int wrlvl_ctl_2;
	unsigned int wrlvl_ctl_3;

	int half_strength_drive_en;
	int twot_en;
	int threet_en;
	unsigned int bstopre;
	unsigned int tfaw_ps;

	int rtt_override;
	unsigned int rtt_override_value;
	unsigned int rtt_wr_override_value;
	unsigned int rtt_park;

	int auto_self_refresh_en;
	unsigned int sr_it;
	unsigned int ddr_cdr1;
	unsigned int ddr_cdr2;

	unsigned int trwt_override;
	unsigned int trwt;
	unsigned int twrt;
	unsigned int trrt;
	unsigned int twwt;

	unsigned int vref_phy;
	unsigned int vref_dimm;
	unsigned int odt;
	unsigned int phy_tx_impedance;
	unsigned int phy_atx_impedance;
	unsigned int skip2d;
};

#endif /* DDR_OPTS_H */
