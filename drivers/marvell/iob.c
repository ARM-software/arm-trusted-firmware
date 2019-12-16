/*
 * Copyright (C) 2016 - 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* IOW unit device driver for Marvell CP110 and CP115 SoCs */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/marvell/iob.h>
#include <lib/mmio.h>

#include <armada_common.h>
#include <mvebu.h>
#include <mvebu_def.h>

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define DEBUG_ADDR_MAP
#endif

#define MVEBU_IOB_OFFSET		(0x190000)
#define MVEBU_IOB_MAX_WINS		16

/* common defines */
#define WIN_ENABLE_BIT			(0x1)
/* Physical address of the base of the window = {AddrLow[19:0],20`h0} */
#define ADDRESS_SHIFT			(20 - 4)
#define ADDRESS_MASK			(0xFFFFFFF0)
#define IOB_WIN_ALIGNMENT		(0x100000)

/* IOB registers */
#define IOB_WIN_CR_OFFSET(win)		(iob_base + 0x0 + (0x20 * win))
#define IOB_TARGET_ID_OFFSET		(8)
#define IOB_TARGET_ID_MASK		(0xF)

#define IOB_WIN_SCR_OFFSET(win)		(iob_base + 0x4 + (0x20 * win))
#define IOB_WIN_ENA_CTRL_WRITE_SECURE	(0x1)
#define IOB_WIN_ENA_CTRL_READ_SECURE	(0x2)
#define IOB_WIN_ENA_WRITE_SECURE	(0x4)
#define IOB_WIN_ENA_READ_SECURE		(0x8)

#define IOB_WIN_ALR_OFFSET(win)		(iob_base + 0x8 + (0x20 * win))
#define IOB_WIN_AHR_OFFSET(win)		(iob_base + 0xC + (0x20 * win))

#define IOB_WIN_DIOB_CR_OFFSET(win)	(iob_base + 0x10 + (0x20 * win))
#define IOB_WIN_XOR0_DIOB_EN		BIT(0)
#define IOB_WIN_XOR1_DIOB_EN		BIT(1)

uintptr_t iob_base;

static void iob_win_check(struct addr_map_win *win, uint32_t win_num)
{
	/* check if address is aligned to the size */
	if (IS_NOT_ALIGN(win->base_addr, IOB_WIN_ALIGNMENT)) {
		win->base_addr = ALIGN_UP(win->base_addr, IOB_WIN_ALIGNMENT);
		ERROR("Window %d: base address unaligned to 0x%x\n",
		      win_num, IOB_WIN_ALIGNMENT);
		printf("Align up the base address to 0x%llx\n",
		       win->base_addr);
	}

	/* size parameter validity check */
	if (IS_NOT_ALIGN(win->win_size, IOB_WIN_ALIGNMENT)) {
		win->win_size = ALIGN_UP(win->win_size, IOB_WIN_ALIGNMENT);
		ERROR("Window %d: window size unaligned to 0x%x\n", win_num,
		      IOB_WIN_ALIGNMENT);
		printf("Aligning size to 0x%llx\n", win->win_size);
	}
}

static void iob_enable_win(struct addr_map_win *win, uint32_t win_id)
{
	uint32_t iob_win_reg;
	uint32_t alr, ahr;
	uint64_t end_addr;
	uint32_t reg_en;

	/* move XOR (DMA) to use WIN1 which is used for PCI-EP address space */
	reg_en = IOB_WIN_XOR0_DIOB_EN | IOB_WIN_XOR1_DIOB_EN;
	iob_win_reg = mmio_read_32(IOB_WIN_DIOB_CR_OFFSET(0));
	iob_win_reg &= ~reg_en;
	mmio_write_32(IOB_WIN_DIOB_CR_OFFSET(0), iob_win_reg);

	iob_win_reg = mmio_read_32(IOB_WIN_DIOB_CR_OFFSET(1));
	iob_win_reg |= reg_en;
	mmio_write_32(IOB_WIN_DIOB_CR_OFFSET(1), iob_win_reg);

	end_addr = (win->base_addr + win->win_size - 1);
	alr = (uint32_t)((win->base_addr >> ADDRESS_SHIFT) & ADDRESS_MASK);
	ahr = (uint32_t)((end_addr >> ADDRESS_SHIFT) & ADDRESS_MASK);

	mmio_write_32(IOB_WIN_ALR_OFFSET(win_id), alr);
	mmio_write_32(IOB_WIN_AHR_OFFSET(win_id), ahr);

	iob_win_reg = WIN_ENABLE_BIT;
	iob_win_reg |= (win->target_id & IOB_TARGET_ID_MASK)
		       << IOB_TARGET_ID_OFFSET;
	mmio_write_32(IOB_WIN_CR_OFFSET(win_id), iob_win_reg);

}

#ifdef DEBUG_ADDR_MAP
static void dump_iob(void)
{
	uint32_t win_id, win_cr, alr, ahr;
	uint8_t target_id;
	uint64_t start, end;
	char *iob_target_name[IOB_MAX_TID] = {
		"CFG  ", "MCI0 ", "PEX1 ", "PEX2 ",
		"PEX0 ", "NAND ", "RUNIT", "MCI1 " };

	/* Dump all IOB windows */
	printf("bank  id target  start              end\n");
	printf("----------------------------------------------------\n");
	for (win_id = 0; win_id < MVEBU_IOB_MAX_WINS; win_id++) {
		win_cr = mmio_read_32(IOB_WIN_CR_OFFSET(win_id));
		if (win_cr & WIN_ENABLE_BIT) {
			target_id = (win_cr >> IOB_TARGET_ID_OFFSET) &
				     IOB_TARGET_ID_MASK;
			alr = mmio_read_32(IOB_WIN_ALR_OFFSET(win_id));
			start = ((uint64_t)alr << ADDRESS_SHIFT);
			if (win_id != 0) {
				ahr = mmio_read_32(IOB_WIN_AHR_OFFSET(win_id));
				end = (((uint64_t)ahr + 0x10) << ADDRESS_SHIFT);
			} else {
				/* Window #0 size is hardcoded to 16MB, as it's
				 * reserved for CP configuration space.
				 */
				end = start + (16 << 20);
			}
			printf("iob   %02d %s   0x%016llx 0x%016llx\n",
			       win_id, iob_target_name[target_id],
			       start, end);
		}
	}
}
#endif

void iob_cfg_space_update(int ap_idx, int cp_idx, uintptr_t base,
			  uintptr_t new_base)
{
	debug_enter();

	iob_base = base + MVEBU_IOB_OFFSET;

	NOTICE("Change the base address of AP%d-CP%d to %lx\n",
	       ap_idx, cp_idx, new_base);
	mmio_write_32(IOB_WIN_ALR_OFFSET(0), new_base >> ADDRESS_SHIFT);

	iob_base = new_base + MVEBU_IOB_OFFSET;

	/* Make sure the address was configured by the CPU before
	 * any possible access to the CP.
	 */
	dsb();

	debug_exit();
}

int init_iob(uintptr_t base)
{
	struct addr_map_win *win;
	uint32_t win_id, win_reg;
	uint32_t win_count;

	INFO("Initializing IOB Address decoding\n");

	/* Get the base address of the address decoding MBUS */
	iob_base = base + MVEBU_IOB_OFFSET;

	/* Get the array of the windows and fill the map data */
	marvell_get_iob_memory_map(&win, &win_count, base);
	if (win_count <= 0) {
		INFO("no windows configurations found\n");
		return 0;
	} else if (win_count > (MVEBU_IOB_MAX_WINS - 1)) {
		ERROR("IOB mem map array > than max available windows (%d)\n",
		      MVEBU_IOB_MAX_WINS);
		win_count = MVEBU_IOB_MAX_WINS;
	}

	/* disable all IOB windows, start from win_id = 1
	 * because can't disable internal register window
	 */
	for (win_id = 1; win_id < MVEBU_IOB_MAX_WINS; win_id++) {
		win_reg = mmio_read_32(IOB_WIN_CR_OFFSET(win_id));
		win_reg &= ~WIN_ENABLE_BIT;
		mmio_write_32(IOB_WIN_CR_OFFSET(win_id), win_reg);

		win_reg = ~IOB_WIN_ENA_CTRL_WRITE_SECURE;
		win_reg &= ~IOB_WIN_ENA_CTRL_READ_SECURE;
		win_reg &= ~IOB_WIN_ENA_WRITE_SECURE;
		win_reg &= ~IOB_WIN_ENA_READ_SECURE;
		mmio_write_32(IOB_WIN_SCR_OFFSET(win_id), win_reg);
	}

	for (win_id = 1; win_id < win_count + 1; win_id++, win++) {
		iob_win_check(win, win_id);
		iob_enable_win(win, win_id);
	}

#ifdef DEBUG_ADDR_MAP
	dump_iob();
#endif

	INFO("Done IOB Address decoding Initializing\n");

	return 0;
}
