/*
 * Copyright (c) 2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __STM32MP1_RESET_H__
#define __STM32MP1_RESET_H__

#include <stdint.h>

void stm32mp1_reset_assert(uint32_t reset_id);
void stm32mp1_reset_deassert(uint32_t reset_id);

#endif /* __STM32MP1_RESET_H__ */
