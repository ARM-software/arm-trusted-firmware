/*
 * Copyright (c) 2018-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_RESET_H
#define STM32MP_RESET_H

#include <stdint.h>

/*
 * Assert target reset, if @to_us non null, wait until reset is asserted
 *
 * @reset_id: Reset controller ID
 * @to_us: Timeout in microsecond, or 0 if not waiting
 * Return 0 on success and -ETIMEDOUT if waiting and timeout expired
 */
int stm32mp_reset_assert(uint32_t reset_id, unsigned int to_us);

/*
 * Enable reset control for target resource
 *
 * @reset_id: Reset controller ID
 */
static inline void stm32mp_reset_set(uint32_t reset_id)
{
	(void)stm32mp_reset_assert(reset_id, 0U);
}

/*
 * Deassert target reset, if @to_us non null, wait until reset is deasserted
 *
 * @reset_id: Reset controller ID
 * @to_us: Timeout in microsecond, or 0 if not waiting
 * Return 0 on success and -ETIMEDOUT if waiting and timeout expired
 */
int stm32mp_reset_deassert(uint32_t reset_id, unsigned int to_us);

/*
 * Release reset control for target resource
 *
 * @reset_id: Reset controller ID
 */
static inline void stm32mp_reset_release(uint32_t reset_id)
{
	(void)stm32mp_reset_deassert(reset_id, 0U);
}

#endif /* STM32MP_RESET_H */
