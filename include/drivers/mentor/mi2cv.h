/*
 * Copyright (C) 2018 Marvell International Ltd.
 * Copyright (C) 2018 Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* This driver provides support for Mentor Graphics MI2CV IP core */

#ifndef MI2CV_H
#define MI2CV_H

#include <stdint.h>

/*
 * Initialization, must be called once on start up, may be called
 * repeatedly to change the speed and slave addresses.
 */
void i2c_init(void *i2c_base);

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int i2c_read(uint8_t chip,
	     unsigned int addr, int alen, uint8_t *buffer, int len);

int i2c_write(uint8_t chip,
	      unsigned int addr, int alen, uint8_t *buffer, int len);

#endif /* MI2CV_H */
