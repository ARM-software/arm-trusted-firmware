/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_GPIO_COMMON_H
#define MT_GPIO_COMMON_H

#include <stdbool.h>
#include <stdint.h>

#include <plat/common/common_def.h>

/*  Error Code No. */
#define RSUCCESS        0
#define ERACCESS        1
#define ERINVAL         2
#define ERWRAPPER       3
#define MAX_GPIO_PIN    MT_GPIO_BASE_MAX

/* GPIO MODE CONTROL VALUE*/
typedef enum {
	GPIO_MODE_UNSUPPORTED = -1,
	GPIO_MODE_GPIO  = 0,
	GPIO_MODE_00    = 0,
	GPIO_MODE_01,
	GPIO_MODE_02,
	GPIO_MODE_03,
	GPIO_MODE_04,
	GPIO_MODE_05,
	GPIO_MODE_06,
	GPIO_MODE_07,

	GPIO_MODE_MAX,
	GPIO_MODE_DEFAULT = GPIO_MODE_00,
} GPIO_MODE;

/* GPIO DIRECTION */
typedef enum {
	MT_GPIO_DIR_UNSUPPORTED = -1,
	MT_GPIO_DIR_OUT    = 0,
	MT_GPIO_DIR_IN     = 1,
	MT_GPIO_DIR_MAX,
	MT_GPIO_DIR_DEFAULT = MT_GPIO_DIR_IN,
} GPIO_DIR;

/* GPIO PULL ENABLE*/
typedef enum {
	MT_GPIO_PULL_EN_UNSUPPORTED = -1,
	MT_GPIO_PULL_DISABLE   = 0,
	MT_GPIO_PULL_ENABLE    = 1,
	MT_GPIO_PULL_ENABLE_R0 = 2,
	MT_GPIO_PULL_ENABLE_R1 = 3,
	MT_GPIO_PULL_ENABLE_R0R1 = 4,

	MT_GPIO_PULL_EN_MAX,
	MT_GPIO_PULL_EN_DEFAULT = MT_GPIO_PULL_ENABLE,
} GPIO_PULL_EN;

/* GPIO PULL-UP/PULL-DOWN*/
typedef enum {
	MT_GPIO_PULL_UNSUPPORTED = -1,
	MT_GPIO_PULL_NONE        = 0,
	MT_GPIO_PULL_UP          = 1,
	MT_GPIO_PULL_DOWN        = 2,
	MT_GPIO_PULL_MAX,
	MT_GPIO_PULL_DEFAULT = MT_GPIO_PULL_DOWN
} GPIO_PULL;

/* GPIO OUTPUT */
typedef enum {
	MT_GPIO_OUT_UNSUPPORTED = -1,
	MT_GPIO_OUT_ZERO = 0,
	MT_GPIO_OUT_ONE  = 1,

	MT_GPIO_OUT_MAX,
	MT_GPIO_OUT_DEFAULT = MT_GPIO_OUT_ZERO,
	MT_GPIO_DATA_OUT_DEFAULT = MT_GPIO_OUT_ZERO,  /*compatible with DCT*/
} GPIO_OUT;

/* GPIO INPUT */
typedef enum {
	MT_GPIO_IN_UNSUPPORTED = -1,
	MT_GPIO_IN_ZERO = 0,
	MT_GPIO_IN_ONE  = 1,

	MT_GPIO_IN_MAX,
} GPIO_IN;

#define PIN(_id, _flag, _bit, _base, _offset) {		\
		.id = _id,				\
		.flag = _flag,				\
		.bit = _bit,				\
		.base = _base,				\
		.offset = _offset,			\
	}

struct mt_pin_info {
	uint8_t id;
	uint8_t flag;
	uint8_t bit;
	uint16_t base;
	uint16_t offset;
};

void mt_gpio_init(void);
uintptr_t mt_gpio_find_reg_addr(uint32_t pin);
#endif /* MT_GPIO_COMMON_H */
