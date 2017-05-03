/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_DIR_OUT		0
#define GPIO_DIR_IN		1

#define GPIO_LEVEL_LOW		0
#define GPIO_LEVEL_HIGH		1

#define GPIO_PULL_NONE		0
#define GPIO_PULL_UP		1
#define GPIO_PULL_DOWN		2

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

#endif	/* __GPIO_H__ */
