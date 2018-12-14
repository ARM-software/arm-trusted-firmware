/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MC_TRUSTZONE_H
#define MC_TRUSTZONE_H

#include <drivers/marvell/addr_map.h>

#define MVEBU_TZ_MAX_WINS	16

#define TZ_VALID		(1 << 0)
#define TZ_PERM(x)		((x) << 1)
#define TZ_RZ_ENABLE		(1 << 3)

/* tz attr definitions */
#define TZ_PERM_RW		(TZ_PERM(0))
#define TZ_PERM_RO		(TZ_PERM(1))
#define TZ_PERM_WO		(TZ_PERM(2))
#define TZ_PERM_ABORT		(TZ_PERM(3))

void tz_enable_win(int ap_index, const struct addr_map_win *win, int win_id);

#endif /* MC_TRUSTZONE_H */
