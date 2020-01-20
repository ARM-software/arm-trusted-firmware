/*
 * Copyright (c) 2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRIVERS_SPI_NAND_H
#define DRIVERS_SPI_NAND_H

#include <drivers/nand.h>
#include <drivers/spi_mem.h>

#define SPI_NAND_OP_GET_FEATURE		0x0FU
#define SPI_NAND_OP_SET_FEATURE		0x1FU
#define SPI_NAND_OP_READ_ID		0x9FU
#define SPI_NAND_OP_LOAD_PAGE		0x13U
#define SPI_NAND_OP_RESET		0xFFU
#define SPI_NAND_OP_READ_FROM_CACHE	0x03U
#define SPI_NAND_OP_READ_FROM_CACHE_2X	0x3BU
#define SPI_NAND_OP_READ_FROM_CACHE_4X	0x6BU

/* Configuration register */
#define SPI_NAND_REG_CFG		0xB0U
#define SPI_NAND_CFG_ECC_EN		BIT(4)
#define SPI_NAND_CFG_QE			BIT(0)

/* Status register */
#define SPI_NAND_REG_STATUS		0xC0U
#define SPI_NAND_STATUS_BUSY		BIT(0)
#define SPI_NAND_STATUS_ECC_UNCOR	BIT(5)

struct spinand_device {
	struct nand_device *nand_dev;
	struct spi_mem_op spi_read_cache_op;
	uint8_t cfg_cache; /* Cached value of SPI NAND device register CFG */
};

int spi_nand_init(unsigned long long *size, unsigned int *erase_size);

/*
 * Platform can implement this to override default SPI-NAND instance
 * configuration.
 *
 * @device: target SPI-NAND instance.
 * Return 0 on success, negative value otherwise.
 */
int plat_get_spi_nand_data(struct spinand_device *device);

#endif /* DRIVERS_SPI_NAND_H */
