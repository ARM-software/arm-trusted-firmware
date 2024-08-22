/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <plat_pm_helpers.h>

#define ROCKCHIP_PM_REG_REGION_MEM_LEN	(ROCKCHIP_PM_REG_REGION_MEM_SIZE / sizeof(uint32_t))

/* REG region */
#define RGN_LEN(_rgn)		(((_rgn)->end - (_rgn)->start) / (_rgn)->stride + 1)

#ifndef ROCKCHIP_PM_REG_REGION_MEM_SIZE
#define ROCKCHIP_PM_REG_REGION_MEM_SIZE		0
#endif

#ifdef ROCKCHIP_REG_RGN_MEM_BASE
static uint32_t *region_mem = (uint32_t *)ROCKCHIP_REG_RGN_MEM_BASE;
#else
static uint32_t region_mem[ROCKCHIP_PM_REG_REGION_MEM_LEN];
#endif

static int region_mem_idx;

static int alloc_region_mem(uint32_t *buf, int max_len,
			    struct reg_region *rgns, uint32_t rgn_num)
{
	int i;
	int total_len = 0, len = 0;
	struct reg_region *r = rgns;

	assert(buf && rgns && rgn_num);

	for (i = 0; i < rgn_num; i++, r++) {
		if (total_len < max_len)
			r->buf = &buf[total_len];

		len = RGN_LEN(r);
		total_len += len;
	}

	if (total_len > max_len) {
		ERROR("%s The buffer remain length:%d is too small for region:0x%x, at least %d\n",
		      __func__, max_len, rgns[0].start, total_len);
		panic();
	}

	return total_len;
}

/**
 * Alloc memory to reg_region->buf from region_mem.
 * @rgns - struct reg_region array.
 * @rgn_num - struct reg_region array length.
 */
void rockchip_alloc_region_mem(struct reg_region *rgns, uint32_t rgn_num)
{
	int max_len = 0, len;

	assert(rgns && rgn_num);

	max_len = ROCKCHIP_PM_REG_REGION_MEM_LEN - region_mem_idx;

	len = alloc_region_mem(region_mem + region_mem_idx, max_len,
			       rgns, rgn_num);

	region_mem_idx += len;
}

/**
 * Save (reg_region->start ~ reg_region->end) to reg_region->buf.
 * @rgns - struct reg_region array.
 * @rgn_num - struct reg_region array length.
 */
void rockchip_reg_rgn_save(struct reg_region *rgns, uint32_t rgn_num)
{
	struct reg_region *r;
	uint32_t addr;
	int i, j;

	assert(rgns && rgn_num);

	for (i = 0; i < rgn_num; i++) {
		r = &rgns[i];
		for (j = 0, addr = r->start; addr <= r->end; addr += r->stride, j++)
			r->buf[j] = mmio_read_32(addr);
	}
}

/**
 * Restore reg_region->buf to (reg_region->start ~ reg_region->end).
 * @rgns - struct reg_region array.
 * @rgn_num - struct reg_region array length.
 */
void rockchip_reg_rgn_restore(struct reg_region *rgns, uint32_t rgn_num)
{
	struct reg_region *r;
	uint32_t addr;
	int i, j;

	assert(rgns && rgn_num);

	for (i = 0; i < rgn_num; i++) {
		r = &rgns[i];
		for (j = 0, addr = r->start; addr <= r->end; addr += r->stride, j++)
			mmio_write_32(addr, r->buf[j] | r->wmsk);

		dsb();
	}
}

/**
 * Restore reg_region->buf to (reg_region->start ~ reg_region->end) reversely.
 * @rgns - struct reg_region array.
 * @rgn_num - struct reg_region array length.
 */
void rockchip_reg_rgn_restore_reverse(struct reg_region *rgns, uint32_t rgn_num)
{
	struct reg_region *r;
	uint32_t addr;
	int i, j;

	assert(rgns && rgn_num);

	for (i = rgn_num - 1; i >= 0; i--) {
		r = &rgns[i];
		j = RGN_LEN(r) - 1;
		for (addr = r->end; addr >= r->start; addr -= r->stride, j--)
			mmio_write_32(addr, r->buf[j] | r->wmsk);

		dsb();
	}
}

static void rockchip_print_hex(uint32_t val)
{
	int i;
	unsigned char tmp;

	putchar('0');
	putchar('x');
	for (i = 0; i < 8; val <<= 4, ++i) {
		tmp = (val & 0xf0000000) >> 28;
		if (tmp < 10)
			putchar('0' + tmp);
		else
			putchar('a' + tmp - 10);
	}
}

/**
 * Dump registers (base + start_offset ~ base + end_offset)
 * @base - the base addr of the register.
 * @start_offset - the start offset to dump.
 * @end_offset - the end offset to dump.
 * @stride - the stride of the registers.
 */
void rockchip_regs_dump(uint32_t base,
			uint32_t start_offset,
			uint32_t end_offset,
			uint32_t stride)
{
	uint32_t i;

	for (i = start_offset; i <= end_offset; i += stride) {
		if ((i - start_offset) % 16 == 0) {
			putchar('\n');
			rockchip_print_hex(base + i);
			putchar(':');
			putchar(' ');
			putchar(' ');
			putchar(' ');
			putchar(' ');
		}
		rockchip_print_hex(mmio_read_32(base + i));
		putchar(' ');
		putchar(' ');
		putchar(' ');
		putchar(' ');
	}
	putchar('\n');
}

/**
 * Dump reg regions
 * @rgns - struct reg_region array.
 * @rgn_num - struct reg_region array length.
 */
void rockchip_dump_reg_rgns(struct reg_region *rgns, uint32_t rgn_num)
{
	struct reg_region *r;
	int i;

	assert(rgns && rgn_num);

	for (i = 0; i < rgn_num; i++) {
		r = &rgns[i];
		rockchip_regs_dump(0x0, r->start, r->end, r->stride);
	}
}
