/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "../qos_common.h"
#include "qos_init_g2m_v10.h"
#include "qos_init_g2m_v10_mstat.h"
#include "qos_reg.h"

#define RCAR_QOS_VERSION	"rev.0.19"

static const struct rcar_gen3_dbsc_qos_settings g2m_v10_qos[] = {
	/* BUFCAM settings */
	/* DBSC_DBCAM0CNF0 not set */
	{ DBSC_DBCAM0CNF1, 0x00043218U },
	{ DBSC_DBCAM0CNF2, 0x000000F4U },
	{ DBSC_DBCAM0CNF3, 0x00000000U },
	{ DBSC_DBSCHCNT0, 0x080F0037U },
	/* DBSC_DBSCHCNT1 not set */
	{ DBSC_DBSCHSZ0, 0x00000001U },
	{ DBSC_DBSCHRW0, 0x22421111U },

	/* DDR3 */
	{ DBSC_SCFCTST2, 0x012F1123U },

	/* QoS Settings */
	{ DBSC_DBSCHQOS00, 0x00000F00U },
	{ DBSC_DBSCHQOS01, 0x00000B00U },
	{ DBSC_DBSCHQOS02, 0x00000000U },
	{ DBSC_DBSCHQOS03, 0x00000000U },
	{ DBSC_DBSCHQOS40, 0x00000300U },
	{ DBSC_DBSCHQOS41, 0x000002F0U },
	{ DBSC_DBSCHQOS42, 0x00000200U },
	{ DBSC_DBSCHQOS43, 0x00000100U },
	{ DBSC_DBSCHQOS90, 0x00000300U },
	{ DBSC_DBSCHQOS91, 0x000002F0U },
	{ DBSC_DBSCHQOS92, 0x00000200U },
	{ DBSC_DBSCHQOS93, 0x00000100U },
	{ DBSC_DBSCHQOS130, 0x00000100U },
	{ DBSC_DBSCHQOS131, 0x000000F0U },
	{ DBSC_DBSCHQOS132, 0x000000A0U },
	{ DBSC_DBSCHQOS133, 0x00000040U },
	{ DBSC_DBSCHQOS140, 0x000000C0U },
	{ DBSC_DBSCHQOS141, 0x000000B0U },
	{ DBSC_DBSCHQOS142, 0x00000080U },
	{ DBSC_DBSCHQOS143, 0x00000040U },
	{ DBSC_DBSCHQOS150, 0x00000040U },
	{ DBSC_DBSCHQOS151, 0x00000030U },
	{ DBSC_DBSCHQOS152, 0x00000020U },
	{ DBSC_DBSCHQOS153, 0x00000010U },
};

void qos_init_g2m_v10(void)
{
	rzg_qos_dbsc_setting(g2m_v10_qos, ARRAY_SIZE(g2m_v10_qos), false);

	/* DRAM split address mapping */
#if RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_4CH
#if RCAR_LSI == RZ_G2M
#error "Don't set DRAM Split 4ch(G2M)"
#else /* RCAR_LSI == RZ_G2M */
	ERROR("DRAM Split 4ch not supported.(G2M)");
	panic();
#endif /* RCAR_LSI == RZ_G2M */
#elif (RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_2CH) || \
	(RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_AUTO)
	NOTICE("BL2: DRAM Split is 2ch\n");
	mmio_write_32(AXI_ADSPLCR0, 0x00000000U);
	mmio_write_32(AXI_ADSPLCR1, ADSPLCR0_ADRMODE_DEFAULT |
		      ADSPLCR0_SPLITSEL(0xFFU) | ADSPLCR0_AREA(0x1CU) |
		      ADSPLCR0_SWP);
	mmio_write_32(AXI_ADSPLCR2, 0x089A0000U);
	mmio_write_32(AXI_ADSPLCR3, 0x00000000U);
#else /* RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_4CH */
	NOTICE("BL2: DRAM Split is OFF\n");
#endif /* RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_4CH */

#if !(RCAR_QOS_TYPE == RCAR_QOS_NONE)
#if RCAR_QOS_TYPE  == RCAR_QOS_TYPE_DEFAULT
	NOTICE("BL2: QoS is default setting(%s)\n", RCAR_QOS_VERSION);
#endif

	/* Resource Alloc setting */
	mmio_write_32(QOSCTRL_RAS, 0x00000028U);
	mmio_write_32(QOSCTRL_FIXTH, 0x000F0005U);
	mmio_write_32(QOSCTRL_REGGD, 0x00000000U);
	mmio_write_64(QOSCTRL_DANN, 0x0101010102020201UL);
	mmio_write_32(QOSCTRL_DANT, 0x00100804U);
	mmio_write_32(QOSCTRL_EC, 0x00000000U);
	mmio_write_64(QOSCTRL_EMS, 0x0000000000000000UL);
	mmio_write_32(QOSCTRL_FSS, 0x000003e8U);
	mmio_write_32(QOSCTRL_INSFC, 0xC7840001U);
	mmio_write_32(QOSCTRL_BERR, 0x00000000U);
	mmio_write_32(QOSCTRL_RACNT0, 0x00000000U);

	/* QOSBW setting */
	mmio_write_32(QOSCTRL_SL_INIT, SL_INIT_REFFSSLOT | SL_INIT_SLOTSSLOT |
		      SL_INIT_SSLOTCLK);
	mmio_write_32(QOSCTRL_REF_ARS, 0x00330000U);

	/* QOSBW SRAM setting */
	uint32_t i;

	for (i = 0U; i < ARRAY_SIZE(mstat_fix); i++) {
		mmio_write_64(QOSBW_FIX_QOS_BANK0 + i * 8U, mstat_fix[i]);
		mmio_write_64(QOSBW_FIX_QOS_BANK1 + i * 8U, mstat_fix[i]);
	}
	for (i = 0U; i < ARRAY_SIZE(mstat_be); i++) {
		mmio_write_64(QOSBW_BE_QOS_BANK0 + i * 8U, mstat_be[i]);
		mmio_write_64(QOSBW_BE_QOS_BANK1 + i * 8U, mstat_be[i]);
	}

	/* 3DG bus Leaf setting */
	mmio_write_32(0xFD820808U, 0x00001234U);
	mmio_write_32(0xFD820800U, 0x00000006U);
	mmio_write_32(0xFD821800U, 0x00000006U);
	mmio_write_32(0xFD822800U, 0x00000006U);
	mmio_write_32(0xFD823800U, 0x00000006U);
	mmio_write_32(0xFD824800U, 0x00000006U);
	mmio_write_32(0xFD825800U, 0x00000006U);
	mmio_write_32(0xFD826800U, 0x00000006U);
	mmio_write_32(0xFD827800U, 0x00000006U);

	/* RT bus Leaf setting */
	mmio_write_32(0xFFC50800U, 0x00000000U);
	mmio_write_32(0xFFC51800U, 0x00000000U);

	/* Resource Alloc start */
	mmio_write_32(QOSCTRL_RAEN, 0x00000001U);

	/* QOSBW start */
	mmio_write_32(QOSCTRL_STATQC, 0x00000001U);
#else /* !(RCAR_QOS_TYPE == RCAR_QOS_NONE) */
	NOTICE("BL2: QoS is None\n");

	/* Resource Alloc setting */
	mmio_write_32(QOSCTRL_EC, 0x00000000U);
	/* Resource Alloc start */
	mmio_write_32(QOSCTRL_RAEN, 0x00000001U);
#endif /* !(RCAR_QOS_TYPE == RCAR_QOS_NONE) */
}
