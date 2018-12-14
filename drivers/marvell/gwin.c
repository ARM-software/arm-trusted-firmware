/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* GWIN unit device driver for Marvell AP810 SoC */

#include <common/debug.h>
#include <drivers/marvell/gwin.h>
#include <lib/mmio.h>

#include <armada_common.h>
#include <mvebu.h>
#include <mvebu_def.h>

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define DEBUG_ADDR_MAP
#endif

/* common defines */
#define WIN_ENABLE_BIT			(0x1)
#define WIN_TARGET_MASK			(0xF)
#define WIN_TARGET_SHIFT		(0x8)
#define WIN_TARGET(tgt)			(((tgt) & WIN_TARGET_MASK) \
					<< WIN_TARGET_SHIFT)

/* Bits[43:26] of the physical address are the window base,
 * which is aligned to 64MB
 */
#define ADDRESS_RSHIFT			(26)
#define ADDRESS_LSHIFT			(10)
#define GWIN_ALIGNMENT_64M		(0x4000000)

/* AP registers */
#define GWIN_CR_OFFSET(ap, win)		(MVEBU_GWIN_BASE(ap) + 0x0 + \
						(0x10 * (win)))
#define GWIN_ALR_OFFSET(ap, win)	(MVEBU_GWIN_BASE(ap) + 0x8 + \
						(0x10 * (win)))
#define GWIN_AHR_OFFSET(ap, win)	(MVEBU_GWIN_BASE(ap) + 0xc + \
						(0x10 * (win)))

#define CCU_GRU_CR_OFFSET(ap)		(MVEBU_CCU_GRU_BASE(ap))
#define CCR_GRU_CR_GWIN_MBYPASS		(1 << 1)

static void gwin_check(struct addr_map_win *win)
{
	/* The base is always 64M aligned */
	if (IS_NOT_ALIGN(win->base_addr, GWIN_ALIGNMENT_64M)) {
		win->base_addr &= ~(GWIN_ALIGNMENT_64M - 1);
		NOTICE("%s: Align the base address to 0x%llx\n",
		       __func__, win->base_addr);
	}

	/* size parameter validity check */
	if (IS_NOT_ALIGN(win->win_size, GWIN_ALIGNMENT_64M)) {
		win->win_size = ALIGN_UP(win->win_size, GWIN_ALIGNMENT_64M);
		NOTICE("%s: Aligning window size to 0x%llx\n",
		       __func__, win->win_size);
	}
}

static void gwin_enable_window(int ap_index, struct addr_map_win *win,
			       uint32_t win_num)
{
	uint32_t alr, ahr;
	uint64_t end_addr;

	if ((win->target_id & WIN_TARGET_MASK) != win->target_id) {
		ERROR("target ID = %d, is invalid\n", win->target_id);
		return;
	}

	/* calculate 64bit end-address */
	end_addr = (win->base_addr + win->win_size - 1);

	alr = (uint32_t)((win->base_addr >> ADDRESS_RSHIFT) << ADDRESS_LSHIFT);
	ahr = (uint32_t)((end_addr >> ADDRESS_RSHIFT) << ADDRESS_LSHIFT);

	/* write start address and end address for GWIN */
	mmio_write_32(GWIN_ALR_OFFSET(ap_index, win_num), alr);
	mmio_write_32(GWIN_AHR_OFFSET(ap_index, win_num), ahr);

	/* write the target ID and enable the window */
	mmio_write_32(GWIN_CR_OFFSET(ap_index, win_num),
		      WIN_TARGET(win->target_id) | WIN_ENABLE_BIT);
}

static void gwin_disable_window(int ap_index, uint32_t win_num)
{
	uint32_t win_reg;

	win_reg = mmio_read_32(GWIN_CR_OFFSET(ap_index, win_num));
	win_reg &= ~WIN_ENABLE_BIT;
	mmio_write_32(GWIN_CR_OFFSET(ap_index, win_num), win_reg);
}

/* Insert/Remove temporary window for using the out-of reset default
 * CPx base address to access the CP configuration space prior to
 * the further base address update in accordance with address mapping
 * design.
 *
 * NOTE: Use the same window array for insertion and removal of
 *       temporary windows.
 */
void gwin_temp_win_insert(int ap_index, struct addr_map_win *win, int size)
{
	uint32_t win_id;

	for (int i = 0; i < size; i++) {
		win_id = MVEBU_GWIN_MAX_WINS - i - 1;
		gwin_check(win);
		gwin_enable_window(ap_index, win, win_id);
		win++;
	}
}

/*
 * NOTE: Use the same window array for insertion and removal of
 *       temporary windows.
 */
void gwin_temp_win_remove(int ap_index, struct addr_map_win *win, int size)
{
	uint32_t win_id;

	for (int i = 0; i < size; i++) {
		uint64_t base;
		uint32_t target;

		win_id = MVEBU_GWIN_MAX_WINS - i - 1;

		target = mmio_read_32(GWIN_CR_OFFSET(ap_index, win_id));
		target >>= WIN_TARGET_SHIFT;
		target &= WIN_TARGET_MASK;

		base = mmio_read_32(GWIN_ALR_OFFSET(ap_index, win_id));
		base >>= ADDRESS_LSHIFT;
		base <<= ADDRESS_RSHIFT;

		if (win->target_id != target) {
			ERROR("%s: Trying to remove bad window-%d!\n",
			      __func__, win_id);
			continue;
		}
		gwin_disable_window(ap_index, win_id);
		win++;
	}
}

#ifdef DEBUG_ADDR_MAP
static void dump_gwin(int ap_index)
{
	uint32_t win_num;

	/* Dump all GWIN windows */
	printf("\tbank  target     start              end\n");
	printf("\t----------------------------------------------------\n");
	for (win_num = 0; win_num < MVEBU_GWIN_MAX_WINS; win_num++) {
		uint32_t cr;
		uint64_t alr, ahr;

		cr  = mmio_read_32(GWIN_CR_OFFSET(ap_index, win_num));
		/* Window enabled */
		if (cr & WIN_ENABLE_BIT) {
			alr = mmio_read_32(GWIN_ALR_OFFSET(ap_index, win_num));
			alr = (alr >> ADDRESS_LSHIFT) << ADDRESS_RSHIFT;
			ahr = mmio_read_32(GWIN_AHR_OFFSET(ap_index, win_num));
			ahr = (ahr >> ADDRESS_LSHIFT) << ADDRESS_RSHIFT;
			printf("\tgwin   %d     0x%016llx 0x%016llx\n",
			       (cr >> 8) & 0xF, alr, ahr);
		}
	}
}
#endif

int init_gwin(int ap_index)
{
	struct addr_map_win *win;
	uint32_t win_id;
	uint32_t win_count;
	uint32_t win_reg;

	INFO("Initializing GWIN Address decoding\n");

	/* Get the array of the windows and its size */
	marvell_get_gwin_memory_map(ap_index, &win, &win_count);
	if (win_count <= 0) {
		INFO("no windows configurations found\n");
		return 0;
	}

	if (win_count > MVEBU_GWIN_MAX_WINS) {
		ERROR("number of windows is bigger than %d\n",
		      MVEBU_GWIN_MAX_WINS);
		return 0;
	}

	/* disable all windows */
	for (win_id = 0; win_id < MVEBU_GWIN_MAX_WINS; win_id++)
		gwin_disable_window(ap_index, win_id);

	/* enable relevant windows */
	for (win_id = 0; win_id < win_count; win_id++, win++) {
		gwin_check(win);
		gwin_enable_window(ap_index, win, win_id);
	}

	/* GWIN Miss feature has not verified, therefore any access towards
	 * remote AP should be accompanied with proper configuration to
	 * GWIN registers group and therefore the GWIN Miss feature
	 * should be set into Bypass mode, need to make sure all GWIN regions
	 * are defined correctly that will assure no GWIN miss occurrance
	 * JIRA-AURORA2-1630
	 */
	INFO("Update GWIN miss bypass\n");
	win_reg = mmio_read_32(CCU_GRU_CR_OFFSET(ap_index));
	win_reg |= CCR_GRU_CR_GWIN_MBYPASS;
	mmio_write_32(CCU_GRU_CR_OFFSET(ap_index), win_reg);

#ifdef DEBUG_ADDR_MAP
	dump_gwin(ap_index);
#endif

	INFO("Done GWIN Address decoding Initializing\n");

	return 0;
}
