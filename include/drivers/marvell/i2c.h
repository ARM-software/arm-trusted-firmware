/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef _I2C_H_
#define _I2C_H_


void i2c_init(void);

int i2c_read(uint8_t chip,
	     unsigned int addr, int alen, uint8_t *buffer, int len);

int i2c_write(uint8_t chip,
	      unsigned int addr, int alen, uint8_t *buffer, int len);
#endif
