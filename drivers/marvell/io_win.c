/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* IO Window unit device driver for Marvell AP807, AP807 and AP810 SoCs */

#include <common/debug.h>
#include <drivers/marvell/io_win.h>
#include <lib/mmio.h>

#include <armada_common.h>
#include <mvebu.h>
#include <mvebu_def.h>

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define DEBUG_ADDR_MAP
#endif

/* common defines */
#define WIN_ENABLE_BIT			(0x1)
/* Physical address of the base of the window = {Addr[19:0],20`h0} */
#define ADDRESS_SHIFT			(20 - 4)
#define ADDRESS_MASK			(0xFFFFFFF0)
#define IO_WIN_ALIGNMENT_1M		(0x100000)
#define IO_WIN_ALIGNMENT_64K		(0x10000)

/* AP registers */
#define IO_WIN_ALR_OFFSET(ap, win)	(MVEBU_IO_WIN_BASE(ap) + 0x0 + \
						(0x10 * win))
#define IO_WIN_AHR_OFFSET(ap, win)	(MVEBU_IO_WIN_BASE(ap) + 0x8 + \
						(0x10 * win))
#define IO_WIN_CR_OFFSET(ap, win)	(MVEBU_IO_WIN_BASE(ap) + 0xC + \
						(0x10 * win))

/* For storage of CR, ALR, AHR abd GCR */
static uint32_t io_win_regs_save[MVEBU_IO_WIN_MAX_WINS * 3 + 1];

static void io_win_check(struct addr_map_win *win)
{
	/* for IO The base is always 1M aligned */
	/* check if address is aligned to 1M */
	if (IS_NOT_ALIGN(win->base_addr, IO_WIN_ALIGNMENT_1M)) {
		win->base_addr = ALIGN_UP(win->base_addr, IO_WIN_ALIGNMENT_1M);
		NOTICE("%s: Align up the base address to 0x%llx\n",
		       __func__, win->base_addr);
	}

	/* size parameter validity check */
	if (IS_NOT_ALIGN(win->win_size, IO_WIN_ALIGNMENT_1M)) {
		win->win_size = ALIGN_UP(win->win_size, IO_WIN_ALIGNMENT_1M);
		NOTICE("%s: Aligning size to 0x%llx\n",
		       __func__, win->win_size);
	}
}

static void io_win_enable_window(int ap_index, struct addr_map_win *win,
				 uint32_t win_num)
{
	uint32_t alr, ahr;
	uint64_t end_addr;

	if (win->target_id < 0 || win->target_id >= MVEBU_IO_WIN_MAX_WINS) {
		ERROR("target ID = %d, is invalid\n", win->target_id);
		return;
	}

	if ((win_num == 0) || (win_num > MVEBU_IO_WIN_MAX_WINS)) {
		ERROR("Enabling wrong IOW window %d!\n", win_num);
		return;
	}

	/* calculate the end-address */
	end_addr = (win->base_addr + win->win_size - 1);

	alr = (uint32_t)((win->base_addr >> ADDRESS_SHIFT) & ADDRESS_MASK);
	alr |= WIN_ENABLE_BIT;
	ahr = (uint32_t)((end_addr >> ADDRESS_SHIFT) & ADDRESS_MASK);

	/* write start address and end address for IO window */
	mmio_write_32(IO_WIN_ALR_OFFSET(ap_index, win_num), alr);
	mmio_write_32(IO_WIN_AHR_OFFSET(ap_index, win_num), ahr);

	/* write window target */
	mmio_write_32(IO_WIN_CR_OFFSET(ap_index, win_num), win->target_id);
}

static void io_win_disable_window(int ap_index, uint32_t win_num)
{
	uint32_t win_reg;

	if ((win_num == 0) || (win_num > MVEBU_IO_WIN_MAX_WINS)) {
		ERROR("Disabling wrong IOW window %d!\n", win_num);
		return;
	}

	win_reg = mmio_read_32(IO_WIN_ALR_OFFSET(ap_index, win_num));
	win_reg &= ~WIN_ENABLE_BIT;
	mmio_write_32(IO_WIN_ALR_OFFSET(ap_index, win_num), win_reg);
}

/* Insert/Remove temporary window for using the out-of reset default
 * CPx base address to access the CP configuration space prior to
 * the further base address update in accordance with address mapping
 * design.
 *
 * NOTE: Use the same window array for insertion and removal of
 *       temporary windows.
 */
void iow_temp_win_insert(int ap_index, struct addr_map_win *win, int size)
{
	uint32_t win_id;

	for (int i = 0; i < size; i++) {
		win_id = MVEBU_IO_WIN_MAX_WINS - i - 1;
		io_win_check(win);
		io_win_enable_window(ap_index, win, win_id);
		win++;
	}
}

/*
 * NOTE: Use the same window array for insertion and removal of
 *       temporary windows.
 */
void iow_temp_win_remove(int ap_index, struct addr_map_win *win, int size)
{
	uint32_t win_id;

	/* Start from the last window and do not touch Win0 */
	for (int i = 0; i < size; i++) {
		uint64_t base;
		uint32_t target;

		win_id = MVEBU_IO_WIN_MAX_WINS - i - 1;

		target = mmio_read_32(IO_WIN_CR_OFFSET(ap_index, win_id));
		base = mmio_read_32(IO_WIN_ALR_OFFSET(ap_index, win_id));
		base &= ~WIN_ENABLE_BIT;
		base <<= ADDRESS_SHIFT;

		if ((win->target_id != target) || (win->base_addr != base)) {
			ERROR("%s: Trying to remove bad window-%d!\n",
			      __func__, win_id);
			continue;
		}
		io_win_disable_window(ap_index, win_id);
		win++;
	}
}

#ifdef DEBUG_ADDR_MAP
static void dump_io_win(int ap_index)
{
	uint32_t trgt_id, win_id;
	uint32_t alr, ahr;
	uint64_t start, end;

	/* Dump all IO windows */
	printf("\tbank  target     start              end\n");
	printf("\t----------------------------------------------------\n");
	for (win_id = 0; win_id < MVEBU_IO_WIN_MAX_WINS; win_id++) {
		alr = mmio_read_32(IO_WIN_ALR_OFFSET(ap_index, win_id));
		if (alr & WIN_ENABLE_BIT) {
			alr &= ~WIN_ENABLE_BIT;
			ahr = mmio_read_32(IO_WIN_AHR_OFFSET(ap_index, win_id));
			trgt_id = mmio_read_32(IO_WIN_CR_OFFSET(ap_index,
								win_id));
			start = ((uint64_t)alr << ADDRESS_SHIFT);
			end = (((uint64_t)ahr + 0x10) << ADDRESS_SHIFT);
			printf("\tio-win %d     0x%016llx 0x%016llx\n",
			       trgt_id, start, end);
		}
	}
	printf("\tio-win gcr is %x\n",
	       mmio_read_32(MVEBU_IO_WIN_BASE(ap_index) +
	       MVEBU_IO_WIN_GCR_OFFSET));
}
#endif

static void iow_save_win_range(int ap_id, int win_first, int win_last,
			       uint32_t *buffer)
{
	int win_id, idx;

	/* Save IOW */
	for (idx = 0, win_id = win_first; win_id <= win_last; win_id++) {
		buffer[idx++] = mmio_read_32(IO_WIN_CR_OFFSET(ap_id, win_id));
		buffer[idx++] = mmio_read_32(IO_WIN_ALR_OFFSET(ap_id, win_id));
		buffer[idx++] = mmio_read_32(IO_WIN_AHR_OFFSET(ap_id, win_id));
	}
	buffer[idx] = mmio_read_32(MVEBU_IO_WIN_BASE(ap_id) +
				   MVEBU_IO_WIN_GCR_OFFSET);
}

static void iow_restore_win_range(int ap_id, int win_first, int win_last,
				  uint32_t *buffer)
{
	int win_id, idx;

	/* Restore IOW */
	for (idx = 0, win_id = win_first; win_id <= win_last; win_id++) {
		mmio_write_32(IO_WIN_CR_OFFSET(ap_id, win_id), buffer[idx++]);
		mmio_write_32(IO_WIN_ALR_OFFSET(ap_id, win_id), buffer[idx++]);
		mmio_write_32(IO_WIN_AHR_OFFSET(ap_id, win_id), buffer[idx++]);
	}
	mmio_write_32(MVEBU_IO_WIN_BASE(ap_id) + MVEBU_IO_WIN_GCR_OFFSET,
		      buffer[idx++]);
}

void iow_save_win_all(int ap_id)
{
	iow_save_win_range(ap_id, 0, MVEBU_IO_WIN_MAX_WINS - 1,
			   io_win_regs_save);
}

void iow_restore_win_all(int ap_id)
{
	iow_restore_win_range(ap_id, 0, MVEBU_IO_WIN_MAX_WINS - 1,
			      io_win_regs_save);
}

int init_io_win(int ap_index)
{
	struct addr_map_win *win;
	uint32_t win_id, win_reg;
	uint32_t win_count;

	INFO("Initializing IO WIN Address decoding\n");

	/* Get the array of the windows and its size */
	marvell_get_io_win_memory_map(ap_index, &win, &win_count);
	if (win_count <= 0)
		INFO("no windows configurations found\n");

	if (win_count > MVEBU_IO_WIN_MAX_WINS) {
		INFO("number of windows is bigger than %d\n",
		     MVEBU_IO_WIN_MAX_WINS);
		return 0;
	}

	/* Get the default target id to set the GCR */
	win_reg = marvell_get_io_win_gcr_target(ap_index);
	mmio_write_32(MVEBU_IO_WIN_BASE(ap_index) + MVEBU_IO_WIN_GCR_OFFSET,
		      win_reg);

	/* disable all IO windows */
	for (win_id = 1; win_id < MVEBU_IO_WIN_MAX_WINS; win_id++)
		io_win_disable_window(ap_index, win_id);

	/* enable relevant windows, starting from win_id = 1 because
	 * index 0 dedicated for BootROM
	 */
	for (win_id = 1; win_id <= win_count; win_id++, win++) {
		io_win_check(win);
		io_win_enable_window(ap_index, win, win_id);
	}

#ifdef DEBUG_ADDR_MAP
	dump_io_win(ap_index);
#endif

	INFO("Done IO WIN Address decoding Initializing\n");

	return 0;
}
