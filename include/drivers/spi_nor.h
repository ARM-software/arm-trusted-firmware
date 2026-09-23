/*
 * Copyright (c) 2019-2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRIVERS_SPI_NOR_H
#define DRIVERS_SPI_NOR_H

#include <drivers/spi_mem.h>

/* OPCODE */
#define SPI_NOR_OP_WREN			0x06U	/* Write enable */
#define SPI_NOR_OP_WRSR			0x01U	/* Write status register 1 byte */
#define SPI_NOR_OP_READ_ID		0x9FU	/* Read JEDEC ID */
#define SPI_NOR_OP_READ_CR		0x35U	/* Read configuration register */
#define SPI_NOR_OP_READ_SR		0x05U	/* Read status register */
#define SPI_NOR_OP_READ_FSR		0x70U	/* Read flag status register */
#define SPINOR_OP_RDEAR			0xC8U	/* Read Extended Address Register */
#define SPINOR_OP_WREAR			0xC5U	/* Write Extended Address Register */
#define SPI_NOR_OP_SRSTEN		0x66U	/* Software Reset Enable */
#define SPI_NOR_OP_SRST			0x99U	/* Software Reset */

/* Used for Spansion flashes only. */
#define SPINOR_OP_BRWR			0x17U	/* Bank register write */
#define SPINOR_OP_BRRD			0x16U	/* Bank register read */

#define SPI_NOR_OP_READ			0x03U	/* Read data bytes (low frequency) */
#define SPI_NOR_OP_READ_FAST		0x0BU	/* Read data bytes (high frequency) */
#define SPI_NOR_OP_READ_1_1_2		0x3BU	/* Read data bytes (Dual Output SPI) */
#define SPI_NOR_OP_READ_1_2_2		0xBBU	/* Read data bytes (Dual I/O SPI) */
#define SPI_NOR_OP_READ_1_1_4		0x6BU	/* Read data bytes (Quad Output SPI) */
#define SPI_NOR_OP_READ_1_4_4		0xEBU	/* Read data bytes (Quad I/O SPI) */
#define SPI_NOR_OP_READ_1_1_8		0x8BU	/* Read data bytes (Octal Output SPI) */
#define SPI_NOR_OP_READ_1_8_8		0xCBU	/* Read data bytes (Octal I/O SPI) */

/* 4-bytes address opcodes */
#define SPI_NOR_OP_READ_4B		0x13U	/* Read data bytes (low frequency) */
#define SPI_NOR_OP_READ_FAST_4B		0x0CU	/* Read data bytes (high frequency) */
#define SPI_NOR_OP_DT_READ_8_8_8	0xEEU	/* Read data bytes (Octal DTR I/O SPI) */

/* Used for Macronix and Winbond flashes. */
#define SPI_NOR_OP_WR_CR2		0x72U		/* Write configuration register 2 */
#define SPI_NOR_REG_MXIC_CR2_MODE	0x00000000U	/* For setting octal DTR mode */
#define SPI_NOR_REG_MXIC_OPI_DTR_EN	0x2U		/* Enable Octal DTR */
#define SPI_NOR_REG_MXIC_CR2_DC		0x00000300U	/* For setting dummy cycles */
#define SPI_NOR_REG_MXIC_DC_20		0x0U		/* Setting dummy cycles to 20 */

/* Flags for NOR specific configuration */
#define SPI_NOR_USE_FSR			BIT(0)
#define SPI_NOR_USE_BANK		BIT(1)

struct nor_device {
	struct spi_mem_op read_op;
	uint32_t size;
	uint32_t flags;
	uint8_t selected_bank;
	uint8_t bank_write_cmd;
	uint8_t bank_read_cmd;
};

int spi_nor_read(unsigned int offset, uintptr_t buffer, size_t length,
		 size_t *length_read);
int spi_nor_init(unsigned long long *device_size, unsigned int *erase_size);
int spi_nor_reset(void);

/*
 * Platform can implement this to override default NOR instance configuration.
 *
 * @device: target NOR instance.
 * Return 0 on success, negative value otherwise.
 */
int plat_get_nor_data(struct nor_device *device);

#endif /* DRIVERS_SPI_NOR_H */
