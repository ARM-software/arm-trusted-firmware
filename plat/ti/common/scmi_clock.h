/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef SCMI_CLOCK_H
#define SCMI_CLOCK_H

#include <stdint.h>

struct ti_scmi_clock {
	uint32_t dev_id;
	uint32_t clock_id;
	uint32_t *rates;
	char name[64];
};

#endif /* SCMI_CLOCK_H */
