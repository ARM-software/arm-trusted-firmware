/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>

#include "../qos_common.h"
#include "../qos_reg.h"
#include "qos_init_h3_v10.h"

#define	RCAR_QOS_VERSION		"rev.0.36"

#include "qos_init_h3_v10_mstat.h"

void qos_init_h3_v10(void)
{
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
	io_write_32(QOSCTRL_RAS, 0x00000040U);
	io_write_32(QOSCTRL_FIXTH, 0x000F0005U);
	io_write_32(QOSCTRL_REGGD, 0x00000004U);
	io_write_64(QOSCTRL_DANN, 0x0202000004040404UL);
	io_write_32(QOSCTRL_DANT, 0x003C1110U);
	io_write_32(QOSCTRL_EC, 0x00080001U);	/* need for H3 v1.* */
	io_write_64(QOSCTRL_EMS, 0x0000000000000000UL);
	io_write_32(QOSCTRL_INSFC, 0xC7840001U);
	io_write_32(QOSCTRL_BERR, 0x00000000U);

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
