/*
 * Copyright (c) 2017-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __STM32MP1_PMIC_H__
#define __STM32MP1_PMIC_H__

#include <stdbool.h>

bool dt_check_pmic(void);
int dt_pmic_enable_boot_on_regulators(void);
void initialize_pmic_i2c(void);
void initialize_pmic(void);
int pmic_ddr_power_init(enum ddr_type ddr_type);

#endif /* __STM32MP1_PMIC_H__ */
