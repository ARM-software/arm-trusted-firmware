/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */
/* This driver provides I2C support for Marvell A8K and compatible SoCs */

#ifndef MENTOR_I2C_PLAT_H
#define MENTOR_I2C_PLAT_H

#define CONFIG_SYS_TCLK			250000000
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		0x0

#define I2C_CAN_UNSTUCK

struct  mentor_i2c_regs {
	uint32_t slave_address;
	uint32_t data;
	uint32_t control;
	union {
		uint32_t status;	/* when reading */
		uint32_t baudrate;	/* when writing */
	};
	uint32_t xtnd_slave_addr;
	uint32_t reserved[2];
	uint32_t soft_reset;
	uint8_t  reserved2[0xa0 - 0x20];
	uint32_t unstuck;
};

#endif /* MENTOR_I2C_PLAT_H */
