/*
 * Copyright (c) 2017-2020, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_SHARED_RESOURCES_H
#define STM32MP_SHARED_RESOURCES_H

#include <stdbool.h>
#include <stdint.h>

#ifdef STM32MP_SHARED_RESOURCES
enum stm32mp_shres;

/* Return true if @clock_id is shared by secure and non-secure worlds */
bool stm32mp_nsec_can_access_clock(unsigned long clock_id);

/* Return true if and only if @reset_id relates to a non-secure peripheral */
bool stm32mp_nsec_can_access_reset(unsigned int reset_id);

/* Register a shared resource assigned to the secure world */
void stm32mp_register_secure_periph(enum stm32mp_shres id);

/* Register a shared resource assigned to the non-secure world */
void stm32mp_register_non_secure_periph(enum stm32mp_shres id);

/* Register a peripheral as secure or non-secure based on IO base address */
void stm32mp_register_secure_periph_iomem(uintptr_t base);
void stm32mp_register_non_secure_periph_iomem(uintptr_t base);

/* Register a GPIO as secure or non-secure based on its bank and pin numbers */
void stm32mp_register_secure_gpio(unsigned int bank, unsigned int pin);
void stm32mp_register_non_secure_gpio(unsigned int bank, unsigned int pin);

/* Consolidate peripheral states and lock against new peripheral registering */
void stm32mp_lock_periph_registering(void);
#else
static inline void stm32mp_register_secure_periph_iomem(uintptr_t base __unused)
{
}

static inline
void stm32mp_register_non_secure_periph_iomem(uintptr_t base __unused)
{
}

static inline void stm32mp_register_secure_gpio(unsigned int bank __unused,
						unsigned int pin __unused)
{
}

static inline void stm32mp_register_non_secure_gpio(unsigned int bank __unused,
						    unsigned int pin __unused)
{
}
#endif /* STM32MP_SHARED_RESOURCES */
#endif /* STM32MP_SHARED_RESOURCES_H */
