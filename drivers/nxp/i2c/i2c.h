/*
 * Copyright 2016-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */


#ifndef I2C_H
#define I2C_H

#define I2C_TIMEOUT	1000	/* ms */

#define I2C_FD_CONSERV	0x7e
#define I2C_CR_DIS	(1 << 7)
#define I2C_CR_EN	(0 << 7)
#define I2C_CR_MA	(1 << 5)
#define I2C_CR_TX	(1 << 4)
#define I2C_CR_TX_NAK	(1 << 3)
#define I2C_CR_RSTA	(1 << 2)
#define I2C_SR_BB	(1 << 5)
#define I2C_SR_IDLE	(0 << 5)
#define I2C_SR_AL	(1 << 4)
#define I2C_SR_IF	(1 << 1)
#define I2C_SR_RX_NAK	(1 << 0)
#define I2C_SR_RST	(I2C_SR_AL | I2C_SR_IF)

#define I2C_GLITCH_EN 0x8

struct ls_i2c {
	unsigned char ad;
	unsigned char fd;
	unsigned char cr;
	unsigned char sr;
	unsigned char dr;
	unsigned char ic;
	unsigned char dbg;
};

void i2c_init(void);
int i2c_read(unsigned char chip, int addr, int alen,
	     unsigned char *buf, int len);
int i2c_write(unsigned char chip, int addr, int alen,
	      const unsigned char *buf, int len);
int i2c_probe_chip(unsigned char chip);

#endif /* I2C_H */
