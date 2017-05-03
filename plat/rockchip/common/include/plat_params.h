/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_PARAMS_H__
#define __PLAT_PARAMS_H__

#include <stdint.h>

/*
 * We defined several plat parameter structs for BL2 to pass platform related
 * parameters to Rockchip BL31 platform code.  All plat parameters start with
 * a common header, which has a type field to indicate the parameter type, and
 * a next pointer points to next parameter. If the parameter is the last one in
 * the list, next pointer will points to NULL.  After the header comes the
 * variable-sized members that describe the parameter. The picture below shows
 * how the parameters are kept in memory.
 *
 * head of list  ---> +----------------+ --+
 *                    |      type      |   |
 *                    +----------------+   |--> struct bl31_plat_param
 *               +----|      next      |   |
 *               |    +----------------+ --+
 *               |    | parameter data |
 *               |    +----------------+
 *               |
 *               +--> +----------------+ --+
 *                    |      type      |   |
 *                    +----------------+   |--> struct bl31_plat_param
 *           NULL <---|      next      |   |
 *                    +----------------+ --+
 *                    | parameter data |
 *                    +----------------+
 *
 * Note: The SCTLR_EL3.A bit (Alignment fault check enable) of ARM TF is set,
 * so be sure each parameter struct starts on 64-bit aligned address. If not,
 * alignment fault will occur during accessing its data member.
 */

#define BL31_GPIO_DIR_OUT		0
#define BL31_GPIO_DIR_IN		1

#define BL31_GPIO_LEVEL_LOW		0
#define BL31_GPIO_LEVEL_HIGH		1

#define BL31_GPIO_PULL_NONE		0
#define BL31_GPIO_PULL_UP		1
#define BL31_GPIO_PULL_DOWN		2

/* param type */
enum {
	PARAM_NONE = 0,
	PARAM_RESET,
	PARAM_POWEROFF,
	PARAM_SUSPEND_GPIO,
	PARAM_SUSPEND_APIO,
};

struct apio_info {
	uint8_t apio1 : 1;
	uint8_t apio2 : 1;
	uint8_t apio3 : 1;
	uint8_t apio4 : 1;
	uint8_t apio5 : 1;
};

struct gpio_info {
	uint8_t polarity;
	uint8_t direction;
	uint8_t pull_mode;
	uint32_t index;
};

/* common header for all plat parameter type */
struct bl31_plat_param {
	uint64_t type;
	void *next;
};

struct bl31_gpio_param {
	struct bl31_plat_param h;
	struct gpio_info gpio;
};

struct bl31_apio_param {
	struct bl31_plat_param h;
	struct apio_info apio;
};

#endif /* __PLAT_PARAMS_H__ */
