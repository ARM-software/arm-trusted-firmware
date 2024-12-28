/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef REMOTE_H
#define REMOTE_H

#include <stddef.h>
#include <device.h>
#include <types/short_types.h>

struct remote_drv_data {
	struct drv_data		drv_data;
	const struct irq_data	*irq_data;
	uint8_t			host;
};

struct resource_irq;

bool remote_wake_check(uint32_t host);

int32_t remote_do_suspend(struct device *dev);

void remote_pwrdwn_handler(const struct resource_irq *irq, void *data);

extern const struct drv remote_drv;

#endif
