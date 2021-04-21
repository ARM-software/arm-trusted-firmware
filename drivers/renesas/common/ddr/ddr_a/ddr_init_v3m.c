/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <stdint.h>
#include "boot_init_dram.h"
#include "rcar_def.h"
#include "../ddr_regs.h"

static uint32_t init_ddr_v3m_1600(void)
{
	uint32_t i, r2, r5, r6, r7, r12;

	mmio_write_32(DBSC_DBSYSCNT0, 0x00001234);
	mmio_write_32(DBSC_DBKIND, 0x00000007);
#if RCAR_DRAM_DDR3L_MEMCONF == 0
	mmio_write_32(DBSC_DBMEMCONF_0_0, 0x0f030a02); // 1GB: Eagle
#else
	mmio_write_32(DBSC_DBMEMCONF_0_0, 0x10030a02); // 2GB: V3MSK
#endif
	mmio_write_32(DBSC_DBPHYCONF0, 0x00000001);
	mmio_write_32(DBSC_DBTR0, 0x0000000B);
	mmio_write_32(DBSC_DBTR1, 0x00000008);
	mmio_write_32(DBSC_DBTR3, 0x0000000B);
	mmio_write_32(DBSC_DBTR4, 0x000B000B);
	mmio_write_32(DBSC_DBTR5, 0x00000027);
	mmio_write_32(DBSC_DBTR6, 0x0000001C);
	mmio_write_32(DBSC_DBTR7, 0x00060006);
	mmio_write_32(DBSC_DBTR8, 0x00000020);
	mmio_write_32(DBSC_DBTR9, 0x00000006);
	mmio_write_32(DBSC_DBTR10, 0x0000000C);
	mmio_write_32(DBSC_DBTR11, 0x0000000B);
	mmio_write_32(DBSC_DBTR12, 0x00120012);
	mmio_write_32(DBSC_DBTR13, 0x01180118);
	mmio_write_32(DBSC_DBTR14, 0x00140005);
	mmio_write_32(DBSC_DBTR15, 0x00050004);
	mmio_write_32(DBSC_DBTR16, 0x071D0305);
	mmio_write_32(DBSC_DBTR17, 0x040C0010);
	mmio_write_32(DBSC_DBTR18, 0x00000200);
	mmio_write_32(DBSC_DBTR19, 0x01000040);
	mmio_write_32(DBSC_DBTR20, 0x02000120);
	mmio_write_32(DBSC_DBTR21, 0x00040004);
	mmio_write_32(DBSC_DBBL, 0x00000000);
	mmio_write_32(DBSC_DBODT0, 0x00000001);
	mmio_write_32(DBSC_DBADJ0, 0x00000001);
	mmio_write_32(DBSC_DBCAM0CNF1, 0x00082010);
	mmio_write_32(DBSC_DBCAM0CNF2, 0x00002000);
	mmio_write_32(DBSC_DBSCHCNT0, 0x080f003f);
	mmio_write_32(DBSC_DBSCHCNT1, 0x00001010);
	mmio_write_32(DBSC_DBSCHSZ0, 0x00000001);
	mmio_write_32(DBSC_DBSCHRW0, 0x00000200);
	mmio_write_32(DBSC_DBSCHRW1, 0x00000040);
	mmio_write_32(DBSC_DBSCHQOS40, 0x00000600);
	mmio_write_32(DBSC_DBSCHQOS41, 0x00000480);
	mmio_write_32(DBSC_DBSCHQOS42, 0x00000300);
	mmio_write_32(DBSC_DBSCHQOS43, 0x00000180);
	mmio_write_32(DBSC_DBSCHQOS90, 0x00000400);
	mmio_write_32(DBSC_DBSCHQOS91, 0x00000300);
	mmio_write_32(DBSC_DBSCHQOS92, 0x00000200);
	mmio_write_32(DBSC_DBSCHQOS93, 0x00000100);
	mmio_write_32(DBSC_DBSCHQOS130, 0x00000300);
	mmio_write_32(DBSC_DBSCHQOS131, 0x00000240);
	mmio_write_32(DBSC_DBSCHQOS132, 0x00000180);
	mmio_write_32(DBSC_DBSCHQOS133, 0x000000c0);
	mmio_write_32(DBSC_DBSCHQOS140, 0x00000200);
	mmio_write_32(DBSC_DBSCHQOS141, 0x00000180);
	mmio_write_32(DBSC_DBSCHQOS142, 0x00000100);
	mmio_write_32(DBSC_DBSCHQOS143, 0x00000080);
	mmio_write_32(DBSC_DBSCHQOS150, 0x00000100);
	mmio_write_32(DBSC_DBSCHQOS151, 0x000000c0);
	mmio_write_32(DBSC_DBSCHQOS152, 0x00000080);
	mmio_write_32(DBSC_DBSCHQOS153, 0x00000040);
	mmio_write_32(DBSC_DBSYSCONF1, 0x00000002);
	mmio_write_32(DBSC_DBCAM0CNF1, 0x00040C04);
	mmio_write_32(DBSC_DBCAM0CNF2, 0x000001c4);
	mmio_write_32(DBSC_DBSCHSZ0, 0x00000003);
	mmio_write_32(DBSC_DBSCHRW1, 0x001a0080);
	mmio_write_32(DBSC_DBDFICNT_0, 0x00000010);

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
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BB6D);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000095);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BB6B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000099);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0007BB6D);
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
	mmio_write_32(DBSC_DBPDRGD_0, 0x08C0C170);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000022);
	mmio_write_32(DBSC_DBPDRGD_0, 0x1000040B);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000023);
	mmio_write_32(DBSC_DBPDRGD_0, 0x2D9C0B66);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000024);
	mmio_write_32(DBSC_DBPDRGD_0, 0x2A88C400);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000025);
	mmio_write_32(DBSC_DBPDRGD_0, 0x30005200);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000026);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0014A9C9);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000027);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00000D70);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000028);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00000004);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000029);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00000018);
	mmio_write_32(DBSC_DBPDRGA_0, 0x0000002C);
	mmio_write_32(DBSC_DBPDRGD_0, 0x81003047);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000020);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00181884);
	mmio_write_32(DBSC_DBPDRGA_0, 0x0000001A);
	mmio_write_32(DBSC_DBPDRGD_0, 0x13C03C10);
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
	mmio_write_32(DBSC_DBCMD, 0x08000001);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010601);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	for (i = 0; i < 4; i++) {
		mmio_write_32(DBSC_DBPDRGA_0, 0xB1 + i * 0x20);
		r5 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFF00) >> 8;
		mmio_write_32(DBSC_DBPDRGA_0, 0xB4 + i * 0x20);
		r6 = mmio_read_32(DBSC_DBPDRGD_0) & 0xFF;
		mmio_write_32(DBSC_DBPDRGA_0, 0xB3 + i * 0x20);
		r7 = mmio_read_32(DBSC_DBPDRGD_0) & 0x7;

		if (r6 > 0) {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			r2 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFFF8);

			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, ((r7 + 1) & 0x7) | r2);
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			r2 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFF00);
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | r6);
		} else {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			r2 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFFF8);
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 | r7);

			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			r2 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFF00);
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, r2 |
						     (((r5 << 1) + r6) & 0xFF));
		}
	}

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000005);
	mmio_write_32(DBSC_DBPDRGD_0, 0xC1AA00A0);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000A0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000C0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x00010801);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000006);
	while (!(mmio_read_32(DBSC_DBPDRGD_0) & BIT(0)))
		;

	mmio_write_32(DBSC_DBPDRGA_0, 0x00000005);
	mmio_write_32(DBSC_DBPDRGD_0, 0xC1AA00B8);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000001);
	mmio_write_32(DBSC_DBPDRGD_0, 0x0001F001);
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
		r5 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFF00) >> 8;
		mmio_write_32(DBSC_DBPDRGA_0, 0xB4 + i * 0x20);
		r6 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFF);

		mmio_write_32(DBSC_DBPDRGA_0, 0xB3 + i * 0x20);
		r7 = (mmio_read_32(DBSC_DBPDRGD_0) & 0x7);
		r12 = (r5 >> 2);
		if (r6 - r12 > 0) {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			r2 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFFF8);

			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, ((r7 + 1) & 0x7) | r2);
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			r2 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFF00);

			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, ((r6 - r12) & 0xFF) | r2);
		} else {
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			r2 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFFF8);
			mmio_write_32(DBSC_DBPDRGA_0, 0xB2 + i * 0x20);
			mmio_write_32(DBSC_DBPDRGD_0, (r7 & 0x7) | r2);
			mmio_write_32(DBSC_DBPDRGA_0, 0xB0 + i * 0x20);
			r2 = (mmio_read_32(DBSC_DBPDRGD_0) & 0xFFFFFF00);
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
	mmio_write_32(DBSC_DBPDRGA_0, 0x000000E0);
	mmio_write_32(DBSC_DBPDRGD_0, 0x7C0002C5);
	mmio_write_32(DBSC_DBPDRGA_0, 0x00000100);
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

	mmio_write_32(DBSC_DBBUS0CNF1, 0x00000000);
	mmio_write_32(DBSC_DBBUS0CNF0, 0x00010001);
	mmio_write_32(DBSC_DBCALCNF, 0x0100200E);
	mmio_write_32(DBSC_DBRFCNF1, 0x00081860);
	mmio_write_32(DBSC_DBRFCNF2, 0x00010000);
	mmio_write_32(DBSC_DBDFICUPDCNF, 0x40100001);
	mmio_write_32(DBSC_DBRFEN, 0x00000001);
	mmio_write_32(DBSC_DBACEN, 0x00000001);
	mmio_write_32(DBSC_DBPDLK_0, 0x00000000);
	mmio_write_32(0xE67F0024, 0x00000001);
	mmio_write_32(DBSC_DBSYSCNT0, 0x00000000);

	return INITDRAM_OK;
}

int32_t rcar_dram_init(void)
{
	return init_ddr_v3m_1600();
}
