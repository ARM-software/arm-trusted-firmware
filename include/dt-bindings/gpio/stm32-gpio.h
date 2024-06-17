/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (C) 2024 STMicroelectronics - All Rights Reserved
 * Author: Paillet Pascal <p.paillet@foss.st.com> for STMicroelectronics.
 */

#ifndef DT_BINDINGS_STM32_GPIO_H
#define DT_BINDINGS_STM32_GPIO_H

/* Bank IDs used in GPIO driver API */
#define GPIO_BANK_A			0U
#define GPIO_BANK_B			1U
#define GPIO_BANK_C			2U
#define GPIO_BANK_D			3U
#define GPIO_BANK_E			4U
#define GPIO_BANK_F			5U
#define GPIO_BANK_G			6U
#define GPIO_BANK_H			7U
#define GPIO_BANK_I			8U
#define GPIO_BANK_J			9U
#define GPIO_BANK_K			10U
#define GPIO_BANK_Z			25U

/* Bit 0 is used to set GPIO in input mode */
#define GPIOF_DIR_OUT			0x0
#define GPIOF_DIR_IN			0x1

/* Bit 1 is used to set GPIO high level during init */
#define GPIOF_INIT_LOW			0x0
#define GPIOF_INIT_HIGH			0x2

#define GPIOF_IN			(GPIOF_DIR_IN)
#define GPIOF_OUT_INIT_LOW		(GPIOF_DIR_OUT | GPIOF_INIT_LOW)
#define GPIOF_OUT_INIT_HIGH		(GPIOF_DIR_OUT | GPIOF_INIT_HIGH)

/* Bit 2 is used to set GPIO pull up */
#define GPIOF_PULL_UP			0x4
/* Bit 3 is used to set GPIO pull down */
#define GPIOF_PULL_DOWN			0x8

#endif /* DT_BINDINGS_STM32_GPIO_H */
