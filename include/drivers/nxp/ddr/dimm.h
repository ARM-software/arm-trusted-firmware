/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DIMM_H
#define DIMM_H

#define SPD_MEMTYPE_DDR4        0x0C

#define DDR4_SPD_MODULETYPE_MASK        0x0f
#define DDR4_SPD_MODULETYPE_EXT         0x00
#define DDR4_SPD_RDIMM			0x01
#define DDR4_SPD_UDIMM			0x02
#define DDR4_SPD_SO_DIMM		0x03
#define DDR4_SPD_LRDIMM			0x04
#define DDR4_SPD_MINI_RDIMM		0x05
#define DDR4_SPD_MINI_UDIMM		0x06
#define DDR4_SPD_72B_SO_RDIMM		0x08
#define DDR4_SPD_72B_SO_UDIMM		0x09
#define DDR4_SPD_16B_SO_DIMM		0x0c
#define DDR4_SPD_32B_SO_DIMM		0x0d

#define SPD_SPA0_ADDRESS		0x36
#define SPD_SPA1_ADDRESS		0x37

#define spd_to_ps(mtb, ftb)	\
	((mtb) * pdimm->mtb_ps + ((ftb) * pdimm->ftb_10th_ps) / 10)

#ifdef DDR_DEBUG
#define dump_spd(spd, len) {				\
	register int i;					\
	register unsigned char *buf = (void *)(spd);	\
							\
	for (i = 0; i < (len); i++) {			\
		print_uint(i);				\
		puts("\t: 0x");				\
		print_hex(buf[i]);			\
		puts("\n");				\
	}						\
}
#else
#define dump_spd(spd, len) {}
#endif

/* From JEEC Standard No. 21-C release 23A */
struct ddr4_spd {
	/* General Section: Bytes 0-127 */
	unsigned char info_size_crc;	/*  0 # bytes */
	unsigned char spd_rev;		/*  1 Total # bytes of SPD */
	unsigned char mem_type;		/*  2 Key Byte / mem type */
	unsigned char module_type;	/*  3 Key Byte / Module Type */
	unsigned char density_banks;	/*  4 Density and Banks	*/
	unsigned char addressing;	/*  5 Addressing */
	unsigned char package_type;	/*  6 Package type */
	unsigned char opt_feature;	/*  7 Optional features */
	unsigned char thermal_ref;	/*  8 Thermal and refresh */
	unsigned char oth_opt_features;	/*  9 Other optional features */
	unsigned char res_10;		/* 10 Reserved */
	unsigned char module_vdd;	/* 11 Module nominal voltage */
	unsigned char organization;	/* 12 Module Organization */
	unsigned char bus_width;	/* 13 Module Memory Bus Width */
	unsigned char therm_sensor;	/* 14 Module Thermal Sensor */
	unsigned char ext_type;		/* 15 Extended module type */
	unsigned char res_16;
	unsigned char timebases;	/* 17 MTb and FTB */
	unsigned char tck_min;		/* 18 tCKAVGmin */
	unsigned char tck_max;		/* 19 TCKAVGmax */
	unsigned char caslat_b1;	/* 20 CAS latencies, 1st byte */
	unsigned char caslat_b2;	/* 21 CAS latencies, 2nd byte */
	unsigned char caslat_b3;	/* 22 CAS latencies, 3rd byte */
	unsigned char caslat_b4;	/* 23 CAS latencies, 4th byte */
	unsigned char taa_min;		/* 24 Min CAS Latency Time */
	unsigned char trcd_min;		/* 25 Min RAS# to CAS# Delay Time */
	unsigned char trp_min;		/* 26 Min Row Precharge Delay Time */
	unsigned char tras_trc_ext;	/* 27 Upper Nibbles for tRAS and tRC */
	unsigned char tras_min_lsb;	/* 28 tRASmin, lsb */
	unsigned char trc_min_lsb;	/* 29 tRCmin, lsb */
	unsigned char trfc1_min_lsb;	/* 30 Min Refresh Recovery Delay Time */
	unsigned char trfc1_min_msb;	/* 31 Min Refresh Recovery Delay Time */
	unsigned char trfc2_min_lsb;	/* 32 Min Refresh Recovery Delay Time */
	unsigned char trfc2_min_msb;	/* 33 Min Refresh Recovery Delay Time */
	unsigned char trfc4_min_lsb;	/* 34 Min Refresh Recovery Delay Time */
	unsigned char trfc4_min_msb;	/* 35 Min Refresh Recovery Delay Time */
	unsigned char tfaw_msb;		/* 36 Upper Nibble for tFAW */
	unsigned char tfaw_min;		/* 37 tFAW, lsb */
	unsigned char trrds_min;	/* 38 tRRD_Smin, MTB */
	unsigned char trrdl_min;	/* 39 tRRD_Lmin, MTB */
	unsigned char tccdl_min;	/* 40 tCCS_Lmin, MTB */
	unsigned char res_41[60-41];	/* 41 Rserved */
	unsigned char mapping[78-60];	/* 60~77 Connector to SDRAM bit map */
	unsigned char res_78[117-78];	/* 78~116, Reserved */
	signed char fine_tccdl_min;	/* 117 Fine offset for tCCD_Lmin */
	signed char fine_trrdl_min;	/* 118 Fine offset for tRRD_Lmin */
	signed char fine_trrds_min;	/* 119 Fine offset for tRRD_Smin */
	signed char fine_trc_min;	/* 120 Fine offset for tRCmin */
	signed char fine_trp_min;	/* 121 Fine offset for tRPmin */
	signed char fine_trcd_min;	/* 122 Fine offset for tRCDmin */
	signed char fine_taa_min;	/* 123 Fine offset for tAAmin */
	signed char fine_tck_max;	/* 124 Fine offset for tCKAVGmax */
	signed char fine_tck_min;	/* 125 Fine offset for tCKAVGmin */
	/* CRC: Bytes 126-127 */
	unsigned char crc[2];		/* 126-127 SPD CRC */

	/* Module-Specific Section: Bytes 128-255 */
	union {
		struct {
			/* 128 (Unbuffered) Module Nominal Height */
			unsigned char mod_height;
			/* 129 (Unbuffered) Module Maximum Thickness */
			unsigned char mod_thickness;
			/* 130 (Unbuffered) Reference Raw Card Used */
			unsigned char ref_raw_card;
			/* 131 (Unbuffered) Address Mapping from
			 *     Edge Connector to DRAM
			 */
			unsigned char addr_mapping;
			/* 132~253 (Unbuffered) Reserved */
			unsigned char res_132[254-132];
			/* 254~255 CRC */
			unsigned char crc[2];
		} unbuffered;
		struct {
			/* 128 (Registered) Module Nominal Height */
			unsigned char mod_height;
			/* 129 (Registered) Module Maximum Thickness */
			unsigned char mod_thickness;
			/* 130 (Registered) Reference Raw Card Used */
			unsigned char ref_raw_card;
			/* 131 DIMM Module Attributes */
			unsigned char modu_attr;
			/* 132 RDIMM Thermal Heat Spreader Solution */
			unsigned char thermal;
			/* 133 Register Manufacturer ID Code, LSB */
			unsigned char reg_id_lo;
			/* 134 Register Manufacturer ID Code, MSB */
			unsigned char reg_id_hi;
			/* 135 Register Revision Number */
			unsigned char reg_rev;
			/* 136 Address mapping from register to DRAM */
			unsigned char reg_map;
			unsigned char ca_stren;
			unsigned char clk_stren;
			/* 139~253 Reserved */
			unsigned char res_139[254-139];
			/* 254~255 CRC */
			unsigned char crc[2];
		} registered;
		struct {
			/* 128 (Loadreduced) Module Nominal Height */
			unsigned char mod_height;
			/* 129 (Loadreduced) Module Maximum Thickness */
			unsigned char mod_thickness;
			/* 130 (Loadreduced) Reference Raw Card Used */
			unsigned char ref_raw_card;
			/* 131 DIMM Module Attributes */
			unsigned char modu_attr;
			/* 132 RDIMM Thermal Heat Spreader Solution */
			unsigned char thermal;
			/* 133 Register Manufacturer ID Code, LSB */
			unsigned char reg_id_lo;
			/* 134 Register Manufacturer ID Code, MSB */
			unsigned char reg_id_hi;
			/* 135 Register Revision Number */
			unsigned char reg_rev;
			/* 136 Address mapping from register to DRAM */
			unsigned char reg_map;
			/* 137 Register Output Drive Strength for CMD/Add*/
			unsigned char reg_drv;
			/* 138 Register Output Drive Strength for CK */
			unsigned char reg_drv_ck;
			/* 139 Data Buffer Revision Number */
			unsigned char data_buf_rev;
			/* 140 DRAM VrefDQ for Package Rank 0 */
			unsigned char vrefqe_r0;
			/* 141 DRAM VrefDQ for Package Rank 1 */
			unsigned char vrefqe_r1;
			/* 142 DRAM VrefDQ for Package Rank 2 */
			unsigned char vrefqe_r2;
			/* 143 DRAM VrefDQ for Package Rank 3 */
			unsigned char vrefqe_r3;
			/* 144 Data Buffer VrefDQ for DRAM Interface */
			unsigned char data_intf;
			/*
			 * 145 Data Buffer MDQ Drive Strength and RTT
			 * for data rate <= 1866
			 */
			unsigned char data_drv_1866;
			/*
			 * 146 Data Buffer MDQ Drive Strength and RTT
			 * for 1866 < data rate <= 2400
			 */
			unsigned char data_drv_2400;
			/*
			 * 147 Data Buffer MDQ Drive Strength and RTT
			 * for 2400 < data rate <= 3200
			 */
			unsigned char data_drv_3200;
			/* 148 DRAM Drive Strength */
			unsigned char dram_drv;
			/*
			 * 149 DRAM ODT (RTT_WR, RTT_NOM)
			 * for data rate <= 1866
			 */
			unsigned char dram_odt_1866;
			/*
			 * 150 DRAM ODT (RTT_WR, RTT_NOM)
			 * for 1866 < data rate <= 2400
			 */
			unsigned char dram_odt_2400;
			/*
			 * 151 DRAM ODT (RTT_WR, RTT_NOM)
			 * for 2400 < data rate <= 3200
			 */
			unsigned char dram_odt_3200;
			/*
			 * 152 DRAM ODT (RTT_PARK)
			 * for data rate <= 1866
			 */
			unsigned char dram_odt_park_1866;
			/*
			 * 153 DRAM ODT (RTT_PARK)
			 * for 1866 < data rate <= 2400
			 */
			unsigned char dram_odt_park_2400;
			/*
			 * 154 DRAM ODT (RTT_PARK)
			 * for 2400 < data rate <= 3200
			 */
			unsigned char dram_odt_park_3200;
			unsigned char res_155[254-155];	/* Reserved */
			/* 254~255 CRC */
			unsigned char crc[2];
		} loadreduced;
		unsigned char uc[128]; /* 128-255 Module-Specific Section */
	} mod_section;

	unsigned char res_256[320-256];	/* 256~319 Reserved */

	/* Module supplier's data: Byte 320~383 */
	unsigned char mmid_lsb;		/* 320 Module MfgID Code LSB */
	unsigned char mmid_msb;		/* 321 Module MfgID Code MSB */
	unsigned char mloc;		/* 322 Mfg Location */
	unsigned char mdate[2];		/* 323~324 Mfg Date */
	unsigned char sernum[4];	/* 325~328 Module Serial Number */
	unsigned char mpart[20];	/* 329~348 Mfg's Module Part Number */
	unsigned char mrev;		/* 349 Module Revision Code */
	unsigned char dmid_lsb;		/* 350 DRAM MfgID Code LSB */
	unsigned char dmid_msb;		/* 351 DRAM MfgID Code MSB */
	unsigned char stepping;		/* 352 DRAM stepping */
	unsigned char msd[29];		/* 353~381 Mfg's Specific Data */
	unsigned char res_382[2];	/* 382~383 Reserved */
};

/* Parameters for a DDR dimm computed from the SPD */
struct dimm_params {
	/* DIMM organization parameters */
	char mpart[19];		/* guaranteed null terminated */

	unsigned int n_ranks;
	unsigned int die_density;
	unsigned long long rank_density;
	unsigned long long capacity;
	unsigned int primary_sdram_width;
	unsigned int ec_sdram_width;
	unsigned int rdimm;
	unsigned int package_3ds;	/* number of dies in 3DS */
	unsigned int device_width;	/* x4, x8, x16 components */
	unsigned int rc;

	/* SDRAM device parameters */
	unsigned int n_row_addr;
	unsigned int n_col_addr;
	unsigned int edc_config;	/* 0 = none, 1 = parity, 2 = ECC */
	unsigned int bank_addr_bits;
	unsigned int bank_group_bits;
	unsigned int burst_lengths_bitmask;	/* BL=4 bit 2, BL=8 = bit 3 */

	/* mirrored DIMMs */
	unsigned int mirrored_dimm;	/* only for ddr3 */

	/* DIMM timing parameters */

	int mtb_ps;	/* medium timebase ps */
	int ftb_10th_ps; /* fine timebase, in 1/10 ps */
	int taa_ps;	/* minimum CAS latency time */
	int tfaw_ps;	/* four active window delay */

	/*
	 * SDRAM clock periods
	 * The range for these are 1000-10000 so a short should be sufficient
	 */
	int tckmin_x_ps;
	int tckmax_ps;

	/* SPD-defined CAS latencies */
	unsigned int caslat_x;

	/* basic timing parameters */
	int trcd_ps;
	int trp_ps;
	int tras_ps;

	int trfc1_ps;
	int trfc2_ps;
	int trfc4_ps;
	int trrds_ps;
	int trrdl_ps;
	int tccdl_ps;
	int trfc_slr_ps;

	int trc_ps;	/* maximum = 254 ns + .75 ns = 254750 ps */
	int twr_ps;	/* 15ns  for all speed bins */

	unsigned int refresh_rate_ps;
	unsigned int extended_op_srt;

	/* RDIMM */
	unsigned char rcw[16];	/* Register Control Word 0-15 */
	unsigned int dq_mapping[18];
	unsigned int dq_mapping_ors;
};

int read_spd(unsigned char chip, void *buf, int len);
int crc16(unsigned char *ptr, int count);
int cal_dimm_params(const struct ddr4_spd *spd, struct dimm_params *pdimm);

#endif /* DIMM_H */
