/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef I2C_H
#define I2C_H


void i2c_init(void);

int i2c_read(uint8_t chip,
	     unsigned int addr, int alen, uint8_t *buffer, int len);

int i2c_write(uint8_t chip,
	      unsigned int addr, int alen, uint8_t *buffer, int len);

#endif /* I2C_H */
