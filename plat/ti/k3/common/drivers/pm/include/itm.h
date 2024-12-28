/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ITM_H
#define ITM_H

#include <config.h>
#include <device.h>
#include <types/errno.h>

struct debugss_drv_data {
	struct drv_data drv_data;
	unsigned char	itm_channel;
};

#ifdef CONFIG_ITM
void itm_putchar(int32_t c);
int32_t itm_init(struct device *dev);
#else
static inline void itm_putchar(int32_t c)
{
}
static inline int32_t itm_init(struct device *dev)
{
	return -ENODEV;
}
#endif

#endif	/* ITM_H */
