/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SUNXI_HELPERS_H__
#define __SUNXI_HELPERS_H__

uint16_t sunxi_read_soc_id(void);
int platform_init_r_twi(uint16_t socid, bool use_i2c);
void sunxi_set_gpio_out(char port, int pin, bool level_high);

#endif
