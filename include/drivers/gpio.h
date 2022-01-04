/*
 * Copyright (c) 2016-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPIO_H
#define GPIO_H

#include <export/drivers/gpio_exp.h>

#define GPIO_DIR_OUT		ARM_TF_GPIO_DIR_OUT
#define GPIO_DIR_IN		ARM_TF_GPIO_DIR_IN

#define GPIO_LEVEL_LOW		ARM_TF_GPIO_LEVEL_LOW
#define GPIO_LEVEL_HIGH		ARM_TF_GPIO_LEVEL_HIGH

#define GPIO_PULL_NONE		ARM_TF_GPIO_PULL_NONE
#define GPIO_PULL_UP		ARM_TF_GPIO_PULL_UP
#define GPIO_PULL_DOWN		ARM_TF_GPIO_PULL_DOWN
#define GPIO_PULL_REPEATER	ARM_TF_GPIO_PULL_REPEATER

typedef struct gpio_ops {
	int (*get_direction)(int gpio);
	void (*set_direction)(int gpio, int direction);
	int (*get_value)(int gpio);
	void (*set_value)(int gpio, int value);
	void (*set_pull)(int gpio, int pull);
	int (*get_pull)(int gpio);
} gpio_ops_t;

int gpio_get_direction(int gpio);
void gpio_set_direction(int gpio, int direction);
int gpio_get_value(int gpio);
void gpio_set_value(int gpio, int value);
void gpio_set_pull(int gpio, int pull);
int gpio_get_pull(int gpio);
void gpio_init(const gpio_ops_t *ops);

#endif /* GPIO_H */
