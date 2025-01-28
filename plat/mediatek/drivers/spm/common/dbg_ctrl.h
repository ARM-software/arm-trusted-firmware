/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DBG_CTRL_H
#define DBG_CTRL_H

/* SPM_WAKEUP_MISC */
#define WAKE_MISC_TWAM		BIT(18)
#define WAKE_MISC_PCM_TIMER	BIT(19)
#define WAKE_MISC_CPU_WAKE	BIT(20)

struct dbg_ctrl {
	uint32_t count;
	uint32_t duration;
	void *ext;
};

enum dbg_ctrl_enum {
	DBG_CTRL_COUNT,
	DBG_CTRL_DURATION,
	DBG_CTRL_MAX,
};

#endif /* DBG_CTRL_H */
