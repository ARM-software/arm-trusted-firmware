/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "rcar_def.h"
#if RCAR_LSI == RCAR_AUTO
#include "H3/pfc_init_h3_v1.h"
#include "H3/pfc_init_h3_v2.h"
#include "M3/pfc_init_m3.h"
#include "M3N/pfc_init_m3n.h"
#include "V3M/pfc_init_v3m.h"
#endif
#if (RCAR_LSI == RCAR_H3) || (RCAR_LSI == RCAR_H3N)	/* H3 */
#include "H3/pfc_init_h3_v1.h"
#include "H3/pfc_init_h3_v2.h"
#endif
#if RCAR_LSI == RCAR_M3		/* M3 */
#include "M3/pfc_init_m3.h"
#endif
#if RCAR_LSI == RCAR_M3N	/* M3N */
#include "M3N/pfc_init_m3n.h"
#endif
#if RCAR_LSI == RCAR_V3M	/* V3M */
#include "V3M/pfc_init_v3m.h"
#endif
#if RCAR_LSI == RCAR_E3		/* E3 */
#include "E3/pfc_init_e3.h"
#endif
#if RCAR_LSI == RCAR_D3		/* D3 */
#include "D3/pfc_init_d3.h"
#endif

 /* Product Register */
#define PRR			(0xFFF00044U)
#define PRR_PRODUCT_MASK	(0x00007F00U)
#define PRR_CUT_MASK		(0x000000FFU)
#define PRR_PRODUCT_H3		(0x00004F00U)	/* R-Car H3 */
#define PRR_PRODUCT_M3		(0x00005200U)	/* R-Car M3 */
#define PRR_PRODUCT_V3M		(0x00005400U)	/* R-Car V3M */
#define PRR_PRODUCT_M3N		(0x00005500U)	/* R-Car M3N */
#define PRR_PRODUCT_E3		(0x00005700U)	/* R-Car E3 */
#define PRR_PRODUCT_D3		(0x00005800U)	/* R-Car D3 */
#define PRR_PRODUCT_10		(0x00U)
#define PRR_PRODUCT_11		(0x01U)
#define PRR_PRODUCT_20		(0x10U)

#define PRR_PRODUCT_ERR(reg)	do{\
				ERROR("LSI Product ID(PRR=0x%x) PFC "\
				"initialize not supported.\n",reg);\
				panic();\
				}while(0)
#define PRR_CUT_ERR(reg)	do{\
				ERROR("LSI Cut ID(PRR=0x%x) PFC "\
				"initialize not supported.\n",reg);\
				panic();\
				}while(0)

void rcar_pfc_init(void)
{
	uint32_t reg;

	reg = mmio_read_32(RCAR_PRR);
#if RCAR_LSI == RCAR_AUTO
	switch (reg & RCAR_PRODUCT_MASK) {
	case RCAR_PRODUCT_H3:
		switch (reg & PRR_CUT_MASK) {
		case PRR_PRODUCT_10:	/* H3 Ver.1.0 */
			pfc_init_h3_v1();
			break;
		case PRR_PRODUCT_11:	/* H3 Ver.1.1 */
			pfc_init_h3_v1();
			break;
		default:	/* H3 Ver.2.0 or later */
			pfc_init_h3_v2();
			break;
		}
		break;
	case RCAR_PRODUCT_M3:
		pfc_init_m3();
		break;
	case RCAR_PRODUCT_M3N:
		pfc_init_m3n();
		break;
	case RCAR_PRODUCT_V3M:
		pfc_init_v3m();
		break;
	default:
		PRR_PRODUCT_ERR(reg);
		break;
	}

#elif RCAR_LSI_CUT_COMPAT
	switch (reg & PRR_PRODUCT_MASK) {
	case PRR_PRODUCT_H3:
#if (RCAR_LSI != RCAR_H3) && (RCAR_LSI != RCAR_H3N)
		PRR_PRODUCT_ERR(reg);
#else
		switch (reg & PRR_CUT_MASK) {
		case PRR_PRODUCT_10:	/* H3 Ver.1.0 */
			pfc_init_h3_v1();
			break;
		case PRR_PRODUCT_11:	/* H3 Ver.1.1 */
			pfc_init_h3_v1();
			break;
		default:	/* H3 Ver.2.0 or later */
			pfc_init_h3_v2();
			break;
		}
#endif
		break;
	case PRR_PRODUCT_M3:
#if RCAR_LSI != RCAR_M3
		PRR_PRODUCT_ERR(reg);
#else
		pfc_init_m3();
#endif
		break;
	case PRR_PRODUCT_M3N:
#if RCAR_LSI != RCAR_M3N
		PRR_PRODUCT_ERR(reg);
#else
		pfc_init_m3n();
#endif
		break;
	case PRR_PRODUCT_V3M:
#if RCAR_LSI != RCAR_V3M
		PRR_PRODUCT_ERR(reg);
#else
		pfc_init_v3m();
#endif
		break;
	case PRR_PRODUCT_E3:
#if RCAR_LSI != RCAR_E3
		PRR_PRODUCT_ERR(reg);
#else
		pfc_init_e3();
#endif
		break;
	case PRR_PRODUCT_D3:
#if RCAR_LSI != RCAR_D3
		PRR_PRODUCT_ERR(reg);
#else
		pfc_init_d3();
#endif
		break;
	default:
		PRR_PRODUCT_ERR(reg);
		break;
	}

#else
#if (RCAR_LSI == RCAR_H3) || (RCAR_LSI == RCAR_H3N)	/* H3 */
#if RCAR_LSI_CUT == RCAR_CUT_10
	/* H3 Ver.1.0 */
	if ((PRR_PRODUCT_H3 | PRR_PRODUCT_10)
	    != (reg & (PRR_PRODUCT_MASK | PRR_CUT_MASK))) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_h3_v1();
#elif RCAR_LSI_CUT == RCAR_CUT_11
	/* H3 Ver.1.1 */
	if ((PRR_PRODUCT_H3 | PRR_PRODUCT_11)
	    != (reg & (PRR_PRODUCT_MASK | PRR_CUT_MASK))) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_h3_v1();
#else
	/* H3 Ver.2.0 or later */
	if (PRR_PRODUCT_H3 != (reg & PRR_PRODUCT_MASK)) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_h3_v2();
#endif
#elif RCAR_LSI == RCAR_M3	/* M3 */
	if ((PRR_PRODUCT_M3) != (reg & PRR_PRODUCT_MASK)) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_m3();
#elif RCAR_LSI == RCAR_M3N	/* M3N */
	if ((PRR_PRODUCT_M3N) != (reg & PRR_PRODUCT_MASK)) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_m3n();
#elif RCAR_LSI == RCAR_V3M	/* V3M */
	if ((PRR_PRODUCT_V3M) != (reg & PRR_PRODUCT_MASK)) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_v3m();
#elif RCAR_LSI == RCAR_E3	/* E3 */
	if ((PRR_PRODUCT_E3) != (reg & PRR_PRODUCT_MASK)) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_e3();
#elif RCAR_LSI == RCAR_D3	/* D3 */
	if ((PRR_PRODUCT_D3) != (reg & PRR_PRODUCT_MASK)) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_d3();
#else
#error "Don't have PFC initialize routine(unknown)."
#endif
#endif
}
