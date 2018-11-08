/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_CONTEXT_H
#define STM32MP1_CONTEXT_H

#include <stdint.h>

int stm32_save_boot_interface(uint32_t interface, uint32_t instance);

#endif /* STM32MP1_CONTEXT_H */
