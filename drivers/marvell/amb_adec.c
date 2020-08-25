/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* AXI to M-Bridge decoding unit driver for Marvell Armada 8K and 8K+ SoCs */

#include <inttypes.h>
#include <stdint.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <armada_common.h>
#include <mvebu.h>
#include <mvebu_def.h>

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define DEBUG_ADDR_MAP
#endif

/* common defines */
#define WIN_ENABLE_BIT			(0x1)

#define MVEBU_AMB_ADEC_OFFSET		(0x70ff00)

#define AMB_WIN_CR_OFFSET(win)		(amb_base + 0x0 + (0x8 * win))
#define AMB_ATTR_OFFSET			8
#define AMB_ATTR_MASK			0xFF
#define AMB_SIZE_OFFSET			16
#define AMB_SIZE_MASK			0xFF

#define AMB_WIN_BASE_OFFSET(win)	(amb_base + 0x4 + (0x8 * win))
#define AMB_BASE_OFFSET			16
#define AMB_BASE_ADDR_MASK		((1 << (32 - AMB_BASE_OFFSET)) - 1)

#define AMB_WIN_ALIGNMENT_64K		(0x10000)
#define AMB_WIN_ALIGNMENT_1M		(0x100000)

uintptr_t amb_base;

static void amb_check_win(struct addr_map_win *win, uint32_t win_num)
{
	uint32_t base_addr;

	/* make sure the base address is in 16-bit range */
	if (win->base_addr > AMB_BASE_ADDR_MASK) {
		WARN("Window %d: base address is too big 0x%" PRIx64 "\n",
		       win_num, win->base_addr);
		win->base_addr = AMB_BASE_ADDR_MASK;
		WARN("Set the base address to 0x%" PRIx64 "\n", win->base_addr);
	}

	base_addr  = win->base_addr << AMB_BASE_OFFSET;
	/* for AMB The base is always 1M aligned */
	/* check if address is aligned to 1M */
	if (IS_NOT_ALIGN(base_addr, AMB_WIN_ALIGNMENT_1M)) {
		win->base_addr = ALIGN_UP(base_addr, AMB_WIN_ALIGNMENT_1M);
		WARN("Window %d: base address unaligned to 0x%x\n",
		       win_num, AMB_WIN_ALIGNMENT_1M);
		WARN("Align up the base address to 0x%" PRIx64 "\n", win->base_addr);
	}

	/* size parameter validity check */
	if (!IS_POWER_OF_2(win->win_size)) {
		WARN("Window %d: window size is not power of 2 (0x%" PRIx64 ")\n",
		       win_num, win->win_size);
		win->win_size = ROUND_UP_TO_POW_OF_2(win->win_size);
		WARN("Rounding size to 0x%" PRIx64 "\n", win->win_size);
	}
}

static void amb_enable_win(struct addr_map_win *win, uint32_t win_num)
{
	uint32_t ctrl, base, size;

	/*
	 * size is 64KB granularity.
	 * The number of ones specifies the size of the
	 * window in 64 KB granularity. 0 is 64KB
	 */
	size = (win->win_size / AMB_WIN_ALIGNMENT_64K) - 1;
	ctrl = (size << AMB_SIZE_OFFSET) | (win->target_id << AMB_ATTR_OFFSET);
	base = win->base_addr << AMB_BASE_OFFSET;

	mmio_write_32(AMB_WIN_BASE_OFFSET(win_num), base);
	mmio_write_32(AMB_WIN_CR_OFFSET(win_num), ctrl);

	/* enable window after configuring window size (and attributes) */
	ctrl |= WIN_ENABLE_BIT;
	mmio_write_32(AMB_WIN_CR_OFFSET(win_num), ctrl);
}

#ifdef DEBUG_ADDR_MAP
static void dump_amb_adec(void)
{
	uint32_t ctrl, base, win_id, attr;
	uint32_t size, size_count;

	/* Dump all AMB windows */
	printf("bank  attribute     base          size\n");
	printf("--------------------------------------------\n");
	for (win_id = 0; win_id < AMB_MAX_WIN_ID; win_id++) {
		ctrl = mmio_read_32(AMB_WIN_CR_OFFSET(win_id));
		if (ctrl & WIN_ENABLE_BIT) {
			base = mmio_read_32(AMB_WIN_BASE_OFFSET(win_id));
			attr = (ctrl >> AMB_ATTR_OFFSET) & AMB_ATTR_MASK;
			size_count = (ctrl >> AMB_SIZE_OFFSET) & AMB_SIZE_MASK;
			size = (size_count + 1) * AMB_WIN_ALIGNMENT_64K;
			printf("amb   0x%04x        0x%08x    0x%08x\n",
			       attr, base, size);
		}
	}
}
#endif

int init_amb_adec(uintptr_t base)
{
	struct addr_map_win *win;
	uint32_t win_id, win_reg;
	uint32_t win_count;

	INFO("Initializing AXI to MBus Bridge Address decoding\n");

	/* Get the base address of the AMB address decoding */
	amb_base = base + MVEBU_AMB_ADEC_OFFSET;

	/* Get the array of the windows and its size */
	marvell_get_amb_memory_map(&win, &win_count, base);
	if (win_count <= 0)
		INFO("no windows configurations found\n");

	if (win_count > AMB_MAX_WIN_ID) {
		INFO("number of windows is bigger than %d\n", AMB_MAX_WIN_ID);
		return 0;
	}

	/* disable all AMB windows */
	for (win_id = 0; win_id < AMB_MAX_WIN_ID; win_id++) {
		win_reg = mmio_read_32(AMB_WIN_CR_OFFSET(win_id));
		win_reg &= ~WIN_ENABLE_BIT;
		mmio_write_32(AMB_WIN_CR_OFFSET(win_id), win_reg);
	}

	/* enable relevant windows */
	for (win_id = 0; win_id < win_count; win_id++, win++) {
		amb_check_win(win, win_id);
		amb_enable_win(win, win_id);
	}

#ifdef DEBUG_ADDR_MAP
	dump_amb_adec();
#endif

	INFO("Done AXI to MBus Bridge Address decoding Initializing\n");

	return 0;
}
