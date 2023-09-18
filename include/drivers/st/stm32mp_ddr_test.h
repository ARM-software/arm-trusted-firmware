/*
 * Copyright (C) 2022-2023, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_DDR_TEST_H
#define STM32MP_DDR_TEST_H

#include <stdint.h>

uintptr_t stm32mp_ddr_test_rw_access(void);
uintptr_t stm32mp_ddr_test_data_bus(void);
uintptr_t stm32mp_ddr_test_addr_bus(size_t size);
size_t stm32mp_ddr_check_size(void);

#endif /* STM32MP_DDR_TEST_H */
