/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <stdint.h>

#include <common/debug.h>

#include "boot_init_dram.h"
#include "rcar_def.h"
#include "../ddr_regs.h"

#include "../dram_sub_func.h"

#define RCAR_E3_DDR_VERSION    "rev.0.12"

/* Average periodic refresh interval[ns]. Support 3900,7800 */
#ifdef ddr_qos_init_setting
#define REFRESH_RATE	3900U
#else
#if RCAR_REF_INT == 1
#define REFRESH_RATE	7800U
#else
#define REFRESH_RATE	3900U
#endif
#endif

/*
 *  Initialize ddr
 */
uint32_t init_ddr(void)
{
	uint32_t i, r2, r5, r6, r7, r12;
	uint32_t ddr_md;
	uint32_t regval, j;
	uint32_t dqsgd_0c, bdlcount_0c, bdlcount_0c_div2, bdlcount_0c_div4;
	uint32_t bdlcount_0c_div8, bdlcount_0c_div16;
	uint32_t gatesl_0c, rdqsd_0c, rdqsnd_0c, rbd_0c[4];
	uint32_t pdqsr_ctl, lcdl_ctl, lcdl_judge1, lcdl_judge2;
	uint32_t pdr_ctl;
	uint32_t byp_ctl;

	if ((mmio_read_32(0xFFF00044) & 0x000000FF) == 0x00000000) {
		pdqsr_ctl = 1;
		lcdl_ctl = 1;
		pdr_ctl = 1;
		byp_ctl = 1;
	} else {
		pdqsr_ctl = 0;
		lcdl_ctl = 0;
		pdr_ctl = 0;
		byp_ctl = 0;
	}

	/* Judge the DDR bit rate (ddr_md : 0 = 1584Mbps, 1 = 1856Mbps) */
	ddr_md = (mmio_read_32(RST_MODEMR) >> 19) & BIT(0);

	/* 1584Mbps setting */
	if (ddr_md == 0) {
		mmio_write_32(CPG_CPGWPR, 0x5A5AFFFF);
		mmio_write_32(CPG_CPGWPCR, 0xA5A50000);

		mmio_write_32(CPG_SRCR4, 0x20000000);

		mmio_write_32(0xE61500DC, 0xe2200000);	/* Change to 1584Mbps */
		while (!(mmio_read_32(CPG_PLLECR) & BIT(11)))
			;

		mmio_write_32(CPG_SRSTCLR4, 0x20000000);

		mmio_write_32(CPG_CPGWPCR, 0xA5A50001);
	}

	mmio_write_32(DBSC_DBSYSCNT0, 0x00001234);
	mmio_write_32(DBSC_DBKIND, 0x00000007);

#if RCAR_DRAM_DDR3L_MEMCONF == 0
	mmio_write_32(DBSC_DBMEMCONF_0_0, 0x0f030a02);	/* 1GB */
#else
	mmio_write_32(DBSC_DBMEMCONF_0_0, 0x10030a02);	/* 2GB(default) */
#endif

#if RCAR_DRAM_DDR3L_MEMDUAL == 1
	r2 = mmio_read_32(0xE6790614);
	mmio_write_32(0xE6790614, r2 | 0x3); /* MCS1_N/MODT1 are activated. */
#endif

	mmio_write_32(DBSC_DBPHYCONF0, 0x1);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_DBTR0, 0xB);
		mmio_write_32(DBSC_DBTR1, 0x8);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_DBTR0, 0xD);
		mmio_write_32(DBSC_DBTR1, 0x9);
	}

	mmio_write_32(DBSC_DBTR2, 0x00000000);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_DBTR3, 0x0000000B);
		mmio_write_32(DBSC_DBTR4, 0x000B000B);
		mmio_write_32(DBSC_DBTR5, 0x00000027);
		mmio_write_32(DBSC_DBTR6, 0x0000001C);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_DBTR3, 0x0000000D);
		mmio_write_32(DBSC_DBTR4, 0x000D000D);
		mmio_write_32(DBSC_DBTR5, 0x0000002D);
		mmio_write_32(DBSC_DBTR6, 0x00000020);
	}

	mmio_write_32(DBSC_DBTR7, 0x00060006);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_DBTR8, 0x00000020);
		mmio_write_32(DBSC_DBTR9, 0x00000006);
		mmio_write_32(DBSC_DBTR10, 0x0000000C);
		mmio_write_32(DBSC_DBTR11, 0x0000000A);
		mmio_write_32(DBSC_DBTR12, 0x00120012);
		mmio_write_32(DBSC_DBTR13, 0x000000CE);
		mmio_write_32(DBSC_DBTR14, 0x00140005);
		mmio_write_32(DBSC_DBTR15, 0x00050004);
		mmio_write_32(DBSC_DBTR16, 0x071F0305);
		mmio_write_32(DBSC_DBTR17, 0x040C0000);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_DBTR8, 0x00000021);
		mmio_write_32(DBSC_DBTR9, 0x00000007);
		mmio_write_32(DBSC_DBTR10, 0x0000000E);
		mmio_write_32(DBSC_DBTR11, 0x0000000C);
		mmio_write_32(DBSC_DBTR12, 0x00140014);
		mmio_write_32(DBSC_DBTR13, 0x000000F2);
		mmio_write_32(DBSC_DBTR14, 0x00170006);
		mmio_write_32(DBSC_DBTR15, 0x00060005);
		mmio_write_32(DBSC_DBTR16, 0x09210507);
		mmio_write_32(DBSC_DBTR17, 0x040E0000);
	}

	mmio_write_32(DBSC_DBTR18, 0x00000200);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_DBTR19, 0x01000040);
		mmio_write_32(DBSC_DBTR20, 0x020000D6);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_DBTR19, 0x0129004B);
		mmio_write_32(DBSC_DBTR20, 0x020000FB);
	}

	mmio_write_32(DBSC_DBTR21, 0x00040004);
	mmio_write_32(DBSC_DBBL, 0x00000000);
	mmio_write_32(DBSC_DBODT0, 0x00000001);
	mmio_write_32(DBSC_DBADJ0, 0x00000001);
	mmio_write_32(DBSC_DBSYSCONF1, 0x00000002);
	mmio_write_32(DBSC_DBDFICNT_0, 0x00000010);
	mmio_write_32(DBSC_DBBCAMDIS, 0x00000001);
	mmio_write_32(DBSC_DBSCHRW1, 0x00000046);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_SCFCTST0, 0x0D050B03);
		mmio_write_32(DBSC_SCFCTST1, 0x0306030C);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_SCFCTST0, 0x0C050B03);
		mmio_write_32(DBSC_SCFCTST1, 0x0305030C);
	}

	/*
	 * Initial_Step0( INITBYP )
	 */
	mmio_write_32(DBSC_DBPDLK_0, 0x0000A55A);
	mmio_write_32(DBSC_DBCMD, 0x01840001);
	mmio_write_32(DBSC_DBCMD, 0x08840000);
	NOTICE("BL2: [COLD_BOOT]\n");
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x80010000);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	/*
	 * Initial_Step1( ZCAL,PLLINIT,DCAL,PHYRST training )
	 */
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000008);
	mmio_write_32(DBSC_DBPDRGD_0, 0x000B8000);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x04058904);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x04058A04);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000091);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BB6B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000095);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BBAD);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000099);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BB6B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x04058900);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x04058A00);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000021);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0024641E);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010073);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	/*
	 * Initial_Step2( DRAMRST/DRAMINT training )
	 */
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x0C058900);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x0C058A00);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x04058900);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x04058A00);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000003);
	if (byp_ctl == 1)
		mmio_write_32(DBSC_DBPDRGD_0, 0x0780C720);
	else
		mmio_write_32(DBSC_DBPDRGD_0, 0x0780C700);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000007);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(30)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000004);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, (REFRESH_RATE * 792 / 125) -
					     400 + 0x08B00000);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, (REFRESH_RATE * 928 / 125) -
					     400 + 0x0A300000);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000022);
	mmio_write_32(DBSC_DBPDRGD_0, 0x1000040B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000023);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x2D9C0B66);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x35A00D77);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000024);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x2A88B400);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x2A8A2C28);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000025);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x30005200);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x30005E00);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000026);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x0014A9C9);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x0014CB49);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000027);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000D70);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000F14);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000028);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00000046);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000029);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		if (REFRESH_RATE > 3900)	/* [7]SRT=0 */
			mmio_write_32(DBSC_DBPDRGD_0, 0x18);
		else				/* [7]SRT=1 */
			mmio_write_32(DBSC_DBPDRGD_0, 0x98);
	} else {		/* 1856Mbps */
		if (REFRESH_RATE > 3900)	/* [7]SRT=0 */
			mmio_write_32(DBSC_DBPDRGD_0, 0x20);
		else				/* [7]SRT=1 */
			mmio_write_32(DBSC_DBPDRGD_0, 0xA0);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x0000002C);
	mmio_write_32(DBSC_DBPDRGD_0, 0x81003047);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000020);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00181884);
	mmio_write_32(DBSC_DBPDRGA_0, 0x0000001A);
	mmio_write_32(DBSC_DBPDRGD_0, 0x33C03C10);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A7);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A8);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A9);
	mmio_write_32(DBSC_DBPDRGD_0, 0x000D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C7);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C8);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C9);
	mmio_write_32(DBSC_DBPDRGD_0, 0x000D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000E7);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000E8);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000E9);
	mmio_write_32(DBSC_DBPDRGD_0, 0x000D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000107);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000108);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000109);
	mmio_write_32(DBSC_DBPDRGD_0, 0x000D0D0D);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010181);
	mmio_write_32(DBSC_DBCMD, 0x08840001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	/*
	 * Initial_Step3( WL/QSG training )
	 */
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010601);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	for (i = 0; i < 4; i++) {
		mmio_write_32(DBSC_DBPDRGA_0, 0xB1 + i * 0x20);
		r5 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFF00) >> 0x8;
		mmio_write_32(DBSC_DBPDRGA_0, 0xB4 + i * 0x20);
		r6 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFF;
		mmio_write_32(DBSC_DBPDRGA_0, 0xB3 + i * 0x20);
		r7 = mmio_read_32(DBSC_DBPDRGD_0) & 0x7;

		if (r6 > 0) {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFFF8;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | ((r7 + 0x1) & 0x7));
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFF00;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | r6);
		} else {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFFF8;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | r7);
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFF00;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 |
						     ((r6 + ((r5) << 1)) &
						     0xFF));
		}
	}

	/*
	 * Initial_Step4( WLADJ training )
	 */
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000005);
	mmio_write_32(DBSC_DBPDRGD_0, 0xC1AA00C0);

	if (pdqsr_ctl == 0) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	}

	/* PDR always off */
	if (pdr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000103);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010801);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	/*
	 * Initial_Step5(Read Data Bit Deskew)
	 */
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000005);
	mmio_write_32(DBSC_DBPDRGD_0, 0xC1AA00D8);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00011001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	if (pdqsr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	}

	/* PDR dynamic */
	if (pdr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000103);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
	}

	/*
	 * Initial_Step6(Write Data Bit Deskew)
	 */
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00012001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	/*
	 * Initial_Step7(Read Data Eye Training)
	 */
	if (pdqsr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	}

	/* PDR always off */
	if (pdr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000103);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00014001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	if (pdqsr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	}

	/* PDR dynamic */
	if (pdr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000103);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
	}

	/*
	 * Initial_Step8(Write Data Eye Training)
	 */
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00018001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	/*
	 * Initial_Step3_2( DQS Gate Training )
	 */
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	mmio_write_32(DBSC_DBPDRGA_0, 0x0000002C);
	mmio_write_32(DBSC_DBPDRGD_0, 0x81003087);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010401);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	for (i = 0; i < 4; i++) {
		mmio_write_32(DBSC_DBPDRGA_0, 0xB1 + i * 0x20);
		r5 = ((mmio_read_32(DBSC_DBPDRGD_0) & 0xFF00) >> 0x8);
		mmio_write_32(DBSC_DBPDRGA_0, 0xB4 + i * 0x20);
		r6 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFF;
		mmio_write_32(DBSC_DBPDRGA_0, 0xB3 + i * 0x20);
		r7 = mmio_read_32(DBSC_DBPDRGD_0) & 0x7;
		r12 = (r5 >> 0x2);
		if (r12 < r6) {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFFF8;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | ((r7 + 0x1) & 0x7));
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFF00;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | ((r6 - r12) & 0xFF));
		} else {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFFF8;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | (r7 & 0x7));
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFF00;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | ((r6 + r5 +
						     (r5 >> 1) + r12) & 0xFF));
		}
	}

	/*
	 * Initial_Step5-2_7-2( Rd bit Rd eye )
	 */
	if (pdqsr_ctl == 0) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	}

	/* PDR always off */
	if (pdr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000103);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00015001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	if (lcdl_ctl == 1) {
		for (i = 0; i < 4; i++) {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			dqsgd_0c = mmio_read_32(DBSC_DBPDRGD_0) & 0xFF;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB1 + i * 0x20);
			bdlcount_0c = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFF00) >>
					8;
			bdlcount_0c_div2 = bdlcount_0c >> 1;
			bdlcount_0c_div4 = bdlcount_0c >> 2;
			bdlcount_0c_div8 = bdlcount_0c >> 3;
			bdlcount_0c_div16 = bdlcount_0c >> 4;

			if (ddr_md == 0) {	/* 1584Mbps */
				lcdl_judge1 = bdlcount_0c_div2 +
					      bdlcount_0c_div4 +
					      bdlcount_0c_div8;
				lcdl_judge2 = bdlcount_0c +
					      bdlcount_0c_div4 +
					      bdlcount_0c_div16;
			} else {		/* 1856Mbps */
				lcdl_judge1 = bdlcount_0c_div2 +
					      bdlcount_0c_div4;
				lcdl_judge2 = bdlcount_0c +
					      bdlcount_0c_div4;
			}

			if (dqsgd_0c <= lcdl_judge1)
				continue;

			if (dqsgd_0c <= lcdl_judge2) {
				mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
				regval = mmio_read_32(DBSC_DBPDRGD_0) &
						0xFFFFFF00;
				mmio_write_32(DBSC_DBPDRGD_0,
					      (dqsgd_0c - bdlcount_0c_div8) |
					      regval);
			} else {
				mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
				regval = mmio_read_32(DBSC_DBPDRGD_0) &
						0xFFFFFF00;
				mmio_write_32(DBSC_DBPDRGD_0, regval);
				mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
				gatesl_0c = mmio_read_32(DBSC_DBPDRGD_0) & 0x7;
				mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
				regval = mmio_read_32(DBSC_DBPDRGD_0) &
						0xFFFFFFF8;
				mmio_write_32(DBSC_DBPDRGD_0, regval |
							     (gatesl_0c + 1));
				mmio_write_32(DBSC_DBPDRGA_0, 0xAF + i * 0x20);
				regval = (mmio_read_32(DBSC_DBPDRGD_0));
				rdqsd_0c = (regval & 0xFF00) >> 8;
				rdqsnd_0c = (regval & 0xFF0000) >> 16;
				mmio_write_32(DBSC_DBPDRGA_0, 0xAF + i * 0x20);
				mmio_write_32(DBSC_DBPDRGD_0,
					      (regval & 0xFF0000FF) |
					      ((rdqsd_0c +
						bdlcount_0c_div4) << 8) |
					      ((rdqsnd_0c +
						bdlcount_0c_div4) << 16));
				mmio_write_32(DBSC_DBPDRGA_0, 0xAA + i * 0x20);
				regval = (mmio_read_32(DBSC_DBPDRGD_0));
				rbd_0c[0] = (regval) & 0x1f;
				rbd_0c[1] = (regval >> 8) & 0x1f;
				rbd_0c[2] = (regval >> 16) & 0x1f;
				rbd_0c[3] = (regval >> 24) & 0x1f;
				mmio_write_32(DBSC_DBPDRGA_0, 0xAA + i * 0x20);
				regval = mmio_read_32(DBSC_DBPDRGD_0) &
					0xE0E0E0E0;
				for (j = 0; j < 4; j++) {
					rbd_0c[j] = rbd_0c[j] +
						    bdlcount_0c_div4;
					if (rbd_0c[j] > 0x1F)
						rbd_0c[j] = 0x1F;
					regval = regval | (rbd_0c[j] << 8 * j);
				}
				mmio_write_32(DBSC_DBPDRGD_0, regval);
				mmio_write_32(DBSC_DBPDRGA_0, 0xAB + i * 0x20);
				regval = (mmio_read_32(DBSC_DBPDRGD_0));
				rbd_0c[0] = (regval) & 0x1f;
				rbd_0c[1] = (regval >> 8) & 0x1f;
				rbd_0c[2] = (regval >> 16) & 0x1f;
				rbd_0c[3] = (regval >> 24) & 0x1f;
				mmio_write_32(DBSC_DBPDRGA_0, 0xAB + i * 0x20);
				regval = mmio_read_32(DBSC_DBPDRGD_0) &
					0xE0E0E0E0;
				for (j = 0; j < 4; j++) {
					rbd_0c[j] = rbd_0c[j] +
						    bdlcount_0c_div4;
					if (rbd_0c[j] > 0x1F)
						rbd_0c[j] = 0x1F;
					regval = regval | (rbd_0c[j] << 8 * j);
				}
				mmio_write_32(DBSC_DBPDRGD_0, regval);
			}
		}
		mmio_write_32(DBSC_DBPDRGA_0, 0x2);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7D81E37);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000003);
	if (byp_ctl == 1)
		mmio_write_32(DBSC_DBPDRGD_0, 0x0380C720);
	else
		mmio_write_32(DBSC_DBPDRGD_0, 0x0380C700);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000007);
	while (mmio_read_32(DBSC_DBPDRGD_0) & BIT(30))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000021);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0024643E);

	mmio_write_32(DBSC_DBBUS0CNF1, 0x00000010);
	mmio_write_32(DBSC_DBCALCNF, (64000000 / REFRESH_RATE) + 0x01000000);
	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_DBRFCNF1,
			      (REFRESH_RATE * 99 / 125) + 0x00080000);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_DBRFCNF1,
			      (REFRESH_RATE * 116 / 125) + 0x00080000);
	}

	mmio_write_32(DBSC_DBRFCNF2, 0x00010000);
	mmio_write_32(DBSC_DBDFICUPDCNF, 0x40100001);
	mmio_write_32(DBSC_DBRFEN, 0x00000001);
	mmio_write_32(DBSC_DBACEN, 0x00000001);

	if (pdqsr_ctl == 1) {
		mmio_write_32(0xE67F0018, 0x00000001);
		regval = mmio_read_32(0x40000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGD_0, regval);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	}

	/* PDR dynamic */
	if (pdr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000103);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
	}

	/*
	 * Initial_Step9( Initial End )
	 */
	mmio_write_32(DBSC_DBPDLK_0, 0x00000000);
	mmio_write_32(DBSC_DBSYSCNT0, 0x00000000);

#ifdef ddr_qos_init_setting /* only for non qos_init */
	mmio_write_32(DBSC_DBSYSCNT0, 0x00001234);
	mmio_write_32(DBSC_DBCAM0CNF1, 0x00043218);
	mmio_write_32(DBSC_DBCAM0CNF2, 0x000000F4);
	mmio_write_32(DBSC_DBSCHCNT0, 0x000f0037);
	mmio_write_32(DBSC_DBSCHSZ0, 0x00000001);
	mmio_write_32(DBSC_DBSCHRW0, 0x22421111);
	mmio_write_32(DBSC_SCFCTST2, 0x012F1123);
	mmio_write_32(DBSC_DBSCHQOS00, 0x00000F00);
	mmio_write_32(DBSC_DBSCHQOS01, 0x00000B00);
	mmio_write_32(DBSC_DBSCHQOS02, 0x00000000);
	mmio_write_32(DBSC_DBSCHQOS03, 0x00000000);
	mmio_write_32(DBSC_DBSCHQOS40, 0x00000300);
	mmio_write_32(DBSC_DBSCHQOS41, 0x000002F0);
	mmio_write_32(DBSC_DBSCHQOS42, 0x00000200);
	mmio_write_32(DBSC_DBSCHQOS43, 0x00000100);
	mmio_write_32(DBSC_DBSCHQOS90, 0x00000100);
	mmio_write_32(DBSC_DBSCHQOS91, 0x000000F0);
	mmio_write_32(DBSC_DBSCHQOS92, 0x000000A0);
	mmio_write_32(DBSC_DBSCHQOS93, 0x00000040);
	mmio_write_32(DBSC_DBSCHQOS130, 0x00000100);
	mmio_write_32(DBSC_DBSCHQOS131, 0x000000F0);
	mmio_write_32(DBSC_DBSCHQOS132, 0x000000A0);
	mmio_write_32(DBSC_DBSCHQOS133, 0x00000040);
	mmio_write_32(DBSC_DBSCHQOS140, 0x000000C0);
	mmio_write_32(DBSC_DBSCHQOS141, 0x000000B0);
	mmio_write_32(DBSC_DBSCHQOS142, 0x00000080);
	mmio_write_32(DBSC_DBSCHQOS143, 0x00000040);
	mmio_write_32(DBSC_DBSCHQOS150, 0x00000040);
	mmio_write_32(DBSC_DBSCHQOS151, 0x00000030);
	mmio_write_32(DBSC_DBSCHQOS152, 0x00000020);
	mmio_write_32(DBSC_DBSCHQOS153, 0x00000010);

	if (pdqsr_ctl == 0)
		mmio_write_32(0xE67F0018, 0x00000001);

	mmio_write_32(DBSC_DBSYSCNT0, 0x00000000);
#endif

	return 1;
}

static uint32_t recovery_from_backup_mode(uint32_t ddr_backup)
{
	/*
	 * recovery_Step0(DBSC Setting 1) / same "init_ddr"
	 */
	uint32_t r2, r5, r6, r7, r12, i;
	uint32_t ddr_md;
	uint32_t err;
	uint32_t regval, j;
	uint32_t dqsgd_0c, bdlcount_0c, bdlcount_0c_div2, bdlcount_0c_div4;
	uint32_t bdlcount_0c_div8, bdlcount_0c_div16;
	uint32_t gatesl_0c, rdqsd_0c, rdqsnd_0c, rbd_0c[4];
	uint32_t pdqsr_ctl, lcdl_ctl, lcdl_judge1, lcdl_judge2;
	uint32_t pdr_ctl;
	uint32_t byp_ctl;

	if ((mmio_read_32(0xFFF00044) & 0x000000FF) == 0x00000000) {
		pdqsr_ctl = 1;
		lcdl_ctl = 1;
		pdr_ctl = 1;
		byp_ctl = 1;
	} else {
		pdqsr_ctl = 0;
		lcdl_ctl = 0;
		pdr_ctl = 0;
		byp_ctl = 0;
	}

	/* Judge the DDR bit rate (ddr_md : 0 = 1584Mbps, 1 = 1856Mbps) */
	ddr_md = (mmio_read_32(RST_MODEMR) >> 19) & BIT(0);

	/* 1584Mbps setting */
	if (ddr_md == 0) {
		mmio_write_32(CPG_CPGWPR, 0x5A5AFFFF);
		mmio_write_32(CPG_CPGWPCR, 0xA5A50000);

		mmio_write_32(CPG_SRCR4, 0x20000000);

		mmio_write_32(0xE61500DC, 0xe2200000);	/* Change to 1584Mbps */
		while (!(mmio_read_32(CPG_PLLECR) & BIT(11)))
			;

		mmio_write_32(CPG_SRSTCLR4, 0x20000000);

		mmio_write_32(CPG_CPGWPCR, 0xA5A50001);
	}

	mmio_write_32(DBSC_DBSYSCNT0, 0x00001234);
	mmio_write_32(DBSC_DBKIND, 0x00000007);

#if RCAR_DRAM_DDR3L_MEMCONF == 0
	mmio_write_32(DBSC_DBMEMCONF_0_0, 0x0f030a02);
#else
	mmio_write_32(DBSC_DBMEMCONF_0_0, 0x10030a02);
#endif

#if RCAR_DRAM_DDR3L_MEMDUAL == 1
	r2 = mmio_read_32(0xE6790614);
	mmio_write_32(0xE6790614, r2 | 0x3); /* MCS1_N/MODT1 are activated. */
#endif

	mmio_write_32(DBSC_DBPHYCONF0, 0x00000001);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_DBTR0, 0x0000000B);
		mmio_write_32(DBSC_DBTR1, 0x00000008);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_DBTR0, 0x0000000D);
		mmio_write_32(DBSC_DBTR1, 0x00000009);
	}

	mmio_write_32(DBSC_DBTR2, 0x00000000);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_DBTR3, 0x0000000B);
		mmio_write_32(DBSC_DBTR4, 0x000B000B);
		mmio_write_32(DBSC_DBTR5, 0x00000027);
		mmio_write_32(DBSC_DBTR6, 0x0000001C);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_DBTR3, 0x0000000D);
		mmio_write_32(DBSC_DBTR4, 0x000D000D);
		mmio_write_32(DBSC_DBTR5, 0x0000002D);
		mmio_write_32(DBSC_DBTR6, 0x00000020);
	}

	mmio_write_32(DBSC_DBTR7, 0x00060006);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_DBTR8, 0x00000020);
		mmio_write_32(DBSC_DBTR9, 0x00000006);
		mmio_write_32(DBSC_DBTR10, 0x0000000C);
		mmio_write_32(DBSC_DBTR11, 0x0000000A);
		mmio_write_32(DBSC_DBTR12, 0x00120012);
		mmio_write_32(DBSC_DBTR13, 0x000000CE);
		mmio_write_32(DBSC_DBTR14, 0x00140005);
		mmio_write_32(DBSC_DBTR15, 0x00050004);
		mmio_write_32(DBSC_DBTR16, 0x071F0305);
		mmio_write_32(DBSC_DBTR17, 0x040C0000);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_DBTR8, 0x00000021);
		mmio_write_32(DBSC_DBTR9, 0x00000007);
		mmio_write_32(DBSC_DBTR10, 0x0000000E);
		mmio_write_32(DBSC_DBTR11, 0x0000000C);
		mmio_write_32(DBSC_DBTR12, 0x00140014);
		mmio_write_32(DBSC_DBTR13, 0x000000F2);
		mmio_write_32(DBSC_DBTR14, 0x00170006);
		mmio_write_32(DBSC_DBTR15, 0x00060005);
		mmio_write_32(DBSC_DBTR16, 0x09210507);
		mmio_write_32(DBSC_DBTR17, 0x040E0000);
	}

	mmio_write_32(DBSC_DBTR18, 0x00000200);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_DBTR19, 0x01000040);
		mmio_write_32(DBSC_DBTR20, 0x020000D6);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_DBTR19, 0x0129004B);
		mmio_write_32(DBSC_DBTR20, 0x020000FB);
	}

	mmio_write_32(DBSC_DBTR21, 0x00040004);
	mmio_write_32(DBSC_DBBL, 0x00000000);
	mmio_write_32(DBSC_DBODT0, 0x00000001);
	mmio_write_32(DBSC_DBADJ0, 0x00000001);
	mmio_write_32(DBSC_DBSYSCONF1, 0x00000002);
	mmio_write_32(DBSC_DBDFICNT_0, 0x00000010);
	mmio_write_32(DBSC_DBBCAMDIS, 0x00000001);
	mmio_write_32(DBSC_DBSCHRW1, 0x00000046);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_SCFCTST0, 0x0D050B03);
		mmio_write_32(DBSC_SCFCTST1, 0x0306030C);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_SCFCTST0, 0x0C050B03);
		mmio_write_32(DBSC_SCFCTST1, 0x0305030C);
	}

	/*
	 * recovery_Step1(PHY setting 1)
	 */
	mmio_write_32(DBSC_DBPDLK_0, 0x0000A55A);
	mmio_write_32(DBSC_DBCMD, 0x01840001);
	mmio_write_32(DBSC_DBCMD, 0x0A840000);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000008);	/* DDR_PLLCR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x000B8000);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000003);	/* DDR_PGCR1 */
	if (byp_ctl == 1)
		mmio_write_32(DBSC_DBPDRGD_0, 0x0780C720);
	else
		mmio_write_32(DBSC_DBPDRGD_0, 0x0780C700);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000020);	/* DDR_DXCCR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x00181884);
	mmio_write_32(DBSC_DBPDRGA_0, 0x0000001A);	/* DDR_ACIOCR0 */
	mmio_write_32(DBSC_DBPDRGD_0, 0x33C03C10);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000007);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(30)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000004);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, (REFRESH_RATE * 792 / 125) -
					     400 + 0x08B00000);
	} else {		/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, (REFRESH_RATE * 928 / 125) -
					     400 + 0x0A300000);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000022);
	mmio_write_32(DBSC_DBPDRGD_0, 0x1000040B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000023);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x2D9C0B66);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x35A00D77);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000024);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x2A88B400);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x2A8A2C28);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000025);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x30005200);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x30005E00);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000026);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x0014A9C9);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x0014CB49);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000027);

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000D70);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000F14);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000028);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00000046);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000029);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		if (REFRESH_RATE > 3900)
			mmio_write_32(DBSC_DBPDRGD_0, 0x18);	/* [7]SRT=0 */
		else
			mmio_write_32(DBSC_DBPDRGD_0, 0x98);	/* [7]SRT=1 */
	} else {	/* 1856Mbps */
		if (REFRESH_RATE > 3900)
			mmio_write_32(DBSC_DBPDRGD_0, 0x20);	/* [7]SRT=0 */
		else
			mmio_write_32(DBSC_DBPDRGD_0, 0xA0);	/* [7]SRT=1 */
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x0000002C);
	mmio_write_32(DBSC_DBPDRGD_0, 0x81003047);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000091);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BB6B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000095);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BBAD);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000099);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BB6B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000021);	/* DDR_DSGCR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x0024641E);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);	/* DDR_PGSR0 */
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);	/* DDR_PIR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x40010000);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);	/* DDR_PGSR0 */
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000092);	/* DDR_ZQ0DR */
	mmio_write_32(DBSC_DBPDRGD_0, 0xC2C59AB5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000096);	/* DDR_ZQ1DR */
	mmio_write_32(DBSC_DBPDRGD_0, 0xC4285FBF);
	mmio_write_32(DBSC_DBPDRGA_0, 0x0000009A);	/* DDR_ZQ2DR */
	mmio_write_32(DBSC_DBPDRGD_0, 0xC2C59AB5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);	/* DDR_ZQCR */

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x0C058900);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x0C058A00);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);	/* DDR_ZQCR */

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x04058900);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x04058A00);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);	/* DDR_PIR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x00050001);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);	/* DDR_PGSR0 */
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	/* ddr backupmode end */
	if (ddr_backup)
		NOTICE("BL2: [WARM_BOOT]\n");
	else
		NOTICE("BL2: [COLD_BOOT]\n");

	err = rcar_dram_update_boot_status(ddr_backup);
	if (err) {
		NOTICE("BL2: [BOOT_STATUS_UPDATE_ERROR]\n");
		return INITDRAM_ERR_I;
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000092);	/* DDR_ZQ0DR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x02C59AB5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000096);	/* DDR_ZQ1DR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x04285FBF);
	mmio_write_32(DBSC_DBPDRGA_0, 0x0000009A);	/* DDR_ZQ2DR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x02C59AB5);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);	/* DDR_PIR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x08000000);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);	/* DDR_PIR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x00000003);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);	/* DDR_PGSR0 */
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);	/* DDR_PIR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x80010000);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);	/* DDR_PGSR0 */
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);	/* DDR_PIR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010073);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);	/* DDR_PGSR0 */
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);	/* DDR_ZQCR */

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x0C058900);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x0C058A00);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);	/* DDR_ZQCR */

	/* Select setting value in bps */
	if (ddr_md == 0)	/* 1584Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x04058900);
	else			/* 1856Mbps */
		mmio_write_32(DBSC_DBPDRGD_0, 0x04058A00);

	mmio_write_32(DBSC_DBPDRGA_0, 0x0000000C);
	mmio_write_32(DBSC_DBPDRGD_0, 0x18000040);

	/*
	 * recovery_Step2(PHY setting 2)
	 */
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A7);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A8);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A9);
	mmio_write_32(DBSC_DBPDRGD_0, 0x000D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C7);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C8);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C9);
	mmio_write_32(DBSC_DBPDRGD_0, 0x000D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000E7);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000E8);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000E9);
	mmio_write_32(DBSC_DBPDRGD_0, 0x000D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000107);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000108);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0D0D0D0D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000109);
	mmio_write_32(DBSC_DBPDRGD_0, 0x000D0D0D);

	mmio_write_32(DBSC_DBCALCNF, (64000000 / REFRESH_RATE) + 0x01000000);
	mmio_write_32(DBSC_DBBUS0CNF1, 0x00000010);

	/* Select setting value in bps */
	if (ddr_md == 0) {	/* 1584Mbps */
		mmio_write_32(DBSC_DBRFCNF1,
			      (REFRESH_RATE * 99 / 125) + 0x00080000);
	} else {			/* 1856Mbps */
		mmio_write_32(DBSC_DBRFCNF1,
			      (REFRESH_RATE * 116 / 125) + 0x00080000);
	}

	mmio_write_32(DBSC_DBRFCNF2, 0x00010000);
	mmio_write_32(DBSC_DBRFEN, 0x00000001);
	mmio_write_32(DBSC_DBCMD, 0x0A840001);
	while (mmio_read_32(DBSC_DBWAIT) & BIT(0))
		;

	mmio_write_32(DBSC_DBCMD, 0x00000000);

	mmio_write_32(DBSC_DBCMD, 0x04840010);
	while (mmio_read_32(DBSC_DBWAIT) & BIT(0))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);	/* DDR_PGSR0 */
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);	/* DDR_PIR */
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010701);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);	/* DDR_PGSR0 */
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	for (i = 0; i < 4; i++) {
		mmio_write_32(DBSC_DBPDRGA_0, 0xB1 + i * 0x20);
		r5 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFF00) >> 0x8;
		mmio_write_32(DBSC_DBPDRGA_0, 0xB4 + i * 0x20);
		r6 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFF;
		mmio_write_32(DBSC_DBPDRGA_0, 0xB3 + i * 0x20);
		r7 = mmio_read_32(DBSC_DBPDRGD_0) & 0x7;

		if (r6 > 0) {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFFF8;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | ((r7 + 0x1) & 0x7));
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFF00;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | r6);
		} else {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFFF8;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | r7);
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFF00;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0,
				      r2 | ((r6 + (r5 << 1)) & 0xFF));
		}
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000005);
	mmio_write_32(DBSC_DBPDRGD_0, 0xC1AA00C0);

	if (pdqsr_ctl == 0) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	}

	/* PDR always off */
	if (pdr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000103);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010801);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000005);
	mmio_write_32(DBSC_DBPDRGD_0, 0xC1AA00D8);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00011001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	if (pdqsr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	}

	/* PDR dynamic */
	if (pdr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000103);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00012001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	if (pdqsr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	}

	/* PDR always off */
	if (pdr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000103);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00014001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	if (pdqsr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	}

	/* PDR dynamic */
	if (pdr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000103);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00018001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	mmio_write_32(DBSC_DBPDRGA_0, 0x0000002C);
	mmio_write_32(DBSC_DBPDRGD_0, 0x81003087);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010401);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	for (i = 0; i < 4; i++) {
		mmio_write_32(DBSC_DBPDRGA_0, 0xB1 + i * 0x20);
		r5 = ((mmio_read_32(DBSC_DBPDRGD_0) & 0xFF00) >> 0x8);
		mmio_write_32(DBSC_DBPDRGA_0, 0xB4 + i * 0x20);
		r6 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFF;
		mmio_write_32(DBSC_DBPDRGA_0, 0xB3 + i * 0x20);
		r7 = mmio_read_32(DBSC_DBPDRGD_0) & 0x7;
		r12 = r5 >> 0x2;

		if (r12 < r6) {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFFF8;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | ((r7 + 0x1) & 0x7));
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFF00;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | ((r6 - r12) & 0xFF));
		} else {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFFF8;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | (r7 & 0x7));
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			r2 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFF00;
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0,
				      r2 |
				      ((r6 + r5 + (r5 >> 1) + r12) & 0xFF));
		}
	}

	if (pdqsr_ctl == 0) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	}

	/* PDR always off */
	if (pdr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000103);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000008);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00015001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	if (lcdl_ctl == 1) {
		for (i = 0; i < 4; i++) {
			mmio_write_32(DBSC_DBPDRGA_0, 0x000000B0 + i * 0x20);
			dqsgd_0c = mmio_read_32(DBSC_DBPDRGD_0) & 0x000000FF;
			mmio_write_32(DBSC_DBPDRGA_0, 0x000000B1 + i * 0x20);
			bdlcount_0c = (mmio_read_32(DBSC_DBPDRGD_0) &
					0x0000FF00) >> 8;
			bdlcount_0c_div2 = (bdlcount_0c >> 1);
			bdlcount_0c_div4 = (bdlcount_0c >> 2);
			bdlcount_0c_div8 = (bdlcount_0c >> 3);
			bdlcount_0c_div16 = (bdlcount_0c >> 4);

			if (ddr_md == 0) {	/* 1584Mbps */
				lcdl_judge1 = bdlcount_0c_div2 +
					      bdlcount_0c_div4 +
					      bdlcount_0c_div8;
				lcdl_judge2 = bdlcount_0c +
					      bdlcount_0c_div4 +
					      bdlcount_0c_div16;
			} else {	/* 1856Mbps */
				lcdl_judge1 = bdlcount_0c_div2 +
					      bdlcount_0c_div4;
				lcdl_judge2 = bdlcount_0c +
					      bdlcount_0c_div4;
			}

			if (dqsgd_0c <= lcdl_judge1)
				continue;

			if (dqsgd_0c <= lcdl_judge2) {
				mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
				regval = mmio_read_32(DBSC_DBPDRGD_0) &
						0xFFFFFF00;
				mmio_write_32(DBSC_DBPDRGD_0,
					      (dqsgd_0c - bdlcount_0c_div8) |
					      regval);
			} else {
				mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
				regval = mmio_read_32(DBSC_DBPDRGD_0) &
						0xFFFFFF00;
				mmio_write_32(DBSC_DBPDRGD_0, regval);
				mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
				gatesl_0c = mmio_read_32(DBSC_DBPDRGD_0) & 0x7;
				mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
				regval = mmio_read_32(DBSC_DBPDRGD_0) &
						0xFFFFFFF8;
				mmio_write_32(DBSC_DBPDRGD_0,
					      regval | (gatesl_0c + 1));
				mmio_write_32(DBSC_DBPDRGA_0, 0xAF + i * 0x20);
				regval = mmio_read_32(DBSC_DBPDRGD_0);
				rdqsd_0c = (regval & 0xFF00) >> 8;
				rdqsnd_0c = (regval & 0xFF0000) >> 16;
				mmio_write_32(DBSC_DBPDRGA_0, 0xAF + i * 0x20);
				mmio_write_32(DBSC_DBPDRGD_0,
					      (regval & 0xFF0000FF) |
					      ((rdqsd_0c +
						bdlcount_0c_div4) << 8) |
					      ((rdqsnd_0c +
						bdlcount_0c_div4) << 16));
				mmio_write_32(DBSC_DBPDRGA_0, 0xAA + i * 0x20);
				regval = (mmio_read_32(DBSC_DBPDRGD_0));
				rbd_0c[0] = (regval) & 0x1f;
				rbd_0c[1] = (regval >>  8) & 0x1f;
				rbd_0c[2] = (regval >> 16) & 0x1f;
				rbd_0c[3] = (regval >> 24) & 0x1f;
				mmio_write_32(DBSC_DBPDRGA_0, 0xAA + i * 0x20);
				regval = mmio_read_32(DBSC_DBPDRGD_0) &
						0xE0E0E0E0;
				for (j = 0; j < 4; j++) {
					rbd_0c[j] = rbd_0c[j] +
						    bdlcount_0c_div4;
					if (rbd_0c[j] > 0x1F)
						rbd_0c[j] = 0x1F;
					regval = regval | (rbd_0c[j] << 8 * j);
				}
				mmio_write_32(DBSC_DBPDRGD_0, regval);
				mmio_write_32(DBSC_DBPDRGA_0, 0xAB + i * 0x20);
				regval = (mmio_read_32(DBSC_DBPDRGD_0));
				rbd_0c[0] = regval & 0x1f;
				rbd_0c[1] = (regval >> 8) & 0x1f;
				rbd_0c[2] = (regval >> 16) & 0x1f;
				rbd_0c[3] = (regval >> 24) & 0x1f;
				mmio_write_32(DBSC_DBPDRGA_0, 0xAB + i * 0x20);
				regval = mmio_read_32(DBSC_DBPDRGD_0) &
						0xE0E0E0E0;
				for (j = 0; j < 4; j++) {
					rbd_0c[j] = rbd_0c[j] +
						    bdlcount_0c_div4;
					if (rbd_0c[j] > 0x1F)
						rbd_0c[j] = 0x1F;
					regval = regval | (rbd_0c[j] << 8 * j);
				}
				mmio_write_32(DBSC_DBPDRGD_0, regval);
			}
		}
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000002);
		mmio_write_32(DBSC_DBPDRGD_0, 0x07D81E37);
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000003);
	if (byp_ctl == 1)
		mmio_write_32(DBSC_DBPDRGD_0, 0x0380C720);
	else
		mmio_write_32(DBSC_DBPDRGD_0, 0x0380C700);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000007);
	while (mmio_read_32(DBSC_DBPDRGD_0) & BIT(30))
		;
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000021);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0024643E);

	/*
	 * recovery_Step3(DBSC Setting 2)
	 */
	mmio_write_32(DBSC_DBDFICUPDCNF, 0x40100001);
	mmio_write_32(DBSC_DBACEN, 0x00000001);

	if (pdqsr_ctl == 1) {
		mmio_write_32(0xE67F0018, 0x00000001);
		regval = mmio_read_32(0x40000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGD_0, regval);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
		mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	}

	/* PDR dynamic */
	if (pdr_ctl == 1) {
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000A3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000C3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x000000E3);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
		mmio_write_32(DBSC_DBPDRGA_0, 0x00000103);
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000000);
	}

	mmio_write_32(DBSC_DBPDLK_0, 0x00000000);
	mmio_write_32(DBSC_DBSYSCNT0, 0x00000000);

#ifdef ddr_qos_init_setting /* only for non qos_init */
	mmio_write_32(DBSC_DBSYSCNT0, 0x00001234);
	mmio_write_32(DBSC_DBCAM0CNF1, 0x00043218);
	mmio_write_32(DBSC_DBCAM0CNF2, 0x000000F4);
	mmio_write_32(DBSC_DBSCHCNT0, 0x000f0037);
	mmio_write_32(DBSC_DBSCHSZ0, 0x00000001);
	mmio_write_32(DBSC_DBSCHRW0, 0x22421111);
	mmio_write_32(DBSC_SCFCTST2, 0x012F1123);
	mmio_write_32(DBSC_DBSCHQOS00, 0x00000F00);
	mmio_write_32(DBSC_DBSCHQOS01, 0x00000B00);
	mmio_write_32(DBSC_DBSCHQOS02, 0x00000000);
	mmio_write_32(DBSC_DBSCHQOS03, 0x00000000);
	mmio_write_32(DBSC_DBSCHQOS40, 0x00000300);
	mmio_write_32(DBSC_DBSCHQOS41, 0x000002F0);
	mmio_write_32(DBSC_DBSCHQOS42, 0x00000200);
	mmio_write_32(DBSC_DBSCHQOS43, 0x00000100);
	mmio_write_32(DBSC_DBSCHQOS90, 0x00000100);
	mmio_write_32(DBSC_DBSCHQOS91, 0x000000F0);
	mmio_write_32(DBSC_DBSCHQOS92, 0x000000A0);
	mmio_write_32(DBSC_DBSCHQOS93, 0x00000040);
	mmio_write_32(DBSC_DBSCHQOS130, 0x00000100);
	mmio_write_32(DBSC_DBSCHQOS131, 0x000000F0);
	mmio_write_32(DBSC_DBSCHQOS132, 0x000000A0);
	mmio_write_32(DBSC_DBSCHQOS133, 0x00000040);
	mmio_write_32(DBSC_DBSCHQOS140, 0x000000C0);
	mmio_write_32(DBSC_DBSCHQOS141, 0x000000B0);
	mmio_write_32(DBSC_DBSCHQOS142, 0x00000080);
	mmio_write_32(DBSC_DBSCHQOS143, 0x00000040);
	mmio_write_32(DBSC_DBSCHQOS150, 0x00000040);
	mmio_write_32(DBSC_DBSCHQOS151, 0x00000030);
	mmio_write_32(DBSC_DBSCHQOS152, 0x00000020);
	mmio_write_32(DBSC_DBSCHQOS153, 0x00000010);

	if (pdqsr_ctl == 0)
		mmio_write_32(0xE67F0018, 0x00000001);

	mmio_write_32(DBSC_DBSYSCNT0, 0x00000000);
#endif

	return 1;

} /* recovery_from_backup_mode */

/*
 * init_ddr : MD19=0,DDR3L,1584Mbps / MD19=1,DDR3L,1856Mbps
 */

/*
 * DDR Initialize entry for IPL
 */
int32_t rcar_dram_init(void)
{
	uint32_t dataL;
	uint32_t failcount;
	uint32_t md = 0;
	uint32_t ddr = 0;
	uint32_t ddr_backup;

	md = *((volatile uint32_t*)RST_MODEMR);
	ddr = (md & 0x00080000) >> 19;
	if (ddr == 0x0)
		NOTICE("BL2: DDR1584(%s)\n", RCAR_E3_DDR_VERSION);
	else if (ddr == 0x1)
		NOTICE("BL2: DDR1856(%s)\n", RCAR_E3_DDR_VERSION);

	rcar_dram_get_boot_status(&ddr_backup);

	if (ddr_backup == DRAM_BOOT_STATUS_WARM)
		dataL = recovery_from_backup_mode(ddr_backup);	/* WARM boot */
	else
		dataL = init_ddr();				/* COLD boot */

	if (dataL == 1)
		failcount = 0;
	else
		failcount = 1;

	if (failcount == 0)
		return INITDRAM_OK;
	else
		return INITDRAM_NG;

}
