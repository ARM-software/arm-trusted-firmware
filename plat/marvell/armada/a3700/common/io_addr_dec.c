/*
 * Copyright (C) 2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include <io_addr_dec.h>
#include <plat_marvell.h>

#define MVEBU_DEC_WIN_CTRL_REG(base, win, off)	(MVEBU_REGS_BASE + (base) + \
						(win) * (off))
#define MVEBU_DEC_WIN_BASE_REG(base, win, off)	(MVEBU_REGS_BASE + (base) + \
						(win) * (off) + 0x4)
#define MVEBU_DEC_WIN_REMAP_REG(base, win, off)	(MVEBU_REGS_BASE + (base) + \
						(win) * (off) + 0x8)

#define MVEBU_DEC_WIN_CTRL_SIZE_OFF		(16)
#define MVEBU_DEC_WIN_ENABLE			(0x1)
#define MVEBU_DEC_WIN_CTRL_ATTR_OFF		(8)
#define MVEBU_DEC_WIN_CTRL_TARGET_OFF		(4)
#define MVEBU_DEC_WIN_CTRL_EN_OFF		(0)
#define MVEBU_DEC_WIN_BASE_OFF			(16)

#define MVEBU_WIN_BASE_SIZE_ALIGNMENT		(0x10000)

/* There are up to 14 IO unit which need address decode in Armada-3700 */
#define IO_UNIT_NUM_MAX				(14)

#define MVEBU_MAX_ADDRSS_4GB			(0x100000000ULL)


static void set_io_addr_dec_win(int win_id, uintptr_t base_addr,
				uintptr_t win_size,
				struct dec_win_config *dec_win)
{
	uint32_t ctrl = 0;
	uint32_t base = 0;

	/* set size */
	ctrl = ((win_size / MVEBU_WIN_BASE_SIZE_ALIGNMENT) - 1) <<
	       MVEBU_DEC_WIN_CTRL_SIZE_OFF;
	/* set attr according to IO decode window */
	ctrl |= dec_win->win_attr << MVEBU_DEC_WIN_CTRL_ATTR_OFF;
	/* set target */
	ctrl |= DRAM_CPU_DEC_TARGET_NUM << MVEBU_DEC_WIN_CTRL_TARGET_OFF;
	/* set base */
	base = (base_addr / MVEBU_WIN_BASE_SIZE_ALIGNMENT) <<
	       MVEBU_DEC_WIN_BASE_OFF;

	/* set base address*/
	mmio_write_32(MVEBU_DEC_WIN_BASE_REG(dec_win->dec_reg_base,
		      win_id, dec_win->win_offset),
		      base);
	/* set remap window, some unit does not have remap window */
	if (win_id < dec_win->max_remap)
		mmio_write_32(MVEBU_DEC_WIN_REMAP_REG(dec_win->dec_reg_base,
			      win_id, dec_win->win_offset), base);
	/* set control register */
	mmio_write_32(MVEBU_DEC_WIN_CTRL_REG(dec_win->dec_reg_base,
		      win_id, dec_win->win_offset), ctrl);
	/* enable the address decode window at last to make it effective */
	ctrl |= MVEBU_DEC_WIN_ENABLE << MVEBU_DEC_WIN_CTRL_EN_OFF;
	mmio_write_32(MVEBU_DEC_WIN_CTRL_REG(dec_win->dec_reg_base,
		      win_id, dec_win->win_offset), ctrl);

	INFO("set_io_addr_dec %d result: ctrl(0x%x) base(0x%x)",
	     win_id, mmio_read_32(MVEBU_DEC_WIN_CTRL_REG(dec_win->dec_reg_base,
	     win_id, dec_win->win_offset)),
	     mmio_read_32(MVEBU_DEC_WIN_BASE_REG(dec_win->dec_reg_base,
			  win_id, dec_win->win_offset)));
	if (win_id < dec_win->max_remap)
		INFO(" remap(%x)\n",
		     mmio_read_32(MVEBU_DEC_WIN_REMAP_REG(dec_win->dec_reg_base,
		     win_id, dec_win->win_offset)));
	else
		INFO("\n");
}

/* Set io decode window */
static int set_io_addr_dec(struct dram_win_map *win_map,
			   struct dec_win_config *dec_win)
{
	struct dram_win *win;
	int id;

	/* disable all windows first */
	for (id = 0; id < dec_win->max_dram_win; id++)
		mmio_write_32(MVEBU_DEC_WIN_CTRL_REG(dec_win->dec_reg_base, id,
			      dec_win->win_offset), 0);

	/* configure IO decode windows for DRAM, inheritate DRAM size,
	 * base and target from CPU-DRAM decode window and others
	 * from hard coded IO decode window settings array.
	 */
	if (win_map->dram_win_num > dec_win->max_dram_win) {
		/*
		 * If cpu dram windows number exceeds the io decode windows
		 * max number, then fill the first io decode window
		 * with base(0) and size(4GB).
		 */
		set_io_addr_dec_win(0, 0, MVEBU_MAX_ADDRSS_4GB, dec_win);

		return 0;
	}

	for (id = 0; id < win_map->dram_win_num; id++, win++) {
		win = &win_map->dram_windows[id];
		set_io_addr_dec_win(id, win->base_addr, win->win_size, dec_win);
	}

	return 0;
}

/*
 * init_io_addr_dec
 *
 * This function initializes io address decoder windows by
 * cpu dram window mapping information
 *
 * @input: N/A
 *     - dram_wins_map: cpu dram windows mapping
 *     - io_dec_config: io address decoder windows configuration
 *     - io_unit_num: io address decoder unit number
 * @output: N/A
 *
 * @return:  0 on success and others on failure
 */
int init_io_addr_dec(struct dram_win_map *dram_wins_map,
		     struct dec_win_config *io_dec_config, uint32_t io_unit_num)
{
	int32_t index;
	struct dec_win_config *io_dec_win;
	int32_t ret;

	INFO("Initializing IO address decode windows\n");

	if (io_dec_config == NULL || io_unit_num == 0) {
		ERROR("No IO address decoder windows configurations!\n");
		return -1;
	}

	if (io_unit_num > IO_UNIT_NUM_MAX) {
		ERROR("IO address decoder windows number %d is over max %d\n",
		      io_unit_num, IO_UNIT_NUM_MAX);
		return -1;
	}

	if (dram_wins_map == NULL) {
		ERROR("No cpu dram decoder windows map!\n");
		return -1;
	}

	for (index = 0; index < dram_wins_map->dram_win_num; index++)
		INFO("DRAM mapping %d base(0x%lx) size(0x%lx)\n",
		     index, dram_wins_map->dram_windows[index].base_addr,
		     dram_wins_map->dram_windows[index].win_size);

	/* Set address decode window for each IO */
	for (index = 0; index < io_unit_num; index++) {
		io_dec_win = io_dec_config + index;
		ret = set_io_addr_dec(dram_wins_map, io_dec_win);
		if (ret) {
			ERROR("Failed to set IO address decode\n");
			return -1;
		}
		INFO("Set IO decode window successfully, base(0x%x)",
		     io_dec_win->dec_reg_base);
		INFO(" win_attr(%x) max_dram_win(%d) max_remap(%d)",
		     io_dec_win->win_attr, io_dec_win->max_dram_win,
		     io_dec_win->max_remap);
		INFO(" win_offset(%d)\n", io_dec_win->win_offset);
	}

	return 0;
}
