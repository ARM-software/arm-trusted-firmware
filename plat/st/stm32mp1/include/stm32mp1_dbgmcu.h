/*
 * Copyright (c) 2015-2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_DBGMCU_H
#define STM32MP1_DBGMCU_H

#include <stdint.h>

/* Get chip version and ID from DBGMCU registers */
int stm32mp1_dbgmcu_get_chip_version(uint32_t *chip_version);
int stm32mp1_dbgmcu_get_chip_dev_id(uint32_t *chip_dev_id);

#endif /* STM32MP1_DBGMCU_H */
