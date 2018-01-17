/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ZynqMP system level PM-API functions for pin control.
 */

#ifndef _PM_API_PINCTRL_H_
#define _PM_API_PINCTRL_H_

#include "pm_common.h"

enum pm_pinctrl_config_param {
	PINCTRL_CONFIG_SLEW_RATE,
	PINCTRL_CONFIG_BIAS_STATUS,
	PINCTRL_CONFIG_PULL_CTRL,
	PINCTRL_CONFIG_SCHMITT_CMOS,
	PINCTRL_CONFIG_DRIVE_STRENGTH,
	PINCTRL_CONFIG_VOLTAGE_STATUS,
	PINCTRL_CONFIG_MAX,
};

enum pm_pinctrl_slew_rate {
	PINCTRL_SLEW_RATE_FAST,
	PINCTRL_SLEW_RATE_SLOW,
};

enum pm_pinctrl_bias_status {
	PINCTRL_BIAS_DISABLE,
	PINCTRL_BIAS_ENABLE,
};

enum pm_pinctrl_pull_ctrl {
	PINCTRL_BIAS_PULL_DOWN,
	PINCTRL_BIAS_PULL_UP,
};

enum pm_pinctrl_schmitt_cmos {
	PINCTRL_INPUT_TYPE_CMOS,
	PINCTRL_INPUT_TYPE_SCHMITT,
};

enum pm_pinctrl_drive_strength {
	PINCTRL_DRIVE_STRENGTH_2MA,
	PINCTRL_DRIVE_STRENGTH_4MA,
	PINCTRL_DRIVE_STRENGTH_8MA,
	PINCTRL_DRIVE_STRENGTH_12MA,
};

enum pm_ret_status pm_api_pinctrl_set_function(unsigned int pin,
					       enum pm_node_id nid);
enum pm_ret_status pm_api_pinctrl_get_function(unsigned int pin,
					       enum pm_node_id *nid);
enum pm_ret_status pm_api_pinctrl_set_config(unsigned int pin,
					     unsigned int param,
					     unsigned int value);
enum pm_ret_status pm_api_pinctrl_get_config(unsigned int pin,
					     unsigned int param,
					     unsigned int *value);

#endif /* _PM_API_PINCTRL_H_ */
