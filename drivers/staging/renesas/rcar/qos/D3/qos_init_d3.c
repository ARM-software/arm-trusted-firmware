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
	uint32_t md=0;

	/* BUFCAM settings */
	//DBSC_DBCAM0CNF0 not set
	io_write_32(DBSC_DBCAM0CNF1, 0x00043218);	//dbcam0cnf1
	io_write_32(DBSC_DBCAM0CNF2, 0x000000F4);	//dbcam0cnf2
	io_write_32(DBSC_DBSCHCNT0,  0x000F0037);	//dbschcnt0
	//DBSC_DBSCHCNT1 not set
	io_write_32(DBSC_DBSCHSZ0,   0x00000001);	//dbschsz0
	io_write_32(DBSC_DBSCHRW0,   0x22421111);	//dbschrw0

	md = (*((volatile uint32_t*)RST_MODEMR) & 0x00080000) >> 19;

	switch (md) {
	case 0x0:	//MD19=0 : DDR3L-1600, 4GByte(1GByte x4)
		/* DDR1600 */
		io_write_32(DBSC_SCFCTST2, 0x012F1123);
		break;
	default:	//MD19=1 : DDR3L-1856, 4GByte(1GByte x4)
		/* DDR1856 */
		io_write_32(DBSC_SCFCTST2, 0x012F1123);
		break;
	}

	/* QoS Settings */
	io_write_32(DBSC_DBSCHQOS00,  0x00000F00);
	io_write_32(DBSC_DBSCHQOS01,  0x00000B00);
	io_write_32(DBSC_DBSCHQOS02,  0x00000000);
	io_write_32(DBSC_DBSCHQOS03,  0x00000000);
	//DBSC_DBSCHQOS10 not set
	//DBSC_DBSCHQOS11 not set
	//DBSC_DBSCHQOS12 not set
	//DBSC_DBSCHQOS13 not set
	//DBSC_DBSCHQOS20 not set
	//DBSC_DBSCHQOS21 not set
	//DBSC_DBSCHQOS22 not set
	//DBSC_DBSCHQOS23 not set
	//DBSC_DBSCHQOS30 not set
	//DBSC_DBSCHQOS31 not set
	//DBSC_DBSCHQOS32 not set
	//DBSC_DBSCHQOS33 not set
	io_write_32(DBSC_DBSCHQOS40,  0x00000300);
	io_write_32(DBSC_DBSCHQOS41,  0x000002F0);
	io_write_32(DBSC_DBSCHQOS42,  0x00000200);
	io_write_32(DBSC_DBSCHQOS43,  0x00000100);
	//DBSC_DBSCHQOS50 not set
	//DBSC_DBSCHQOS51 not set
	//DBSC_DBSCHQOS52 not set
	//DBSC_DBSCHQOS53 not set
	//DBSC_DBSCHQOS60 not set
	//DBSC_DBSCHQOS61 not set
	//DBSC_DBSCHQOS62 not set
	//DBSC_DBSCHQOS63 not set
	//DBSC_DBSCHQOS70 not set
	//DBSC_DBSCHQOS71 not set
	//DBSC_DBSCHQOS72 not set
	//DBSC_DBSCHQOS73 not set
	//DBSC_DBSCHQOS80 not set
	//DBSC_DBSCHQOS81 not set
	//DBSC_DBSCHQOS82 not set
	//DBSC_DBSCHQOS83 not set
	io_write_32(DBSC_DBSCHQOS90,  0x00000300);
	io_write_32(DBSC_DBSCHQOS91,  0x000002F0);
	io_write_32(DBSC_DBSCHQOS92,  0x00000200);
	io_write_32(DBSC_DBSCHQOS93,  0x00000100);
	//DBSC_DBSCHQOS100 not set
	//DBSC_DBSCHQOS101 not set
	//DBSC_DBSCHQOS102 not set
	//DBSC_DBSCHQOS103 not set
	//DBSC_DBSCHQOS110 not set
	//DBSC_DBSCHQOS111 not set
	//DBSC_DBSCHQOS112 not set
	//DBSC_DBSCHQOS113 not set
	//DBSC_DBSCHQOS120 not set
	//DBSC_DBSCHQOS121 not set
	//DBSC_DBSCHQOS122 not set
	//DBSC_DBSCHQOS123 not set
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
}

void qos_init_d3(void)
{
	io_write_32(DBSC_DBSYSCNT0, 0x00001234);

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
	{
	uint32_t i;

	for (i = 0U; i < ARRAY_SIZE(mstat_fix); i++) {
		io_write_64(QOSBW_FIX_QOS_BANK0 + mstat_fix[i].addr,
				mstat_fix[i].value);
		io_write_64(QOSBW_FIX_QOS_BANK1 + mstat_fix[i].addr,
				mstat_fix[i].value);
	}
	for (i = 0U; i < ARRAY_SIZE(mstat_be); i++) {
		io_write_64(QOSBW_BE_QOS_BANK0 + mstat_be[i].addr,
				mstat_be[i].value);
		io_write_64(QOSBW_BE_QOS_BANK1 + mstat_be[i].addr,
				mstat_be[i].value);
	}
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
	io_write_32(DBSC_DBSYSCNT0, 0x00000000);
}
