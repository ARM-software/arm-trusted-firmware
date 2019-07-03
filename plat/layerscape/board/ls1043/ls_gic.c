/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <endian.h>

#include <platform_def.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "soc.h"

/*
 * Get GIC offset
 * For LS1043a rev1.0, GIC base address align with 4k.
 * For LS1043a rev1.1, if DCFG_GIC400_ALIGN[GIC_ADDR_BIT]
 * is set, GIC base address align with 4K, or else align
 * with 64k.
 */
void get_gic_offset(uint32_t *gicc_base, uint32_t *gicd_base)
{

	uint32_t *ccsr_svr = (uint32_t *)DCFG_CCSR_SVR;
	uint32_t *gic_align = (uint32_t *)SCFG_GIC400_ALIGN;
	uint32_t val;
	uint32_t soc_dev_id;

	val = be32toh(mmio_read_32((uintptr_t)ccsr_svr));
	soc_dev_id = val & (SVR_WO_E << 8);

	if ((soc_dev_id == (SVR_LS1043A << 8) ||
			soc_dev_id == (SVR_LS1043AE << 8)) &&
			((val & 0xff) == REV1_1)) {
		val = be32toh(mmio_read_32((uintptr_t)gic_align));
		if (val & (1U << GIC_ADDR_BIT)) {
			*gicc_base = GICC_BASE;
			*gicd_base = GICD_BASE;
		} else {
			*gicc_base = GICC_BASE_64K;
			*gicd_base = GICD_BASE_64K;
		}
	} else {
		*gicc_base = GICC_BASE;
		*gicd_base = GICD_BASE;
	}
}
