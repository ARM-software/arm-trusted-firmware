/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>

#include "../qos_common.h"
#include "../qos_reg.h"
#include "qos_init_v3m.h"

#define	RCAR_QOS_VERSION		"rev.0.01"

#include "qos_init_v3m_mstat.h"

static void dbsc_setting(void)
{

	/* BUFCAM settings */
	//DBSC_DBCAM0CNF0 not set
	io_write_32(DBSC_DBCAM0CNF1, 0x00044218);	//dbcam0cnf1
	io_write_32(DBSC_DBCAM0CNF2, 0x000000F4);	//dbcam0cnf2
	//io_write_32(DBSC_DBCAM0CNF3, 0x00000007);	//dbcam0cnf3
	io_write_32(DBSC_DBSCHCNT0,  0x080F003F);	//dbschcnt0
	io_write_32(DBSC_DBSCHCNT1,  0x00001010);	//dbschcnt0

	io_write_32(DBSC_DBSCHSZ0,   0x00000001);	//dbschsz0
	io_write_32(DBSC_DBSCHRW0,   0x22421111);	//dbschrw0
	io_write_32(DBSC_DBSCHRW1,   0x00180034);	//dbschrw1
	io_write_32(DBSC_SCFCTST0,0x180B1708);
	io_write_32(DBSC_SCFCTST1,0x0808070C);
	io_write_32(DBSC_SCFCTST2,0x012F1123);

	/* QoS Settings */
	io_write_32(DBSC_DBSCHQOS00,  0x0000F000);
	io_write_32(DBSC_DBSCHQOS01,  0x0000E000);
	io_write_32(DBSC_DBSCHQOS02,  0x00007000);
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
	io_write_32(DBSC_DBSCHQOS40,  0x0000F000);
	io_write_32(DBSC_DBSCHQOS41,  0x0000EFFF);
	io_write_32(DBSC_DBSCHQOS42,  0x0000B000);
	io_write_32(DBSC_DBSCHQOS43,  0x00000000);
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
	io_write_32(DBSC_DBSCHQOS90,  0x0000F000);
	io_write_32(DBSC_DBSCHQOS91,  0x0000EFFF);
	io_write_32(DBSC_DBSCHQOS92,  0x0000D000);
	io_write_32(DBSC_DBSCHQOS93,  0x00000000);
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
	io_write_32(DBSC_DBSCHQOS130, 0x0000F000);
	io_write_32(DBSC_DBSCHQOS131, 0x0000EFFF);
	io_write_32(DBSC_DBSCHQOS132, 0x0000E800);
	io_write_32(DBSC_DBSCHQOS133, 0x00007000);
	io_write_32(DBSC_DBSCHQOS140, 0x0000F000);
	io_write_32(DBSC_DBSCHQOS141, 0x0000EFFF);
	io_write_32(DBSC_DBSCHQOS142, 0x0000E800);
	io_write_32(DBSC_DBSCHQOS143, 0x0000B000);
	io_write_32(DBSC_DBSCHQOS150, 0x000007D0);
	io_write_32(DBSC_DBSCHQOS151, 0x000007CF);
	io_write_32(DBSC_DBSCHQOS152, 0x000005D0);
	io_write_32(DBSC_DBSCHQOS153, 0x000003D0);
}

void qos_init_v3m(void)
{
return;

	dbsc_setting();

#if !(RCAR_QOS_TYPE == RCAR_QOS_NONE)
#if RCAR_QOS_TYPE  == RCAR_QOS_TYPE_DEFAULT
	NOTICE("BL2: QoS is default setting(%s)\n", RCAR_QOS_VERSION);
#endif

	/* Resource Alloc setting */
	io_write_32(QOSCTRL_RAS,   0x00000020U);
	io_write_32(QOSCTRL_FIXTH, 0x000F0005U);
	io_write_32(QOSCTRL_REGGD, 0x00000004U);
	io_write_64(QOSCTRL_DANN,  0x0202020104040200U);
	io_write_32(QOSCTRL_DANT,  0x00201008U);
	io_write_32(QOSCTRL_EC,    0x00080001U);	/* need for H3 ES1 */
	io_write_64(QOSCTRL_EMS,   0x0000000000000000U);
	io_write_32(QOSCTRL_INSFC, 0x63C20001U);
	io_write_32(QOSCTRL_BERR,  0x00000000U);

	/* QOSBW setting */
	io_write_32(QOSCTRL_SL_INIT, 0x0305007DU);
	io_write_32(QOSCTRL_REF_ARS, 0x00330000U);

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

	/* AXI-IF arbitration setting */
	io_write_32(DBSC_AXARB, 0x18010000U);

	/* Resource Alloc start */
	io_write_32(QOSCTRL_RAEN,  0x00000001U);

	/* QOSBW start */
	io_write_32(QOSCTRL_STATQC, 0x00000001U);

#else
	NOTICE("BL2: QoS is None\n");
#endif /* !(RCAR_QOS_TYPE == RCAR_QOS_NONE) */
}
