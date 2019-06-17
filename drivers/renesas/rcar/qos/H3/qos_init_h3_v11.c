/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>

#include <rcar_def.h>

#include "../qos_common.h"
#include "../qos_reg.h"
#include "qos_init_h3_v11.h"

#define	RCAR_QOS_VERSION		"rev.0.37"

#include "qos_init_h3_v11_mstat.h"

struct rcar_gen3_dbsc_qos_settings h3_v11_qos[] = {
	/* BUFCAM settings */
	/* DBSC_DBCAM0CNF0 not set */
	{ DBSC_DBCAM0CNF1, 0x00044218 },
	{ DBSC_DBCAM0CNF2, 0x000000F4 },
	/* DBSC_DBCAM0CNF3 not set */
	{ DBSC_DBSCHCNT0, 0x080F0037 },
	{ DBSC_DBSCHCNT1, 0x00001010 },
	{ DBSC_DBSCHSZ0, 0x00000001 },
	{ DBSC_DBSCHRW0, 0x22421111 },

	/* DDR3 */
	{ DBSC_SCFCTST2, 0x012F1123 },

	/* QoS Settings */
	{ DBSC_DBSCHQOS00, 0x0000F000 },
	{ DBSC_DBSCHQOS01, 0x0000E000 },
	{ DBSC_DBSCHQOS02, 0x00007000 },
	{ DBSC_DBSCHQOS03, 0x00000000 },
	{ DBSC_DBSCHQOS40, 0x00000E00 },
	{ DBSC_DBSCHQOS41, 0x00000DFF },
	{ DBSC_DBSCHQOS42, 0x00000400 },
	{ DBSC_DBSCHQOS43, 0x00000200 },
	{ DBSC_DBSCHQOS90, 0x00000C00 },
	{ DBSC_DBSCHQOS91, 0x00000BFF },
	{ DBSC_DBSCHQOS92, 0x00000400 },
	{ DBSC_DBSCHQOS93, 0x00000200 },
	{ DBSC_DBSCHQOS130, 0x00000980 },
	{ DBSC_DBSCHQOS131, 0x0000097F },
	{ DBSC_DBSCHQOS132, 0x00000300 },
	{ DBSC_DBSCHQOS133, 0x00000180 },
	{ DBSC_DBSCHQOS140, 0x00000800 },
	{ DBSC_DBSCHQOS141, 0x000007FF },
	{ DBSC_DBSCHQOS142, 0x00000300 },
	{ DBSC_DBSCHQOS143, 0x00000180 },
	{ DBSC_DBSCHQOS150, 0x000007D0 },
	{ DBSC_DBSCHQOS151, 0x000007CF },
	{ DBSC_DBSCHQOS152, 0x000005D0 },
	{ DBSC_DBSCHQOS153, 0x000003D0 },
};

void qos_init_h3_v11(void)
{
	rcar_qos_dbsc_setting(h3_v11_qos, ARRAY_SIZE(h3_v11_qos), false);

	/* DRAM Split Address mapping */
#if (RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_4CH) || \
    (RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_AUTO)
	NOTICE("BL2: DRAM Split is 4ch\n");
	io_write_32(AXI_ADSPLCR0, ADSPLCR0_ADRMODE_DEFAULT
		    | ADSPLCR0_SPLITSEL(0xFFU)
		    | ADSPLCR0_AREA(0x1BU)
		    | ADSPLCR0_SWP);
	io_write_32(AXI_ADSPLCR1, 0x00000000U);
	io_write_32(AXI_ADSPLCR2, 0xA8A90000U);
	io_write_32(AXI_ADSPLCR3, 0x00000000U);
#elif RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_2CH
	NOTICE("BL2: DRAM Split is 2ch\n");
	io_write_32(AXI_ADSPLCR0, 0x00000000U);
	io_write_32(AXI_ADSPLCR1, ADSPLCR0_ADRMODE_DEFAULT
		    | ADSPLCR0_SPLITSEL(0xFFU)
		    | ADSPLCR0_AREA(0x1BU)
		    | ADSPLCR0_SWP);
	io_write_32(AXI_ADSPLCR2, 0x00000000U);
	io_write_32(AXI_ADSPLCR3, 0x00000000U);
#else
	NOTICE("BL2: DRAM Split is OFF\n");
#endif

#if !(RCAR_QOS_TYPE == RCAR_QOS_NONE)
#if RCAR_QOS_TYPE  == RCAR_QOS_TYPE_DEFAULT
	NOTICE("BL2: QoS is default setting(%s)\n", RCAR_QOS_VERSION);
#endif

	/* AR Cache setting */
	io_write_32(0xE67D1000U, 0x00000100U);
	io_write_32(0xE67D1008U, 0x00000100U);

	/* Resource Alloc setting */
#if RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_2CH
	io_write_32(QOSCTRL_RAS, 0x00000020U);
#else
	io_write_32(QOSCTRL_RAS, 0x00000040U);
#endif
	io_write_32(QOSCTRL_FIXTH, 0x000F0005U);
	io_write_32(QOSCTRL_REGGD, 0x00000000U);
#if RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_2CH
	io_write_64(QOSCTRL_DANN, 0x0101010102020201UL);
	io_write_32(QOSCTRL_DANT, 0x00181008U);
#else
	io_write_64(QOSCTRL_DANN, 0x0101000004040401UL);
	io_write_32(QOSCTRL_DANT, 0x003C2010U);
#endif
	io_write_32(QOSCTRL_EC, 0x00080001U);	/* need for H3 v1.* */
	io_write_64(QOSCTRL_EMS, 0x0000000000000000UL);
	io_write_32(QOSCTRL_INSFC, 0xC7840001U);
	io_write_32(QOSCTRL_BERR, 0x00000000U);
	io_write_32(QOSCTRL_RACNT0, 0x00000000U);

	/* QOSBW setting */
	io_write_32(QOSCTRL_SL_INIT,
		    SL_INIT_REFFSSLOT | SL_INIT_SLOTSSLOT | SL_INIT_SSLOTCLK);
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

	/* 3DG bus Leaf setting */
	io_write_32(0xFD820808U, 0x00001234U);
	io_write_32(0xFD820800U, 0x0000003FU);
	io_write_32(0xFD821800U, 0x0000003FU);
	io_write_32(0xFD822800U, 0x0000003FU);
	io_write_32(0xFD823800U, 0x0000003FU);
	io_write_32(0xFD824800U, 0x0000003FU);
	io_write_32(0xFD825800U, 0x0000003FU);
	io_write_32(0xFD826800U, 0x0000003FU);
	io_write_32(0xFD827800U, 0x0000003FU);

	/* VIO bus Leaf setting */
	io_write_32(0xFEB89800, 0x00000001U);
	io_write_32(0xFEB8A800, 0x00000001U);
	io_write_32(0xFEB8B800, 0x00000001U);
	io_write_32(0xFEB8C800, 0x00000001U);

	/* HSC bus Leaf setting */
	io_write_32(0xE6430800, 0x00000001U);
	io_write_32(0xE6431800, 0x00000001U);
	io_write_32(0xE6432800, 0x00000001U);
	io_write_32(0xE6433800, 0x00000001U);

	/* MP bus Leaf setting */
	io_write_32(0xEC620800, 0x00000001U);
	io_write_32(0xEC621800, 0x00000001U);

	/* PERIE bus Leaf setting */
	io_write_32(0xE7760800, 0x00000001U);
	io_write_32(0xE7768800, 0x00000001U);

	/* PERIW bus Leaf setting */
	io_write_32(0xE6760800, 0x00000001U);
	io_write_32(0xE6768800, 0x00000001U);

	/* RT bus Leaf setting */
	io_write_32(0xFFC50800, 0x00000001U);
	io_write_32(0xFFC51800, 0x00000001U);

	/* CCI bus Leaf setting */
	uint32_t modemr = io_read_32(RCAR_MODEMR);

	modemr &= MODEMR_BOOT_CPU_MASK;

	if ((modemr == MODEMR_BOOT_CPU_CA57) ||
	    (modemr == MODEMR_BOOT_CPU_CA53)) {
		io_write_32(0xF1300800, 0x00000001U);
		io_write_32(0xF1340800, 0x00000001U);
		io_write_32(0xF1380800, 0x00000001U);
		io_write_32(0xF13C0800, 0x00000001U);
	}

	/* Resource Alloc start */
	io_write_32(QOSCTRL_RAEN, 0x00000001U);

	/* QOSBW start */
	io_write_32(QOSCTRL_STATQC, 0x00000001U);
#else
	NOTICE("BL2: QoS is None\n");

	/* Resource Alloc setting */
	io_write_32(QOSCTRL_EC, 0x00080001U);	/* need for H3 v1.* */
#endif /* !(RCAR_QOS_TYPE == RCAR_QOS_NONE) */
}
