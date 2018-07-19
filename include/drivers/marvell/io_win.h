/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* IO Window unit device driver for Marvell AP807, AP807 and AP810 SoCs */

#ifndef _IO_WIN_H_
#define _IO_WIN_H_

#include <addr_map.h>

int init_io_win(int ap_index);
void iow_temp_win_insert(int ap_index, struct addr_map_win *win, int size);
void iow_temp_win_remove(int ap_index, struct addr_map_win *win, int size);
void iow_save_win_all(int ap_id);
void iow_restore_win_all(int ap_id);

#endif /* _IO_WIN_H_ */
