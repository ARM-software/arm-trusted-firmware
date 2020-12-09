/*
 * Copyright 2021 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _INPUT_H_
#define _INPUT_H_

enum dram_types {
	DDR4,
	DDR3,
	LPDDR4,
	LPDDR3,
	LPDDR2,
	DDR5,
};

enum dimm_types {
	UDIMM,
	SODIMM,
	RDIMM,
	LRDIMM,
	NODIMM,
};

struct input_basic {
	enum dram_types dram_type;
	enum dimm_types dimm_type;
	int lp4x_mode;		/* 0x1 = lpddr4x mode, when dram_type is lpddr4
				 */
				/* not used for protocols other than lpddr4 */
	int num_dbyte;		/* number of dbytes physically instantiated */
	int num_active_dbyte_dfi0;	/* number of active dbytes to be
					 * controlled by dfi0
					 */
	int num_active_dbyte_dfi1;	/* number of active dbytes to be
					 * controlled by  dfi1. Not used for
					 * protocols other than lpddr3 and
					 * lpddr4
					 */
	int num_anib;		/* number of anibs physically instantiated */
	int num_rank_dfi0;	/* number of ranks in dfi0 channel */
	int num_rank_dfi1;	/* number of ranks in dfi1 channel */
	int dram_data_width;	/* 4,8,16 or 32 depending on protocol and dram
				 * type
				 */
	int num_pstates;
	int frequency;		/* memclk frequency in mhz -- round up */
	int pll_bypass;		/* pll bypass enable */
	int dfi_freq_ratio;	/* selected dfi frequency ratio */
	int dfi1exists;		/* whether they phy config has dfi1 channel */
	int train2d;
	int hard_macro_ver;
	int read_dbienable;
	int dfi_mode;		/* no longer used */
};

struct input_advanced {
	int d4rx_preamble_length;
	int d4tx_preamble_length;
	int ext_cal_res_val;	/* external pull-down resistor */
	int is2ttiming;
	int odtimpedance;
	int tx_impedance;
	int atx_impedance;
	int mem_alert_en;
	int mem_alert_puimp;
	int mem_alert_vref_level;
	int mem_alert_sync_bypass;
	int dis_dyn_adr_tri;
	int phy_mstr_train_interval;
	int phy_mstr_max_req_to_ack;
	int wdqsext;
	int cal_interval;
	int cal_once;
	int dram_byte_swap;
	int rx_en_back_off;
	int train_sequence_ctrl;
	int phy_gen2_umctl_opt;
	int phy_gen2_umctl_f0rc5x;
	int tx_slew_rise_dq;
	int tx_slew_fall_dq;
	int tx_slew_rise_ac;
	int tx_slew_fall_ac;
	int enable_high_clk_skew_fix;
	int disable_unused_addr_lns;
	int phy_init_sequence_num;
	int cs_mode;		/* rdimm */
	int cast_cs_to_cid;	/* rdimm */
};

struct input {
	struct input_basic basic;
	struct input_advanced adv;
	unsigned int mr[7];
	unsigned int cs_d0;
	unsigned int cs_d1;
	unsigned int mirror;
	unsigned int odt[4];
	unsigned int rcw[16];
	unsigned int rcw3x;
	unsigned int vref;
};

#endif
