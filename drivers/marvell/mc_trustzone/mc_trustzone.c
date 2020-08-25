/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <inttypes.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/marvell/addr_map.h>
#include <lib/mmio.h>

#include <mvebu_def.h>

#include "mc_trustzone.h"

#define TZ_SIZE(x)		((x) >> 13)

static int fls(int x)
{
	if (!x)
		return 0;

	return 32 - __builtin_clz(x);
}

/* To not duplicate types, the addr_map_win is used, but the "target"
 * filed is referring to attributes instead of "target".
 */
void tz_enable_win(int ap_index, const struct addr_map_win *win, int win_id)
{
	int tz_size;
	uint32_t val, base = win->base_addr;

	if ((win_id < 0) || (win_id > MVEBU_TZ_MAX_WINS)) {
		ERROR("Enabling wrong MC TrustZone window %d!\n", win_id);
		return;
	}

	/* map the window size to trustzone register convention */
	tz_size = fls(TZ_SIZE(win->win_size));

	VERBOSE("%s: window size = 0x%" PRIx64 " maps to tz_size %d\n",
		__func__, win->win_size, tz_size);
	if (tz_size < 0 || tz_size > 31) {
		ERROR("Using not allowed size for MC TrustZone window %d!\n",
		      win_id);
		return;
	}

	if (base & 0xfff) {
		base = base & ~0xfff;
		WARN("Attempt to open MC TZ win. at 0x%" PRIx64 ", truncate to 0x%x\n",
		     win->base_addr, base);
	}

	val = base | (tz_size << 7) | win->target_id | TZ_VALID;

	VERBOSE("%s: base 0x%x, tz_size moved 0x%x, attr 0x%x, val 0x%x\n",
		__func__, base, (tz_size << 7), win->target_id, val);

	mmio_write_32(MVEBU_AP_MC_TRUSTZONE_REG_LOW(ap_index, win_id), val);

	VERBOSE("%s: Win%d[0x%x] configured to 0x%x\n", __func__, win_id,
		MVEBU_AP_MC_TRUSTZONE_REG_LOW(ap_index, win_id),
		mmio_read_32(MVEBU_AP_MC_TRUSTZONE_REG_LOW(ap_index, win_id)));

	mmio_write_32(MVEBU_AP_MC_TRUSTZONE_REG_HIGH(ap_index, win_id),
		     (win->base_addr >> 32));

	VERBOSE("%s: Win%d[0x%x] configured to 0x%x\n", __func__, win_id,
		MVEBU_AP_MC_TRUSTZONE_REG_HIGH(ap_index, win_id),
		mmio_read_32(MVEBU_AP_MC_TRUSTZONE_REG_HIGH(ap_index, win_id)));
}
