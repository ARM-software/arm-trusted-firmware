/*
 * Copyright (c) 2022-2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32_RNG_H
#define STM32_RNG_H

#include <stdint.h>

int stm32_rng_read(uint8_t *out, uint32_t size);
int stm32_rng_init(void);
void stm32_rng_select(uintptr_t rng_base);

#endif /* STM32_RNG_H */
