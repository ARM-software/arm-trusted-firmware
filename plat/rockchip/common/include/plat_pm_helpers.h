/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PM_HELPERS_H
#define PLAT_PM_HELPERS_H

#include <stdint.h>

/**
 * Use this macro to define a register region.
 * start: start offset from the base address.
 * end: end offset from the base address.
 * stride: stride of registers in region.
 * base: base address of registers in region.
 * wmsk: write mask of registers in region.
 */
#define REG_REGION(_start, _end, _stride, _base, _wmsk)	\
{							\
	.start = (_base) + (_start),			\
	.end   = (_base) + (_end),			\
	.stride   = _stride,				\
	.wmsk  = _wmsk					\
}

struct reg_region {
	/* Start address of region */
	uint32_t start;
	/* End address of region */
	uint32_t end;
	/* Stride of registers in region */
	uint32_t stride;
	/* Write mask of registers in region */
	uint32_t wmsk;
	/* Buffer to save/restore registers in region */
	uint32_t *buf;
};

void rockchip_alloc_region_mem(struct reg_region *rgns, uint32_t rgn_num);
void rockchip_reg_rgn_save(struct reg_region *rgns, uint32_t rgn_num);
void rockchip_reg_rgn_restore(struct reg_region *rgns, uint32_t rgn_num);
void rockchip_reg_rgn_restore_reverse(struct reg_region *rgns, uint32_t rgn_num);
void rockchip_regs_dump(uint32_t base,
			uint32_t start_offset,
			uint32_t end_offset,
			uint32_t stride);
void rockchip_dump_reg_rgns(struct reg_region *rgns, uint32_t rgn_num);

#endif /* PLAT_PM_HELPERS_H */
