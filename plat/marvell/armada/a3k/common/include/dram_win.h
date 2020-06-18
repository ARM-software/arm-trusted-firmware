/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef DRAM_WIN_H
#define DRAM_WIN_H

#include <common/bl_common.h>

#include <io_addr_dec.h>

void dram_win_map_build(struct dram_win_map *win_map);
void cpu_wins_init(void);

#endif /* DRAM_WIN_H */
