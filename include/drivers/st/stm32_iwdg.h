/*
 * Copyright (c) 2018-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32_IWDG_H
#define STM32_IWDG_H

#include <stdint.h>

#define IWDG_HW_ENABLED			BIT(0)
#define IWDG_DISABLE_ON_STOP		BIT(1)
#define IWDG_DISABLE_ON_STANDBY		BIT(2)

int stm32_iwdg_init(void);
void stm32_iwdg_refresh(void);

#endif /* STM32_IWDG_H */
