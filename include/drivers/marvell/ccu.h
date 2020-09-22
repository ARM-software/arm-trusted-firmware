/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* CCU unit device driver for Marvell AP807, AP807 and AP810 SoCs */

#ifndef CCU_H
#define CCU_H

#ifndef __ASSEMBLER__
#include <drivers/marvell/addr_map.h>
#endif

/* CCU registers definitions */
#define CCU_WIN_CR_OFFSET(ap, win)		(MVEBU_CCU_BASE(ap) + 0x0 + \
						(0x10 * win))
#define CCU_TARGET_ID_OFFSET			(8)
#define CCU_TARGET_ID_MASK			(0x7F)

#define CCU_WIN_SCR_OFFSET(ap, win)		(MVEBU_CCU_BASE(ap) + 0x4 + \
						(0x10 * win))
#define CCU_WIN_ENA_WRITE_SECURE		(0x1)
#define CCU_WIN_ENA_READ_SECURE			(0x2)

#define CCU_WIN_ALR_OFFSET(ap, win)		(MVEBU_CCU_BASE(ap) + 0x8 + \
						(0x10 * win))
#define CCU_WIN_AHR_OFFSET(ap, win)		(MVEBU_CCU_BASE(ap) + 0xC + \
						(0x10 * win))

#define CCU_WIN_GCR_OFFSET(ap)			(MVEBU_CCU_BASE(ap) + 0xD0)
#define CCU_GCR_TARGET_OFFSET			(8)
#define CCU_GCR_TARGET_MASK			(0xFF)

#define CCU_SRAM_WIN_CR				CCU_WIN_CR_OFFSET(MVEBU_AP0, 1)

#ifndef __ASSEMBLER__
int init_ccu(int);
void ccu_win_check(struct addr_map_win *win);
void ccu_enable_win(int ap_index, struct addr_map_win *win, uint32_t win_id);
void ccu_temp_win_insert(int ap_index, struct addr_map_win *win, int size);
void ccu_temp_win_remove(int ap_index, struct addr_map_win *win, int size);
void ccu_dram_win_config(int ap_index, struct addr_map_win *win);
void ccu_dram_target_set(int ap_index, uint32_t target);
void ccu_save_win_all(int ap_id);
void ccu_restore_win_all(int ap_id);
int ccu_is_win_enabled(int ap_index, uint32_t win_id);
void errata_wa_init(void);
#endif

#endif /* CCU_H */
