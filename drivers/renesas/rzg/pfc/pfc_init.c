/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#if RCAR_LSI == RCAR_AUTO
#include "G2M/pfc_init_g2m.h"
#endif /* RCAR_LSI == RCAR_AUTO */
#if (RCAR_LSI == RZ_G2M)
#include "G2M/pfc_init_g2m.h"
#endif /* RCAR_LSI == RZ_G2M */
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
#else /* RCAR_LSI == RZ_G2M */
#error "Don't have PFC initialize routine(unknown)."
#endif /* RCAR_LSI == RZ_G2M */
#endif /* RCAR_LSI == RCAR_AUTO */
}
