/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef IO_ADDR_DEC_H
#define IO_ADDR_DEC_H

#include <stdint.h>

/* There are 5 configurable cpu decoder windows. */
#define DRAM_WIN_MAP_NUM_MAX	5
/* Target number for dram in cpu decoder windows. */
#define DRAM_CPU_DEC_TARGET_NUM	0

/*
 * Not all configurable decode windows could be used for dram, some units have
 * to reserve one decode window for other unit they have to communicate with;
 * for example, DMA engineer has 3 configurable windows, but only two could be
 * for dram while the last one has to be for pcie, so for DMA, its max_dram_win
 * is 2.
 */
struct dec_win_config {
	uint32_t dec_reg_base; /* IO address decoder register base address */
	uint32_t win_attr;	/* IO address decoder windows attributes */
	/* How many configurable dram decoder windows that this unit has; */
	uint32_t max_dram_win;
	/* The decoder windows number including remapping that this unit has */
	uint32_t max_remap;
	/* The offset between continuous decode windows
	 * within the same unit, typically 0x10
	 */
	uint32_t win_offset;
};

struct dram_win {
	uintptr_t base_addr;
	uintptr_t win_size;
};

struct  dram_win_map {
	int dram_win_num;
	struct dram_win dram_windows[DRAM_WIN_MAP_NUM_MAX];
};

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
		     struct dec_win_config *io_dec_config,
		     uint32_t io_unit_num);

#endif /* IO_ADDR_DEC_H */
