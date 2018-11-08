/*
 * Copyright (C) 2018 Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
/* This driver provides I2C support for Allwinner sunXi SoCs */

#ifndef MENTOR_I2C_PLAT_H
#define MENTOR_I2C_PLAT_H

#define CONFIG_SYS_TCLK			24000000
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		0

#define I2C_INTERRUPT_CLEAR_INVERTED

struct  mentor_i2c_regs {
	uint32_t slave_address;
	uint32_t xtnd_slave_addr;
	uint32_t data;
	uint32_t control;
	uint32_t status;
	uint32_t baudrate;
	uint32_t soft_reset;
};

#endif /* MENTOR_I2C_PLAT_H */
