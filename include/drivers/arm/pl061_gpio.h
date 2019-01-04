/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PL061_GPIO_H
#define PL061_GPIO_H

#include <drivers/gpio.h>

void pl061_gpio_register(uintptr_t base_addr, int gpio_dev);
void pl061_gpio_init(void);

#endif /* PL061_GPIO_H */
