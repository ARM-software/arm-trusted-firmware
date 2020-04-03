/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IPROC_QSPI_H
#define IPROC_QSPI_H

#include <platform_def.h>

/*SPI configuration enable*/
#define IPROC_QSPI_CLK_SPEED	62500000
#define SPI_CPHA		(1 << 0)
#define SPI_CPOL		(1 << 1)
#define IPROC_QSPI_MODE0	0
#define IPROC_QSPI_MODE3	(SPI_CPOL|SPI_CPHA)

#define IPROC_QSPI_BUS                   0
#define IPROC_QSPI_CS                    0
#define IPROC_QSPI_BASE_REG              QSPI_CTRL_BASE_ADDR
#define IPROC_QSPI_CRU_CONTROL_REG       QSPI_CLK_CTRL

#define QSPI_AXI_CLK                        200000000

#define QSPI_RETRY_COUNT_US_MAX             200000

/* Chip attributes */
#define QSPI_REG_BASE			IPROC_QSPI_BASE_REG
#define CRU_CONTROL_REG			IPROC_QSPI_CRU_CONTROL_REG
#define SPBR_DIV_MIN			8U
#define SPBR_DIV_MAX			255U
#define NUM_CDRAM_BYTES			16U

/* Register fields */
#define MSPI_SPCR0_MSB_BITS_8		0x00000020

/* Flash opcode and parameters */
#define CDRAM_PCS0			2
#define CDRAM_CONT			(1 << 7)
#define CDRAM_BITS_EN			(1 << 6)
#define CDRAM_QUAD_MODE			(1 << 8)
#define CDRAM_RBIT_INPUT		(1 << 10)

/* MSPI registers */
#define QSPI_MSPI_MODE_REG_BASE		(QSPI_REG_BASE + 0x200)
#define MSPI_SPCR0_LSB_REG		0x000
#define MSPI_SPCR0_MSB_REG		0x004
#define MSPI_SPCR1_LSB_REG		0x008
#define MSPI_SPCR1_MSB_REG		0x00c
#define MSPI_NEWQP_REG			0x010
#define MSPI_ENDQP_REG			0x014
#define MSPI_SPCR2_REG			0x018
#define MSPI_STATUS_REG			0x020
#define MSPI_CPTQP_REG			0x024
#define MSPI_TXRAM_REG			0x040
#define MSPI_RXRAM_REG			0x0c0
#define MSPI_CDRAM_REG			0x140
#define MSPI_WRITE_LOCK_REG		0x180
#define MSPI_DISABLE_FLUSH_GEN_REG	0x184

#define MSPI_SPCR0_MSB_REG_MSTR_SHIFT		7
#define MSPI_SPCR0_MSB_REG_16_BITS_PER_WD_SHIFT	(0 << 2)
#define MSPI_SPCR0_MSB_REG_MODE_MASK		0x3

/* BSPI registers */
#define QSPI_BSPI_MODE_REG_BASE		QSPI_REG_BASE
#define BSPI_MAST_N_BOOT_CTRL_REG	0x008
#define BSPI_BUSY_STATUS_REG		0x00c

#define MSPI_CMD_COMPLETE_MASK		1
#define BSPI_BUSY_MASK			1
#define MSPI_CTRL_MASK			1

#define MSPI_SPE			(1 << 6)
#define MSPI_CONT_AFTER_CMD		(1 << 7)

/* State */
enum bcm_qspi_state {
	QSPI_STATE_DISABLED,
	QSPI_STATE_MSPI,
	QSPI_STATE_BSPI
};

/* QSPI private data */
struct bcmspi_priv {
	/* Specified SPI parameters */
	uint32_t max_hz;
	uint32_t spi_mode;

	/* State */
	enum bcm_qspi_state state;
	int mspi_16bit;

	/* Registers */
	uintptr_t mspi_hw;
	uintptr_t bspi_hw;
};

int iproc_qspi_setup(uint32_t bus, uint32_t cs,
		     uint32_t max_hz, uint32_t mode);
int iproc_qspi_claim_bus(void);
void iproc_qspi_release_bus(void);
int iproc_qspi_xfer(uint32_t bitlen, const void *dout,
		    void *din, unsigned long flags);

#endif	/* _IPROC_QSPI_H_ */
