/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PL061_GPIO_H__
#define __PL061_GPIO_H__

#include <gpio.h>

void pl061_gpio_register(uintptr_t base_addr, int gpio_dev);
void pl061_gpio_init(void);

#endif	/* __PL061_GPIO_H__ */
