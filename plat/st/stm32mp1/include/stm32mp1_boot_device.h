/*
 * Copyright (c) 2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_BOOT_DEVICE_H
#define STM32MP1_BOOT_DEVICE_H

#include <drivers/raw_nand.h>
#include <drivers/spi_nand.h>

int plat_get_raw_nand_data(struct rawnand_device *device);
int plat_get_spi_nand_data(struct spinand_device *device);

#endif /* STM32MP1_BOOT_DEVICE_H */
