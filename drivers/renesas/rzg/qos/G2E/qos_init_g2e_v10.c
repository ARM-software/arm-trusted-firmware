/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "qos_init_g2e_v10.h"
#include "../qos_common.h"
#include "../qos_reg.h"

#define RCAR_QOS_VERSION	"rev.0.05"

#define REF_ARS_ARBSTOPCYCLE_G2E	(((SL_INIT_SSLOTCLK_G2E) - 5U) << 16U)

#if RCAR_QOS_TYPE == RCAR_QOS_TYPE_DEFAULT
#if RCAR_REF_INT == RCAR_REF_DEFAULT
#include "qos_init_g2e_v10_mstat390.h"
#else
#include "qos_init_g2e_v10_mstat780.h"
#endif /* RCAR_REF_INT == RCAR_REF_DEFAULT */
#endif /* RCAR_QOS_TYPE == RCAR_QOS_TYPE_DEFAULT */

static const struct rcar_gen3_dbsc_qos_settings g2e_qos[] = {
	/* BUFCAM settings */
	{ DBSC_DBCAM0CNF1, 0x00043218U },
	{ DBSC_DBCAM0CNF2, 0x000000F4U },
	{ DBSC_DBSCHCNT0, 0x000F0037U },
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
	{ DBSC_DBSCHQOS90, 0x00000100U },
	{ DBSC_DBSCHQOS91, 0x000000F0U },
	{ DBSC_DBSCHQOS92, 0x000000A0U },
	{ DBSC_DBSCHQOS93, 0x00000040U },
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

void qos_init_g2e_v10(void)
{
	rzg_qos_dbsc_setting(g2e_qos, ARRAY_SIZE(g2e_qos), true);

	/* DRAM Split Address mapping */
#if RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_4CH
#if RCAR_LSI == RCAR_RZ_G2E
#error "Don't set DRAM Split 4ch(G2E)"
#else
	ERROR("DRAM Split 4ch not supported.(G2E)");
	panic();
#endif
#elif (RCAR_DRAM_SPLIT == RCAR_DRAM_SPLIT_2CH)
#if RCAR_LSI == RCAR_RZ_G2E
#error "Don't set DRAM Split 2ch(G2E)"
#else
	ERROR("DRAM Split 2ch not supported.(G2E)");
	panic();
#endif
#else
	NOTICE("BL2: DRAM Split is OFF\n");
#endif

#if !(RCAR_QOS_TYPE == RCAR_QOS_NONE)
#if RCAR_QOS_TYPE  == RCAR_QOS_TYPE_DEFAULT
	NOTICE("BL2: QoS is default setting(%s)\n", RCAR_QOS_VERSION);
#endif

#if RCAR_REF_INT == RCAR_REF_DEFAULT
	NOTICE("BL2: DRAM refresh interval 3.9 usec\n");
#else
	NOTICE("BL2: DRAM refresh interval 7.8 usec\n");
#endif

	mmio_write_32(QOSCTRL_RAS, 0x00000020U);
	mmio_write_64(QOSCTRL_DANN, 0x0404020002020201UL);
	mmio_write_32(QOSCTRL_DANT, 0x00100804U);
	mmio_write_32(QOSCTRL_FSS, 0x0000000AU);
	mmio_write_32(QOSCTRL_INSFC, 0x06330001U);
	mmio_write_32(QOSCTRL_EARLYR, 0x00000000U);
	mmio_write_32(QOSCTRL_RACNT0, 0x00010003U);

	mmio_write_32(QOSCTRL_SL_INIT, SL_INIT_REFFSSLOT |
		      SL_INIT_SLOTSSLOT | SL_INIT_SSLOTCLK_G2E);
	mmio_write_32(QOSCTRL_REF_ARS, REF_ARS_ARBSTOPCYCLE_G2E);

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

	/* RT bus Leaf setting */
	mmio_write_32(RT_ACT0, 0x00000000U);
	mmio_write_32(RT_ACT1, 0x00000000U);

	/* CCI bus Leaf setting */
	mmio_write_32(CPU_ACT0, 0x00000003U);
	mmio_write_32(CPU_ACT1, 0x00000003U);

	mmio_write_32(QOSCTRL_RAEN, 0x00000001U);

	mmio_write_32(QOSCTRL_STATQC, 0x00000001U);
#else
	NOTICE("BL2: QoS is None\n");

	mmio_write_32(QOSCTRL_RAEN, 0x00000001U);
#endif
}
