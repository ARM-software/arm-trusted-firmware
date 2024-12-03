/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "drivers/gpio_spi.h"

#ifndef SLB9670_GPIO_H
#define SLB9670_GPIO_H

void tpm2_slb9670_reset_chip(struct gpio_spi_data *tpm_gpio_data);

void tpm2_slb9670_gpio_init(struct gpio_spi_data *tpm_gpio_data);

#endif /* SLB9670_GPIO_H */
