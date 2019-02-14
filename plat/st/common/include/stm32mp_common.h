/*
 * Copyright (C) 2018-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_COMMON_H
#define STM32MP_COMMON_H

#include <stdbool.h>

/* Functions to save and get boot context address given by ROM code */
void stm32mp_save_boot_ctx_address(uintptr_t address);
uintptr_t stm32mp_get_boot_ctx_address(void);

/*
 * Platform util functions for the GPIO driver
 * @bank: Target GPIO bank ID as per DT bindings
 *
 * Platform shall implement these functions to provide to stm32_gpio
 * driver the resource reference for a target GPIO bank. That are
 * memory mapped interface base address, interface offset (see below)
 * and clock identifier.
 *
 * stm32_get_gpio_bank_offset() returns a bank offset that is used to
 * check DT configuration matches platform implementation of the banks
 * description.
 */
uintptr_t stm32_get_gpio_bank_base(unsigned int bank);
unsigned long stm32_get_gpio_bank_clock(unsigned int bank);
uint32_t stm32_get_gpio_bank_offset(unsigned int bank);

/*
 * Util for clock gating and to get clock rate for stm32 and platform drivers
 * @id: Target clock ID, ID used in clock DT bindings
 */
bool stm32mp_clk_is_enabled(unsigned long id);
int stm32mp_clk_enable(unsigned long id);
int stm32mp_clk_disable(unsigned long id);
unsigned long stm32mp_clk_get_rate(unsigned long id);

/* Initialise the IO layer and register platform IO devices */
void stm32mp_io_setup(void);

#endif /* STM32MP_COMMON_H */
