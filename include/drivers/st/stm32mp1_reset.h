/*
 * Copyright (c) 2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_RESET_H
#define STM32MP1_RESET_H

#include <stdint.h>

void stm32mp1_reset_assert(uint32_t reset_id);
void stm32mp1_reset_deassert(uint32_t reset_id);

#endif /* STM32MP1_RESET_H */
