/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DDR_IMMAP_H
#define DDR_IMMAP_H

#define	DDR_DBUS_64		0
#define	DDR_DBUS_32		1
#define	DDR_DBUS_16		2

/*
 * DDRC register file for DDRC 5.0 and above
 */
struct ccsr_ddr {
	struct {
		unsigned int a;		 /* 0x0, 0x8, 0x10, 0x18 */
		unsigned int res;	 /* 0x4, 0xc, 0x14, 0x1c */
	} bnds[4];
	unsigned char	res_20[0x40 - 0x20];
	unsigned int	dec[10];	 /* 0x40 */
	unsigned char	res_68[0x80 - 0x68];
	unsigned int	csn_cfg[4];	 /* 0x80, 0x84, 0x88, 0x8c */
	unsigned char	res_90[48];
	unsigned int	csn_cfg_2[4];	 /* 0xc0, 0xc4, 0xc8, 0xcc */
	unsigned char	res_d0[48];
	unsigned int	timing_cfg_3;	 /* SDRAM Timing Configuration 3 */
	unsigned int	timing_cfg_0;	 /* SDRAM Timing Configuration 0 */
	unsigned int	timing_cfg_1;	 /* SDRAM Timing Configuration 1 */
	unsigned int	timing_cfg_2;	 /* SDRAM Timing Configuration 2 */
	unsigned int	sdram_cfg;	 /* SDRAM Control Configuration */
	unsigned int	sdram_cfg_2;	 /* SDRAM Control Configuration 2 */
	unsigned int	sdram_mode;	 /* SDRAM Mode Configuration */
	unsigned int	sdram_mode_2;	 /* SDRAM Mode Configuration 2 */
	unsigned int	sdram_md_cntl;	 /* SDRAM Mode Control */
	unsigned int	sdram_interval;	 /* SDRAM Interval Configuration */
	unsigned int	sdram_data_init; /* SDRAM Data initialization */
	unsigned char	res_12c[4];
	unsigned int	sdram_clk_cntl;	 /* SDRAM Clock Control */
	unsigned char	res_134[20];
	unsigned int	init_addr;	 /* training init addr */
	unsigned int	init_ext_addr;	 /* training init extended addr */
	unsigned char	res_150[16];
	unsigned int	timing_cfg_4;	 /* SDRAM Timing Configuration 4 */
	unsigned int	timing_cfg_5;	 /* SDRAM Timing Configuration 5 */
	unsigned int	timing_cfg_6;	 /* SDRAM Timing Configuration 6 */
	unsigned int	timing_cfg_7;	 /* SDRAM Timing Configuration 7 */
	unsigned int	zq_cntl;	 /* ZQ calibration control*/
	unsigned int	wrlvl_cntl;	 /* write leveling control*/
	unsigned char	reg_178[4];
	unsigned int	ddr_sr_cntr;	 /* self refresh counter */
	unsigned int	ddr_sdram_rcw_1; /* Control Words 1 */
	unsigned int	ddr_sdram_rcw_2; /* Control Words 2 */
	unsigned char	reg_188[8];
	unsigned int	ddr_wrlvl_cntl_2; /* write leveling control 2 */
	unsigned int	ddr_wrlvl_cntl_3; /* write leveling control 3 */
	unsigned char	res_198[0x1a0-0x198];
	unsigned int	ddr_sdram_rcw_3;
	unsigned int	ddr_sdram_rcw_4;
	unsigned int	ddr_sdram_rcw_5;
	unsigned int	ddr_sdram_rcw_6;
	unsigned char	res_1b0[0x200-0x1b0];
	unsigned int	sdram_mode_3;	 /* SDRAM Mode Configuration 3 */
	unsigned int	sdram_mode_4;	 /* SDRAM Mode Configuration 4 */
	unsigned int	sdram_mode_5;	 /* SDRAM Mode Configuration 5 */
	unsigned int	sdram_mode_6;	 /* SDRAM Mode Configuration 6 */
	unsigned int	sdram_mode_7;	 /* SDRAM Mode Configuration 7 */
	unsigned int	sdram_mode_8;	 /* SDRAM Mode Configuration 8 */
	unsigned char	res_218[0x220-0x218];
	unsigned int	sdram_mode_9;	 /* SDRAM Mode Configuration 9 */
	unsigned int	sdram_mode_10;	 /* SDRAM Mode Configuration 10 */
	unsigned int	sdram_mode_11;	 /* SDRAM Mode Configuration 11 */
	unsigned int	sdram_mode_12;	 /* SDRAM Mode Configuration 12 */
	unsigned int	sdram_mode_13;	 /* SDRAM Mode Configuration 13 */
	unsigned int	sdram_mode_14;	 /* SDRAM Mode Configuration 14 */
	unsigned int	sdram_mode_15;	 /* SDRAM Mode Configuration 15 */
	unsigned int	sdram_mode_16;	 /* SDRAM Mode Configuration 16 */
	unsigned char	res_240[0x250-0x240];
	unsigned int	timing_cfg_8;	 /* SDRAM Timing Configuration 8 */
	unsigned int	timing_cfg_9;	 /* SDRAM Timing Configuration 9 */
	unsigned int	timing_cfg_10;	 /* SDRAM Timing COnfigurtion 10 */
	unsigned char   res_258[0x260-0x25c];
	unsigned int	sdram_cfg_3;
	unsigned char	res_264[0x270-0x264];
	unsigned int	sdram_md_cntl_2;
	unsigned char	res_274[0x400-0x274];
	unsigned int	dq_map[4];
	unsigned char	res_410[0x800-0x410];
	unsigned int	tx_cfg[4];
	unsigned char	res_810[0xb20-0x810];
	unsigned int	ddr_dsr1;	 /* Debug Status 1 */
	unsigned int	ddr_dsr2;	 /* Debug Status 2 */
	unsigned int	ddr_cdr1;	 /* Control Driver 1 */
	unsigned int	ddr_cdr2;	 /* Control Driver 2 */
	unsigned char	res_b30[200];
	unsigned int	ip_rev1;	 /* IP Block Revision 1 */
	unsigned int	ip_rev2;	 /* IP Block Revision 2 */
	unsigned int	eor;		 /* Enhanced Optimization Register */
	unsigned char	res_c04[252];
	unsigned int	mtcr;		 /* Memory Test Control Register */
	unsigned char	res_d04[28];
	unsigned int	mtp[10];	 /* Memory Test Patterns */
	unsigned char	res_d48[184];
	unsigned int	data_err_inject_hi; /* Data Path Err Injection Mask Hi*/
	unsigned int	data_err_inject_lo;/* Data Path Err Injection Mask Lo*/
	unsigned int	ecc_err_inject;	 /* Data Path Err Injection Mask ECC */
	unsigned char	res_e0c[20];
	unsigned int	capture_data_hi; /* Data Path Read Capture High */
	unsigned int	capture_data_lo; /* Data Path Read Capture Low */
	unsigned int	capture_ecc;	 /* Data Path Read Capture ECC */
	unsigned char	res_e2c[20];
	unsigned int	err_detect;	 /* Error Detect */
	unsigned int	err_disable;	 /* Error Disable */
	unsigned int	err_int_en;
	unsigned int	capture_attributes; /* Error Attrs Capture */
	unsigned int	capture_address; /* Error Addr Capture */
	unsigned int	capture_ext_address; /* Error Extended Addr Capture */
	unsigned int	err_sbe;	 /* Single-Bit ECC Error Management */
	unsigned char	res_e5c[164];
	unsigned int	debug[64];	 /* debug_1 to debug_64 */
};
#endif /* DDR_IMMAP_H */
