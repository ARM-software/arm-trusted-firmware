/*
 * Copyright (c) 2015-2021, Renesas Electronics Corporation.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <lib/mmio.h>
#include <common/debug.h>
#include "rcar_def.h"
#include "../ddr_regs.h"

#define RCAR_DDR_VERSION	"rev.0.02"

/* Average periodic refresh interval[ns]. Support 3900,7800 */
#define REFRESH_RATE  3900


#if RCAR_LSI != RCAR_D3
#error "Don't have DDR initialize routine."
#endif

static void init_ddr_d3_1866(void)
{
	uint32_t i, r2, r3, r5, r6, r7, r12;

	mmio_write_32(DBSC_DBSYSCNT0, 0x00001234);
	mmio_write_32(DBSC_DBKIND, 0x00000007);
	mmio_write_32(DBSC_DBMEMCONF_0_0, 0x0f030a01);
	mmio_write_32(DBSC_DBPHYCONF0, 0x00000001);
	mmio_write_32(DBSC_DBTR0, 0x0000000D);
	mmio_write_32(DBSC_DBTR1, 0x00000009);
	mmio_write_32(DBSC_DBTR2, 0x00000000);
	mmio_write_32(DBSC_DBTR3, 0x0000000D);
	mmio_write_32(DBSC_DBTR4, 0x000D000D);
	mmio_write_32(DBSC_DBTR5, 0x0000002D);
	mmio_write_32(DBSC_DBTR6, 0x00000020);
	mmio_write_32(DBSC_DBTR7, 0x00060006);
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
	mmio_write_32(DBSC_DBTR18, 0x00000200);
	mmio_write_32(DBSC_DBTR19, 0x0129004B);
	mmio_write_32(DBSC_DBTR20, 0x020000FB);
	mmio_write_32(DBSC_DBTR21, 0x00040004);
	mmio_write_32(DBSC_DBBL, 0x00000000);
	mmio_write_32(DBSC_DBODT0, 0x00000001);
	mmio_write_32(DBSC_DBADJ0, 0x00000001);
	mmio_write_32(DBSC_DBSYSCONF1, 0x00000002);
	mmio_write_32(DBSC_DBDFICNT_0, 0x00000010);
	mmio_write_32(DBSC_DBBCAMDIS, 0x00000001);
	mmio_write_32(DBSC_DBSCHRW1, 0x00000046);
	mmio_write_32(DBSC_SCFCTST0, 0x0C050B03);
	mmio_write_32(DBSC_SCFCTST1, 0x0305030C);

	mmio_write_32(DBSC_DBPDLK_0, 0x0000A55A);
	mmio_write_32(DBSC_DBCMD, 0x01000001);
	mmio_write_32(DBSC_DBCMD, 0x08000000);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x80010000);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000008);
	mmio_write_32(DBSC_DBPDRGD_0, 0x000B8000);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);
	mmio_write_32(DBSC_DBPDRGD_0, 0x04058A04);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000091);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BB6B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000095);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BBAD);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000099);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BB6B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);
	mmio_write_32(DBSC_DBPDRGD_0, 0x04058A00);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000021);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0024641E);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010073);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0C058A00);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);
	mmio_write_32(DBSC_DBPDRGD_0, 0x04058A00);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000003);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0780C700);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000007);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(30)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000004);
	mmio_write_32(DBSC_DBPDRGD_0,
		(uint32_t) (REFRESH_RATE * 928 / 125) - 400
			+ 0x0A300000);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000022);
	mmio_write_32(DBSC_DBPDRGD_0, 0x1000040B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000023);
	mmio_write_32(DBSC_DBPDRGD_0, 0x35A00D77);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000024);
	mmio_write_32(DBSC_DBPDRGD_0, 0x2A8A2C28);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000025);
	mmio_write_32(DBSC_DBPDRGD_0, 0x30005E00);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000026);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0014CB49);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000027);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00000F14);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000028);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00000046);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000029);
	if (REFRESH_RATE > 3900) {
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000020);
	} else {
		mmio_write_32(DBSC_DBPDRGD_0, 0x000000A0);
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

	mmio_write_32(DBSC_DBPDRGA_0, 0x0000000E);
	r2 = (mmio_read_32(DBSC_DBPDRGD_0) & 0x0000FF00) >> 0x9;
	r3 = (r2 << 16) + (r2 << 8) + r2;
	r6 = (r2 << 24) + (r2 << 16) + (r2 << 8) + r2;
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000011);
	mmio_write_32(DBSC_DBPDRGD_0, r3);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000012);
	mmio_write_32(DBSC_DBPDRGD_0, r3);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000016);
	mmio_write_32(DBSC_DBPDRGD_0, r6);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000017);
	mmio_write_32(DBSC_DBPDRGD_0, r6);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000018);
	mmio_write_32(DBSC_DBPDRGD_0, r6);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000019);
	mmio_write_32(DBSC_DBPDRGD_0, r6);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010181);
	mmio_write_32(DBSC_DBCMD, 0x08000001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010601);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	for (i = 0; i < 2; i++) {
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
						     ((r6 + (r5 << 1)) & 0xFF));
		}
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000005);
	mmio_write_32(DBSC_DBPDRGD_0, 0xC1AA00C0);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010801);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000005);
	mmio_write_32(DBSC_DBPDRGD_0, 0xC1AA00D8);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0001F001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x000000AF);
	r2 = mmio_read_32(DBSC_DBPDRGD_0);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000AF);
	mmio_write_32(DBSC_DBPDRGD_0, ((r2 + 0x1) & 0xFF) | (r2 & 0xFFFFFF00));
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000CF);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000CF);
	r2 = mmio_read_32(DBSC_DBPDRGD_0);
	mmio_write_32(DBSC_DBPDRGD_0, ((r2 + 0x1) & 0xFF) | (r2 & 0xFFFFFF00));

	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	mmio_write_32(DBSC_DBPDRGA_0, 0x0000002C);
	mmio_write_32(DBSC_DBPDRGD_0, 0x81003087);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010401);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	for (i = 0; i < 2; i++) {
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
			mmio_write_32(DBSC_DBPDRGD_0, r2 |
						     ((r6 + r5 +
						      (r5 >> 1) + r12) & 0xFF));
		}
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00015001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000003);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0380C700);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000007);
	while (mmio_read_32(DBSC_DBPDRGD_0) & BIT(30))
		;
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000021);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0024643E);

	mmio_write_32(DBSC_DBBUS0CNF1, 0x00000010);
	mmio_write_32(DBSC_DBCALCNF,
		(uint32_t) (64000000 / REFRESH_RATE) + 0x01000000);
	mmio_write_32(DBSC_DBRFCNF1,
		(uint32_t) (REFRESH_RATE * 116 / 125) + 0x00080000);
	mmio_write_32(DBSC_DBRFCNF2, 0x00010000);
	mmio_write_32(DBSC_DBDFICUPDCNF, 0x40100001);
	mmio_write_32(DBSC_DBRFEN, 0x00000001);
	mmio_write_32(DBSC_DBACEN, 0x00000001);
	mmio_write_32(DBSC_DBPDLK_0, 0x00000000);
	mmio_write_32(DBSC_DBSYSCNT0, 0x00000000);

#ifdef ddr_qos_init_setting // only for non qos_init
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
	mmio_write_32(DBSC_DBSCHQOS90, 0x00000300);
	mmio_write_32(DBSC_DBSCHQOS91, 0x000002F0);
	mmio_write_32(DBSC_DBSCHQOS92, 0x00000200);
	mmio_write_32(DBSC_DBSCHQOS93, 0x00000100);
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
	mmio_write_32(0xE67F0018, 0x00000001);
	mmio_write_32(DBSC_DBSYSCNT0, 0x00000000);
#endif
}

static void init_ddr_d3_1600(void)
{
	uint32_t i, r2, r3, r5, r6, r7, r12;

	mmio_write_32(CPG_CPGWPR, 0x5A5AFFFF);
	mmio_write_32(CPG_CPGWPCR, 0xA5A50000);

	mmio_write_32(CPG_SRCR4, 0x20000000);

	mmio_write_32(0xE61500DC, 0xe2200000);
	while (!(mmio_read_32(CPG_PLLECR) & BIT(11)))
		;

	mmio_write_32(CPG_SRSTCLR4, 0x20000000);

	mmio_write_32(CPG_CPGWPCR, 0xA5A50001);

	mmio_write_32(DBSC_DBSYSCNT0, 0x00001234);
	mmio_write_32(DBSC_DBKIND, 0x00000007);
	mmio_write_32(DBSC_DBMEMCONF_0_0, 0x0f030a01);
	mmio_write_32(DBSC_DBPHYCONF0, 0x00000001);
	mmio_write_32(DBSC_DBTR0, 0x0000000B);
	mmio_write_32(DBSC_DBTR1, 0x00000008);
	mmio_write_32(DBSC_DBTR2, 0x00000000);
	mmio_write_32(DBSC_DBTR3, 0x0000000B);
	mmio_write_32(DBSC_DBTR4, 0x000B000B);
	mmio_write_32(DBSC_DBTR5, 0x00000027);
	mmio_write_32(DBSC_DBTR6, 0x0000001C);
	mmio_write_32(DBSC_DBTR7, 0x00060006);
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
	mmio_write_32(DBSC_DBTR18, 0x00000200);
	mmio_write_32(DBSC_DBTR19, 0x01000040);
	mmio_write_32(DBSC_DBTR20, 0x020000D6);
	mmio_write_32(DBSC_DBTR21, 0x00040004);
	mmio_write_32(DBSC_DBBL, 0x00000000);
	mmio_write_32(DBSC_DBODT0, 0x00000001);
	mmio_write_32(DBSC_DBADJ0, 0x00000001);
	mmio_write_32(DBSC_DBSYSCONF1, 0x00000002);
	mmio_write_32(DBSC_DBDFICNT_0, 0x00000010);
	mmio_write_32(DBSC_DBBCAMDIS, 0x00000001);
	mmio_write_32(DBSC_DBSCHRW1, 0x00000046);
	mmio_write_32(DBSC_SCFCTST0, 0x0D050B03);
	mmio_write_32(DBSC_SCFCTST1, 0x0306030C);

	mmio_write_32(DBSC_DBPDLK_0, 0x0000A55A);
	mmio_write_32(DBSC_DBCMD, 0x01000001);
	mmio_write_32(DBSC_DBCMD, 0x08000000);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x80010000);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000008);
	mmio_write_32(DBSC_DBPDRGD_0, 0x000B8000);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);
	mmio_write_32(DBSC_DBPDRGD_0, 0x04058904);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000091);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BB6B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000095);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BBAD);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000099);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BB6B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);
	mmio_write_32(DBSC_DBPDRGD_0, 0x04058900);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000021);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0024641E);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010073);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0C058900);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000090);
	mmio_write_32(DBSC_DBPDRGD_0, 0x04058900);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000003);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0780C700);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000007);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(30)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000004);
	mmio_write_32(DBSC_DBPDRGD_0,
		(uint32_t) (REFRESH_RATE * 792 / 125) - 400 + 0x08B00000);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000022);
	mmio_write_32(DBSC_DBPDRGD_0, 0x1000040B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000023);
	mmio_write_32(DBSC_DBPDRGD_0, 0x2D9C0B66);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000024);
	mmio_write_32(DBSC_DBPDRGD_0, 0x2A88B400);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000025);
	mmio_write_32(DBSC_DBPDRGD_0, 0x30005200);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000026);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0014A9C9);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000027);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00000D70);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000028);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00000046);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000029);
	if (REFRESH_RATE > 3900) {
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000018);
	} else {
		mmio_write_32(DBSC_DBPDRGD_0, 0x00000098);
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

	mmio_write_32(DBSC_DBPDRGA_0, 0x0000000E);
	r2 = (mmio_read_32(DBSC_DBPDRGD_0) & 0x0000FF00) >> 0x9;
	r3 = (r2 << 16) + (r2 << 8) + r2;
	r6 = (r2 << 24) + (r2 << 16) + (r2 << 8) + r2;
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000011);
	mmio_write_32(DBSC_DBPDRGD_0, r3);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000012);
	mmio_write_32(DBSC_DBPDRGD_0, r3);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000016);
	mmio_write_32(DBSC_DBPDRGD_0, r6);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000017);
	mmio_write_32(DBSC_DBPDRGD_0, r6);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000018);
	mmio_write_32(DBSC_DBPDRGD_0, r6);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000019);
	mmio_write_32(DBSC_DBPDRGD_0, r6);

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010181);
	mmio_write_32(DBSC_DBCMD, 0x08000001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010601);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	for (i = 0; i < 2; i++) {
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
						     ((r6 + (r5 << 1)) & 0xFF));
		}
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000005);
	mmio_write_32(DBSC_DBPDRGD_0, 0xC1AA00C0);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010801);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000005);
	mmio_write_32(DBSC_DBPDRGD_0, 0xC1AA00D8);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0001F001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x000000AF);
	r2 = mmio_read_32(DBSC_DBPDRGD_0);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000AF);
	mmio_write_32(DBSC_DBPDRGD_0, ((r2 + 0x1) & 0xFF) | (r2 & 0xFFFFFF00));
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000CF);
	r2 = mmio_read_32(DBSC_DBPDRGD_0);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000CF);
	mmio_write_32(DBSC_DBPDRGD_0, ((r2 + 0x1) & 0xFF) | (r2 & 0xFFFFFF00));

	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C000285);
	mmio_write_32(DBSC_DBPDRGA_0, 0x0000002C);
	mmio_write_32(DBSC_DBPDRGD_0, 0x81003087);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010401);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	for (i = 0; i < 2; i++) {
		mmio_write_32(DBSC_DBPDRGA_0, 0xB1 + i * 0x20);
		r5 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFF00) >> 0x8;
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
			mmio_write_32(DBSC_DBPDRGD_0, r2 |
						     ((r6 + r5 +
						      (r5 >> 1) + r12) & 0xFF));
		}
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00015001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000003);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0380C700);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000007);
	while (mmio_read_32(DBSC_DBPDRGD_0) & BIT(30))
		;
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000021);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0024643E);

	mmio_write_32(DBSC_DBBUS0CNF1, 0x00000010);
	mmio_write_32(DBSC_DBCALCNF,
		(uint32_t) (64000000 / REFRESH_RATE) + 0x01000000);
	mmio_write_32(DBSC_DBRFCNF1,
		(uint32_t) (REFRESH_RATE * 99 / 125) + 0x00080000);
	mmio_write_32(DBSC_DBRFCNF2, 0x00010000);
	mmio_write_32(DBSC_DBDFICUPDCNF, 0x40100001);
	mmio_write_32(DBSC_DBRFEN, 0x00000001);
	mmio_write_32(DBSC_DBACEN, 0x00000001);
	mmio_write_32(DBSC_DBPDLK_0, 0x00000000);
	mmio_write_32(DBSC_DBSYSCNT0, 0x00000000);

#ifdef ddr_qos_init_setting // only for non qos_init
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
	mmio_write_32(DBSC_DBSCHQOS90, 0x00000300);
	mmio_write_32(DBSC_DBSCHQOS91, 0x000002F0);
	mmio_write_32(DBSC_DBSCHQOS92, 0x00000200);
	mmio_write_32(DBSC_DBSCHQOS93, 0x00000100);
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
	mmio_write_32(0xE67F0018, 0x00000001);
	mmio_write_32(DBSC_DBSYSCNT0, 0x00000000);
#endif
}

#define PRR			0xFFF00044U
#define PRR_PRODUCT_MASK	0x00007F00U
#define PRR_PRODUCT_D3		0x00005800U

#define	MODEMR_MD19		BIT(19)

int32_t rcar_dram_init(void)
{
	uint32_t reg;
	uint32_t ddr_mbps;

	reg = mmio_read_32(PRR);
	if ((reg & PRR_PRODUCT_MASK) != PRR_PRODUCT_D3) {
		ERROR("LSI Product ID (PRR=0x%x) DDR initialize not supported.\n",
		      reg);
		panic();
	}

	reg = mmio_read_32(RST_MODEMR);
	if (reg & MODEMR_MD19) {
		init_ddr_d3_1866();
		ddr_mbps = 1866;
	} else {
		init_ddr_d3_1600();
		ddr_mbps = 1600;
	}

	NOTICE("BL2: DDR%d(%s)\n", ddr_mbps, RCAR_DDR_VERSION);

	return 0;
}
