/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#if RCAR_LSI == RCAR_AUTO
#include "G2M/qos_init_g2m_v10.h"
#include "G2M/qos_init_g2m_v11.h"
#include "G2M/qos_init_g2m_v30.h"
#endif /* RCAR_LSI == RCAR_AUTO */
#if (RCAR_LSI == RZ_G2M)
#include "G2M/qos_init_g2m_v10.h"
#include "G2M/qos_init_g2m_v11.h"
#include "G2M/qos_init_g2m_v30.h"
#endif /* RCAR_LSI == RZ_G2M */
#include "qos_common.h"
#include "qos_init.h"
#include "qos_reg.h"
#include "rcar_def.h"

#define DRAM_CH_CNT	0x04U
uint32_t qos_init_ddr_ch;
uint8_t qos_init_ddr_phyvalid;

#define PRR_PRODUCT_ERR(reg)				\
	{						\
		ERROR("LSI Product ID(PRR=0x%x) QoS "	\
		"initialize not supported.\n", reg);	\
		panic();				\
	}

#define PRR_CUT_ERR(reg)				\
	{						\
		ERROR("LSI Cut ID(PRR=0x%x) QoS "	\
		"initialize not supported.\n", reg);	\
		panic();				\
	}

void rzg_qos_init(void)
{
	uint32_t reg;
	uint32_t i;

	qos_init_ddr_ch = 0U;
	qos_init_ddr_phyvalid = rzg_get_boardcnf_phyvalid();
	for (i = 0U; i < DRAM_CH_CNT; i++) {
		if ((qos_init_ddr_phyvalid & (1U << i))) {
			qos_init_ddr_ch++;
		}
	}

	reg = mmio_read_32(PRR);
#if (RCAR_LSI == RCAR_AUTO) || RCAR_LSI_CUT_COMPAT
	switch (reg & PRR_PRODUCT_MASK) {
	case PRR_PRODUCT_M3:
#if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RZ_G2M)
		switch (reg & PRR_CUT_MASK) {
		case PRR_PRODUCT_10:
			qos_init_g2m_v10();
			break;
		case PRR_PRODUCT_21: /* G2M Cut 13 */
			qos_init_g2m_v11();
			break;
		case PRR_PRODUCT_30: /* G2M Cut 30 */
		default:
			qos_init_g2m_v30();
			break;
		}
#else /* (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RZ_G2M) */
		PRR_PRODUCT_ERR(reg);
#endif /* (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RZ_G2M) */
		break;
	default:
		PRR_PRODUCT_ERR(reg);
		break;
	}
#else /* RCAR_LSI == RCAR_AUTO || RCAR_LSI_CUT_COMPAT */
#if (RCAR_LSI == RZ_G2M)
#if RCAR_LSI_CUT == RCAR_CUT_10
	/* G2M Cut 10 */
	if ((PRR_PRODUCT_M3 | PRR_PRODUCT_10)
	    != (reg & (PRR_PRODUCT_MASK | PRR_CUT_MASK))) {
		PRR_PRODUCT_ERR(reg);
	}
	qos_init_g2m_v10();
#elif RCAR_LSI_CUT == RCAR_CUT_11
	/* G2M Cut 11 */
	if ((PRR_PRODUCT_M3 | PRR_PRODUCT_20)
	    != (reg & (PRR_PRODUCT_MASK | PRR_CUT_MASK))) {
		PRR_PRODUCT_ERR(reg);
	}
	qos_init_g2m_v11();
#elif RCAR_LSI_CUT == RCAR_CUT_13
	/* G2M Cut 13 */
	if ((PRR_PRODUCT_M3 | PRR_PRODUCT_21)
	    != (reg & (PRR_PRODUCT_MASK | PRR_CUT_MASK))) {
		PRR_PRODUCT_ERR(reg);
	}
	qos_init_g2m_v11();
#else
	/* G2M Cut 30 or later */
	if ((PRR_PRODUCT_M3)
	    != (reg & (PRR_PRODUCT_MASK))) {
		PRR_PRODUCT_ERR(reg);
	}
	qos_init_g2m_v30();
#endif /* RCAR_LSI_CUT == RCAR_CUT_10 */
#else /* (RCAR_LSI == RZ_G2M) */
#error "Don't have QoS initialize routine(Unknown chip)."
#endif /* (RCAR_LSI == RZ_G2M) */
#endif /* RCAR_LSI == RCAR_AUTO || RCAR_LSI_CUT_COMPAT */
}

uint32_t get_refperiod(void)
{
	uint32_t refperiod = QOSWT_WTSET0_CYCLE;

#if (RCAR_LSI == RCAR_AUTO) || RCAR_LSI_CUT_COMPAT
	uint32_t reg;

	reg = mmio_read_32(PRR);
	switch (reg & PRR_PRODUCT_MASK) {
#if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RZ_G2M)
	case PRR_PRODUCT_M3:
		switch (reg & PRR_CUT_MASK) {
		case PRR_PRODUCT_10:
			break;
		case PRR_PRODUCT_20: /* G2M Cut 11 */
		case PRR_PRODUCT_21: /* G2M Cut 13 */
		case PRR_PRODUCT_30: /* G2M Cut 30 */
		default:
			refperiod = REFPERIOD_CYCLE;
			break;
		}
		break;
#endif /* (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RZ_G2M) */
	default:
		break;
	}
#elif RCAR_LSI == RZ_G2M
#if RCAR_LSI_CUT == RCAR_CUT_10
	/* G2M Cut 10 */
#else /* RCAR_LSI_CUT == RCAR_CUT_10 */
	/* G2M Cut 11|13|30 or later */
	refperiod = REFPERIOD_CYCLE;
#endif /* RCAR_LSI_CUT == RCAR_CUT_10 */
#endif /* RCAR_LSI == RCAR_AUTO || RCAR_LSI_CUT_COMPAT */
	return refperiod;
}

void rzg_qos_dbsc_setting(const struct rcar_gen3_dbsc_qos_settings *qos,
			  unsigned int qos_size, bool dbsc_wren)
{
	unsigned int i;

	/* Register write enable */
	if (dbsc_wren) {
		mmio_write_32(DBSC_DBSYSCNT0, 0x00001234U);
	}

	for (i = 0; i < qos_size; i++) {
		mmio_write_32(qos[i].reg, qos[i].val);
	}

	/* Register write protect */
	if (dbsc_wren) {
		mmio_write_32(DBSC_DBSYSCNT0, 0x00000000U);
	}
}
