/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>

#include "../qos_common.h"
#include "../qos_reg.h"
#include "qos_init_d3.h"

#define	RCAR_QOS_VERSION		"rev.0.05"

#include "qos_init_d3_mstat.h"

static void dbsc_setting(void)
{
	/* Register write enable */
	io_write_32(DBSC_DBSYSCNT0, 0x00001234U);

	/* BUFCAM settings */
	io_write_32(DBSC_DBCAM0CNF1, 0x00043218);
	io_write_32(DBSC_DBCAM0CNF2, 0x000000F4);
	io_write_32(DBSC_DBSCHCNT0, 0x000F0037);
	io_write_32(DBSC_DBSCHSZ0, 0x00000001);
	io_write_32(DBSC_DBSCHRW0, 0x22421111);

	/* DDR3 */
	io_write_32(DBSC_SCFCTST2, 0x012F1123);

	/* QoS Settings */
	io_write_32(DBSC_DBSCHQOS00, 0x00000F00);
	io_write_32(DBSC_DBSCHQOS01, 0x00000B00);
	io_write_32(DBSC_DBSCHQOS02, 0x00000000);
	io_write_32(DBSC_DBSCHQOS03, 0x00000000);
	io_write_32(DBSC_DBSCHQOS40, 0x00000300);
	io_write_32(DBSC_DBSCHQOS41, 0x000002F0);
	io_write_32(DBSC_DBSCHQOS42, 0x00000200);
	io_write_32(DBSC_DBSCHQOS43, 0x00000100);
	io_write_32(DBSC_DBSCHQOS90, 0x00000300);
	io_write_32(DBSC_DBSCHQOS91, 0x000002F0);
	io_write_32(DBSC_DBSCHQOS92, 0x00000200);
	io_write_32(DBSC_DBSCHQOS93, 0x00000100);
	io_write_32(DBSC_DBSCHQOS130, 0x00000100);
	io_write_32(DBSC_DBSCHQOS131, 0x000000F0);
	io_write_32(DBSC_DBSCHQOS132, 0x000000A0);
	io_write_32(DBSC_DBSCHQOS133, 0x00000040);
	io_write_32(DBSC_DBSCHQOS140, 0x000000C0);
	io_write_32(DBSC_DBSCHQOS141, 0x000000B0);
	io_write_32(DBSC_DBSCHQOS142, 0x00000080);
	io_write_32(DBSC_DBSCHQOS143, 0x00000040);
	io_write_32(DBSC_DBSCHQOS150, 0x00000040);
	io_write_32(DBSC_DBSCHQOS151, 0x00000030);
	io_write_32(DBSC_DBSCHQOS152, 0x00000020);
	io_write_32(DBSC_DBSCHQOS153, 0x00000010);

	/* Register write protect */
	io_write_32(DBSC_DBSYSCNT0, 0x00000000U);
}

void qos_init_d3(void)
{
	dbsc_setting();

	/* DRAM Split Address mapping */
#if RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_4CH
	ERROR("DRAM Split 4ch not supported.(D3)");
	panic();
#elif RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_2CH
	ERROR("DRAM Split 2ch not supported.(D3)");
	panic();
#elif RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_AUTO
	ERROR("DRAM Split Auto not supported.(D3)");
	panic();
#elif RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_LINEAR
/*	NOTICE("BL2: DRAM Split is OFF\n"); */
	/* Split setting(DDR 1ch) */
	io_write_32(AXI_ADSPLCR0, 0x00000000U);
	io_write_32(AXI_ADSPLCR3, 0x00000000U);
#else
	ERROR("DRAM split is an invalid value.(D3)");
	panic();
#endif

#if !(RCAR_QOS_TYPE == RCAR_QOS_NONE)
#if RCAR_QOS_TYPE  == RCAR_QOS_TYPE_DEFAULT
	NOTICE("BL2: QoS is default setting(%s)\n", RCAR_QOS_VERSION);
#endif

	/* Resource Alloc setting */
	io_write_32(QOSCTRL_RAS,   0x00000020U);
	io_write_32(QOSCTRL_FIXTH, 0x000F0005U);
	io_write_32(QOSCTRL_RAEN,  0x00000001U);
	io_write_32(QOSCTRL_REGGD, 0x00000000U);
	io_write_64(QOSCTRL_DANN,  0x0404020002020201U);
	io_write_32(QOSCTRL_DANT,  0x00100804U);
	io_write_32(QOSCTRL_EC,    0x00000000U);
	io_write_64(QOSCTRL_EMS,   0x0000000000000000U);
	io_write_32(QOSCTRL_FSS,   0x0000000AU);
	io_write_32(QOSCTRL_INSFC, 0xC7840001U);
	io_write_32(QOSCTRL_BERR,  0x00000000U);
	io_write_32(QOSCTRL_EARLYR,  0x00000000U);
	io_write_32(QOSCTRL_RACNT0,  0x00010003U);
	io_write_32(QOSCTRL_STATGEN0, 0x00000000U);

	/* GPU setting */
	io_write_32(0xFD812030U, 0x00000000U);

	/* QOSBW setting */
	io_write_32(QOSCTRL_SL_INIT, 0x030500ACU);
	io_write_32(QOSCTRL_REF_ARS, 0x00780000U);

	/* QOSBW SRAM setting */
	uint32_t i;

	for (i = 0U; i < ARRAY_SIZE(mstat_fix); i++) {
		io_write_64(QOSBW_FIX_QOS_BANK0 + i * 8, mstat_fix[i]);
		io_write_64(QOSBW_FIX_QOS_BANK1 + i * 8, mstat_fix[i]);
	}
	for (i = 0U; i < ARRAY_SIZE(mstat_be); i++) {
		io_write_64(QOSBW_BE_QOS_BANK0 + i * 8, mstat_be[i]);
		io_write_64(QOSBW_BE_QOS_BANK1 + i * 8, mstat_be[i]);
	}

	/* 3DG bus Leaf setting */
	io_write_32(0xFD820808U, 0x00001234U);
	io_write_32(0xFD820800U, 0x00000000U);
	io_write_32(0xFD821800U, 0x00000000U);
	io_write_32(0xFD822800U, 0x00000000U);
	io_write_32(0xFD823800U, 0x00000000U);

	/* RT bus Leaf setting */
	io_write_32(0xF1300800U, 0x00000003U);
	io_write_32(0xF1340800U, 0x00000003U);
	io_write_32(0xFFC50800U, 0x00000000U);
	io_write_32(0xFFC51800U, 0x00000000U);

	/* Resource Alloc start */
	io_write_32(QOSCTRL_RAEN,  0x00000001U);

	/* QOSBW start */
	io_write_32(QOSCTRL_STATQC, 0x00000001U);
#else
	NOTICE("BL2: QoS is None\n");

	/* Resource Alloc setting */
	io_write_32(QOSCTRL_EC,    0x00000000U);
	/* Resource Alloc start */
	io_write_32(QOSCTRL_RAEN,  0x00000001U);
#endif /* !(RCAR_QOS_TYPE == RCAR_QOS_NONE) */
}
