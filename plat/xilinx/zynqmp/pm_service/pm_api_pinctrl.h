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

enum pm_ret_status pm_api_pinctrl_set_function(unsigned int pin,
					       enum pm_node_id nid);
enum pm_ret_status pm_api_pinctrl_get_function(unsigned int pin,
					       enum pm_node_id *nid);

#endif /* _PM_API_PINCTRL_H_ */
