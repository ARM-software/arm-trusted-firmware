/*
 * Copyright (C) 2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef A3700_PM_H
#define A3700_PM_H

#include <stdint.h>

/* supported wake up sources */
enum pm_wake_up_src_type {
	WAKE_UP_SRC_GPIO,
	/* FOLLOWING SRC NOT SUPPORTED YET */
	WAKE_UP_SRC_TIMER,
	WAKE_UP_SRC_UART0,
	WAKE_UP_SRC_UART1,
	WAKE_UP_SRC_MAX,
};

struct pm_gpio_data {
	/*
	 * bank 0: North bridge GPIO
	 * bank 1: South bridge GPIO
	 */
	uint32_t bank_num;
	uint32_t gpio_num;
};

union pm_wake_up_src_data {
	struct pm_gpio_data gpio_data;
	/* delay in seconds */
	uint32_t timer_delay;
};

struct pm_wake_up_src {
	enum pm_wake_up_src_type wake_up_src_type;

	union pm_wake_up_src_data wake_up_data;
};

struct pm_wake_up_src_config {
	uint32_t	wake_up_src_num;
	struct pm_wake_up_src wake_up_src[WAKE_UP_SRC_MAX];
};

struct pm_wake_up_src_config *mv_wake_up_src_config_get(void);

#endif /* A3700_PM_H */
