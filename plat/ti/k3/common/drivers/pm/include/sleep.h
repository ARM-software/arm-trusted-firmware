/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SLEEP_H
#define SLEEP_H

#include <types/short_types.h>
#include <psc.h>

struct sleep_mode {
	char	*name;
	bool	wake_sources;
	int32_t	(*enter)(void);
	int32_t	(*exit)(void);
};

void sleep_mode_update(uint8_t sleepmode, bool allow);

int32_t sleep_modes_register(const struct sleep_mode *modes, uint8_t *block, size_t n);

#endif
