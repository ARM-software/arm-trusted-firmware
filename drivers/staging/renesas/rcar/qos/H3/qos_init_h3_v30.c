/*
 * Copyright (c) 2018-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>

#include "../qos_common.h"
#include "../qos_reg.h"
#include "qos_init_h3_v30.h"


#define	RCAR_QOS_VERSION		"rev.0.11"

#define QOSCTRL_FSS			(QOS_BASE1 + 0x0048U)

#define QOSWT_TIME_BANK0				(20000000U)	/* unit:ns */

#define	QOSWT_WTEN_ENABLE				(0x1U)

#define QOSCTRL_REF_ARS_ARBSTOPCYCLE_H3_30	(SL_INIT_SSLOTCLK_H3_30 - 0x5U)

#define OSWT_WTREF_SLOT0_EN_REQ1_SLOT	(3U)
#define OSWT_WTREF_SLOT0_EN_REQ2_SLOT	(9U)
#define QOSWT_WTREF_SLOT0_EN			((0x1U << OSWT_WTREF_SLOT0_EN_REQ1_SLOT) | (0x1U << OSWT_WTREF_SLOT0_EN_REQ2_SLOT))
#define QOSWT_WTREF_SLOT1_EN			((0x1U << OSWT_WTREF_SLOT0_EN_REQ1_SLOT) | (0x1U << OSWT_WTREF_SLOT0_EN_REQ2_SLOT))

#define QOSWT_WTSET0_REQ_SSLOT0			(5U)
#define WT_BASE_SUB_SLOT_NUM0			(12U)
#define QOSWT_WTSET0_PERIOD0_H3_30		((QOSWT_TIME_BANK0/QOSWT_WTSET0_CYCLE_H3_30)-1U)
#define QOSWT_WTSET0_SSLOT0				(QOSWT_WTSET0_REQ_SSLOT0 -1U)
#define QOSWT_WTSET0_SLOTSLOT0			(WT_BASE_SUB_SLOT_NUM0 -1U)

#define QOSWT_WTSET1_PERIOD1_H3_30		(QOSWT_WTSET0_PERIOD0_H3_30)
#define QOSWT_WTSET1_SSLOT1				(QOSWT_WTSET0_SSLOT0)
#define QOSWT_WTSET1_SLOTSLOT1			(QOSWT_WTSET0_SLOTSLOT0)

#if RCAR_QOS_TYPE  == RCAR_QOS_TYPE_DEFAULT

#if RCAR_REF_INT == RCAR_REF_DEFAULT
#include "qos_init_h3_v30_mstat195.h"
#else
#include "qos_init_h3_v30_mstat390.h"
#endif

#if RCAR_REWT_TRAINING != RCAR_REWT_TRAINING_DISABLE

#if RCAR_REF_INT == RCAR_REF_DEFAULT
#include "qos_init_h3_v30_qoswt195.h"
#else
#include "qos_init_h3_v30_qoswt390.h"
#endif

#endif /* RCAR_REWT_TRAINING != RCAR_REWT_TRAINING_DISABLE */

#endif

static void dbsc_setting(void)
{
	uint32_t md = 0;

	/* Register write enable */
	io_write_32(DBSC_DBSYSCNT0, 0x00001234U);

	/* BUFCAM settings */
	io_write_32(DBSC_DBCAM0CNF1, 0x00043218U);	/* dbcam0cnf1 */
	io_write_32(DBSC_DBCAM0CNF2, 0x000000F4U);	/* dbcam0cnf2 */
	io_write_32(DBSC_DBCAM0CNF3, 0x00000000U);	/* dbcam0cnf3 */
	io_write_32(DBSC_DBSCHCNT0, 0x000F0037U);	/* dbschcnt0 */
	io_write_32(DBSC_DBSCHSZ0, 0x00000001U);	/* dbschsz0 */
	io_write_32(DBSC_DBSCHRW0, 0x22421111U);	/* dbschrw0 */

	md = (*((volatile uint32_t *)RST_MODEMR) & 0x000A0000) >> 17;

	switch (md) {
	case 0x0:
		/* DDR3200 */
		io_write_32(DBSC_SCFCTST2, 0x012F1123U);
		break;
	case 0x1:		/* MD19=0,MD17=1 : LPDDR4-3000, 4GByte(1GByte x4) */
		/* DDR2800 */
		io_write_32(DBSC_SCFCTST2, 0x012F1123U);
		break;
	case 0x4:		/* MD19=1,MD17=0 : LPDDR4-2400, 4GByte(1GByte x4) */
		/* DDR2400 */
		io_write_32(DBSC_SCFCTST2, 0x012F1123U);
		break;
	default:		/* MD19=1,MD17=1 : LPDDR4-1600, 4GByte(1GByte x4) */
		/* DDR1600 */
		io_write_32(DBSC_SCFCTST2, 0x012F1123U);
		break;
	}

	/* QoS Settings */
	io_write_32(DBSC_DBSCHQOS00, 0x00000F00U);
	io_write_32(DBSC_DBSCHQOS01, 0x00000B00U);
	io_write_32(DBSC_DBSCHQOS02, 0x00000000U);
	io_write_32(DBSC_DBSCHQOS03, 0x00000000U);
	io_write_32(DBSC_DBSCHQOS40, 0x00000300U);
	io_write_32(DBSC_DBSCHQOS41, 0x000002F0U);
	io_write_32(DBSC_DBSCHQOS42, 0x00000200U);
	io_write_32(DBSC_DBSCHQOS43, 0x00000100U);
	io_write_32(DBSC_DBSCHQOS90, 0x00000100U);
	io_write_32(DBSC_DBSCHQOS91, 0x000000F0U);
	io_write_32(DBSC_DBSCHQOS92, 0x000000A0U);
	io_write_32(DBSC_DBSCHQOS93, 0x00000040U);
	io_write_32(DBSC_DBSCHQOS120, 0x00000040U);
	io_write_32(DBSC_DBSCHQOS121, 0x00000030U);
	io_write_32(DBSC_DBSCHQOS122, 0x00000020U);
	io_write_32(DBSC_DBSCHQOS123, 0x00000010U);
	io_write_32(DBSC_DBSCHQOS130, 0x00000100U);
	io_write_32(DBSC_DBSCHQOS131, 0x000000F0U);
	io_write_32(DBSC_DBSCHQOS132, 0x000000A0U);
	io_write_32(DBSC_DBSCHQOS133, 0x00000040U);
	io_write_32(DBSC_DBSCHQOS140, 0x000000C0U);
	io_write_32(DBSC_DBSCHQOS141, 0x000000B0U);
	io_write_32(DBSC_DBSCHQOS142, 0x00000080U);
	io_write_32(DBSC_DBSCHQOS143, 0x00000040U);
	io_write_32(DBSC_DBSCHQOS150, 0x00000040U);
	io_write_32(DBSC_DBSCHQOS151, 0x00000030U);
	io_write_32(DBSC_DBSCHQOS152, 0x00000020U);
	io_write_32(DBSC_DBSCHQOS153, 0x00000010U);

	/* Register write protect */
	io_write_32(DBSC_DBSYSCNT0, 0x00000000U);
}

void qos_init_h3_v30(void)
{
	unsigned int split_area;
	dbsc_setting();

#if RCAR_DRAM_LPDDR4_MEMCONF == 0	/* 1GB */
	split_area = 0x1BU;
#else /* default 2GB */
	split_area = 0x1CU;
#endif

	/* DRAM Split Address mapping */
#if (RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_4CH) || \
    (RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_AUTO)
	NOTICE("BL2: DRAM Split is 4ch(DDR %x)\n", (int)qos_init_ddr_phyvalid);

	io_write_32(AXI_ADSPLCR0, ADSPLCR0_ADRMODE_DEFAULT
		    | ADSPLCR0_SPLITSEL(0xFFU)
		    | ADSPLCR0_AREA(split_area)
		    | ADSPLCR0_SWP);
	io_write_32(AXI_ADSPLCR1, 0x00000000U);
	io_write_32(AXI_ADSPLCR2, 0x00001054U);
	io_write_32(AXI_ADSPLCR3, 0x00000000U);
#elif RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_2CH
	NOTICE("BL2: DRAM Split is 2ch(DDR %x)\n", (int)qos_init_ddr_phyvalid);

	io_write_32(AXI_ADSPLCR0, ADSPLCR0_AREA(split_area));
	io_write_32(AXI_ADSPLCR1, ADSPLCR0_ADRMODE_DEFAULT
		    | ADSPLCR0_SPLITSEL(0xFFU)
		    | ADSPLCR0_AREA(split_area)
		    | ADSPLCR0_SWP);
	io_write_32(AXI_ADSPLCR2, 0x00001004U);
	io_write_32(AXI_ADSPLCR3, 0x00000000U);
#else
	io_write_32(AXI_ADSPLCR0, ADSPLCR0_AREA(split_area));
	NOTICE("BL2: DRAM Split is OFF(DDR %x)\n", (int)qos_init_ddr_phyvalid);
#endif

#if !(RCAR_QOS_TYPE == RCAR_QOS_NONE)
#if RCAR_QOS_TYPE  == RCAR_QOS_TYPE_DEFAULT
	NOTICE("BL2: QoS is default setting(%s)\n", RCAR_QOS_VERSION);
#endif

#if RCAR_REF_INT == RCAR_REF_DEFAULT
	NOTICE("BL2: DRAM refresh interval 1.95 usec\n");
#else
	NOTICE("BL2: DRAM refresh interval 3.9 usec\n");
#endif

#if RCAR_REWT_TRAINING != RCAR_REWT_TRAINING_DISABLE
	NOTICE("BL2: Periodic Write DQ Training\n");
#endif /* RCAR_REWT_TRAINING != RCAR_REWT_TRAINING_DISABLE */

	io_write_32(QOSCTRL_RAS, 0x00000044U);
	io_write_64(QOSCTRL_DANN, 0x0404010002020201UL);
	io_write_32(QOSCTRL_DANT, 0x0020100AU);
	io_write_32(QOSCTRL_FSS, 0x0000000AU);
	io_write_32(QOSCTRL_INSFC, 0x06330001U);
	io_write_32(QOSCTRL_RACNT0, 0x00010003U);

	/* GPU Boost Mode */
	io_write_32(QOSCTRL_STATGEN0, 0x00000001U);

	io_write_32(QOSCTRL_SL_INIT,
		    SL_INIT_REFFSSLOT | SL_INIT_SLOTSSLOT |
		    SL_INIT_SSLOTCLK_H3_30);
	io_write_32(QOSCTRL_REF_ARS,
		    ((QOSCTRL_REF_ARS_ARBSTOPCYCLE_H3_30 << 16)));

	{
		uint32_t i;

		for (i = 0U; i < ARRAY_SIZE(mstat_fix); i++) {
			io_write_64(QOSBW_FIX_QOS_BANK0 + i * 8, mstat_fix[i]);
			io_write_64(QOSBW_FIX_QOS_BANK1 + i * 8, mstat_fix[i]);
		}
		for (i = 0U; i < ARRAY_SIZE(mstat_be); i++) {
			io_write_64(QOSBW_BE_QOS_BANK0 + i * 8, mstat_be[i]);
			io_write_64(QOSBW_BE_QOS_BANK1 + i * 8, mstat_be[i]);
		}
#if RCAR_REWT_TRAINING != RCAR_REWT_TRAINING_DISABLE
		for (i = 0U; i < ARRAY_SIZE(qoswt_fix); i++) {
			io_write_64(QOSWT_FIX_WTQOS_BANK0 + i * 8,
				    qoswt_fix[i]);
			io_write_64(QOSWT_FIX_WTQOS_BANK1 + i * 8,
				    qoswt_fix[i]);
		}
		for (i = 0U; i < ARRAY_SIZE(qoswt_be); i++) {
			io_write_64(QOSWT_BE_WTQOS_BANK0 + i * 8, qoswt_be[i]);
			io_write_64(QOSWT_BE_WTQOS_BANK1 + i * 8, qoswt_be[i]);
		}
#endif /* RCAR_REWT_TRAINING != RCAR_REWT_TRAINING_DISABLE */
	}

	/* AXI setting */
	io_write_32(AXI_MMCR, 0x00010008U);
	io_write_32(AXI_TR3CR, 0x00010000U);
	io_write_32(AXI_TR4CR, 0x00010000U);

	/* RT bus Leaf setting */
	io_write_32(RT_ACT0, 0x00000000U);
	io_write_32(RT_ACT1, 0x00000000U);

	/* CCI bus Leaf setting */
	io_write_32(CPU_ACT0, 0x00000003U);
	io_write_32(CPU_ACT1, 0x00000003U);
	io_write_32(CPU_ACT2, 0x00000003U);
	io_write_32(CPU_ACT3, 0x00000003U);

	io_write_32(QOSCTRL_RAEN, 0x00000001U);

#if RCAR_REWT_TRAINING != RCAR_REWT_TRAINING_DISABLE
	/*  re-write training setting */
	io_write_32(QOSWT_WTREF,
		    ((QOSWT_WTREF_SLOT1_EN << 16) | QOSWT_WTREF_SLOT0_EN));
	io_write_32(QOSWT_WTSET0,
		    ((QOSWT_WTSET0_PERIOD0_H3_30 << 16) |
		     (QOSWT_WTSET0_SSLOT0 << 8) | QOSWT_WTSET0_SLOTSLOT0));
	io_write_32(QOSWT_WTSET1,
		    ((QOSWT_WTSET1_PERIOD1_H3_30 << 16) |
		     (QOSWT_WTSET1_SSLOT1 << 8) | QOSWT_WTSET1_SLOTSLOT1));

	io_write_32(QOSWT_WTEN, QOSWT_WTEN_ENABLE);
#endif /* RCAR_REWT_TRAINING != RCAR_REWT_TRAINING_DISABLE */

	io_write_32(QOSCTRL_STATQC, 0x00000001U);
#else
	NOTICE("BL2: QoS is None\n");

	io_write_32(QOSCTRL_RAEN, 0x00000001U);
#endif /* !(RCAR_QOS_TYPE == RCAR_QOS_NONE) */
}
