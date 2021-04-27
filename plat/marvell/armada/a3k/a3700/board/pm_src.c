/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <a3700_pm.h>
#include <plat_marvell.h>

/* This struct provides the PM wake up src configuration for A3720 Development Board */
static struct pm_wake_up_src_config wake_up_src_cfg = {
	.wake_up_src_num = 3,
	.wake_up_src[0] = {
		.wake_up_src_type = WAKE_UP_SRC_GPIO,
		.wake_up_data = {
			.gpio_data.bank_num = 0, /* North Bridge */
			.gpio_data.gpio_num = 14
		}
	},
	.wake_up_src[1] = {
		.wake_up_src_type = WAKE_UP_SRC_GPIO,
		.wake_up_data = {
			.gpio_data.bank_num = 1, /* South Bridge */
			.gpio_data.gpio_num = 2
		}
	},
	.wake_up_src[2] = {
		.wake_up_src_type = WAKE_UP_SRC_UART1,
	}
};

struct pm_wake_up_src_config *mv_wake_up_src_config_get(void)
{
	return &wake_up_src_cfg;
}

