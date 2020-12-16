/*
 * Copyright (c) 2018-2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include "rcar_private.h"

#if IMAGE_BL31
void __attribute__ ((section(".system_ram"))) cpg_write(uintptr_t regadr, uint32_t regval)
#else
void cpg_write(uintptr_t regadr, uint32_t regval)
#endif
{
	uint32_t value = regval;

	mmio_write_32((uintptr_t) RCAR_CPGWPR, ~value);
	mmio_write_32(regadr, value);
}

#if IMAGE_BL31
void __attribute__ ((section(".system_ram"))) mstpcr_write(uint32_t mstpcr, uint32_t mstpsr,
							   uint32_t target_bit)
#else
void mstpcr_write(uint32_t mstpcr, uint32_t mstpsr, uint32_t target_bit)
#endif
{
	uint32_t reg;

	reg = mmio_read_32(mstpcr);
	reg &= ~target_bit;
	cpg_write(mstpcr, reg);
	while ((mmio_read_32(mstpsr) & target_bit) != 0U) {
	}
}
