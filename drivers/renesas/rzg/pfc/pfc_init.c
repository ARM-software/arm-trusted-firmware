/*
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#if RCAR_LSI == RCAR_AUTO
#include "G2E/pfc_init_g2e.h"
#include "G2H/pfc_init_g2h.h"
#include "G2M/pfc_init_g2m.h"
#include "G2N/pfc_init_g2n.h"
#endif /* RCAR_LSI == RCAR_AUTO */
#if (RCAR_LSI == RZ_G2E)
#include "G2E/pfc_init_g2e.h"
#endif /* RCAR_LSI == RZ_G2N */
#if (RCAR_LSI == RZ_G2H)
#include "G2H/pfc_init_g2h.h"
#endif /* RCAR_LSI == RZ_G2H */
#if (RCAR_LSI == RZ_G2M)
#include "G2M/pfc_init_g2m.h"
#endif /* RCAR_LSI == RZ_G2M */
#if (RCAR_LSI == RZ_G2N)
#include "G2N/pfc_init_g2n.h"
#endif /* RCAR_LSI == RZ_G2N */
#include "rcar_def.h"

#define PRR_PRODUCT_ERR(reg)				\
	do {						\
		ERROR("LSI Product ID(PRR=0x%x) PFC init not supported.\n", \
			reg);				\
		panic();				\
	} while (0)

#define PRR_CUT_ERR(reg)				\
	do {						\
		ERROR("LSI Cut ID(PRR=0x%x) PFC init not supported.\n", \
			reg);				\
		panic();\
	} while (0)

void rzg_pfc_init(void)
{
	uint32_t reg;

	reg = mmio_read_32(RCAR_PRR);
#if RCAR_LSI == RCAR_AUTO
	switch (reg & PRR_PRODUCT_MASK) {
	case PRR_PRODUCT_M3:
		pfc_init_g2m();
		break;
	case PRR_PRODUCT_H3:
		pfc_init_g2h();
		break;
	case PRR_PRODUCT_M3N:
		pfc_init_g2n();
		break;
	case PRR_PRODUCT_E3:
		pfc_init_g2e();
		break;
	default:
		PRR_PRODUCT_ERR(reg);
		break;
	}

#elif RCAR_LSI_CUT_COMPAT /* RCAR_LSI == RCAR_AUTO */
	switch (reg & PRR_PRODUCT_MASK) {
	case PRR_PRODUCT_M3:
#if RCAR_LSI != RZ_G2M
		PRR_PRODUCT_ERR(reg);
#else /* RCAR_LSI != RZ_G2M */
		pfc_init_g2m();
#endif /* RCAR_LSI != RZ_G2M */
		break;
	case PRR_PRODUCT_H3:
#if (RCAR_LSI != RZ_G2H)
		PRR_PRODUCT_ERR(reg);
#else /* RCAR_LSI != RZ_G2H */
		pfc_init_g2h();
#endif /* RCAR_LSI != RZ_G2H */
		break;
	case PRR_PRODUCT_M3N:
#if RCAR_LSI != RZ_G2N
		PRR_PRODUCT_ERR(reg);
#else
		pfc_init_g2n();
#endif /* RCAR_LSI != RZ_G2N */
		break;
	case PRR_PRODUCT_E3:
#if RCAR_LSI != RZ_G2E
		PRR_PRODUCT_ERR(reg);
#else
		pfc_init_g2e();
#endif
		break;
	default:
		PRR_PRODUCT_ERR(reg);
		break;
	}

#else /* RCAR_LSI == RCAR_AUTO */
#if (RCAR_LSI == RZ_G2M)
	if ((reg & PRR_PRODUCT_MASK) != PRR_PRODUCT_M3) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_m3();
#elif (RCAR_LSI == RZ_G2H)
	if ((reg & PRR_PRODUCT_MASK) != PRR_PRODUCT_H3) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_g2h();
#elif (RCAR_LSI == RZ_G2N)	/* G2N */
	if ((reg & PRR_PRODUCT_MASK) != PRR_PRODUCT_M3N) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_g2n();
#elif (RCAR_LSI == RZ_G2E)
	if ((reg & PRR_PRODUCT_MASK) != PRR_PRODUCT_E3) {
		PRR_PRODUCT_ERR(reg);
	}
	pfc_init_g2e();
#else /* RCAR_LSI == RZ_G2M */
#error "Don't have PFC initialize routine(unknown)."
#endif /* RCAR_LSI == RZ_G2M */
#endif /* RCAR_LSI == RCAR_AUTO */
}
