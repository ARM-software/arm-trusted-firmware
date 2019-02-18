/*
 * Copyright (c) 2018-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_RESET_H
#define STM32MP_RESET_H

#include <stdint.h>

void stm32mp_reset_assert(uint32_t reset_id);
void stm32mp_reset_deassert(uint32_t reset_id);

#endif /* STM32MP_RESET_H */
