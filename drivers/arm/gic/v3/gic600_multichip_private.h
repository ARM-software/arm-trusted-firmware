/*
 * Copyright (c) 2019, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GIC600_MULTICHIP_PRIVATE_H
#define GIC600_MULTICHIP_PRIVATE_H

#include <drivers/arm/gic600_multichip.h>

#include "gicv3_private.h"

/* GIC600 GICD multichip related offsets */
#define GICD_CHIPSR			U(0xC000)
#define GICD_DCHIPR			U(0xC004)
#define GICD_CHIPR			U(0xC008)

/* GIC600 GICD multichip related masks */
#define GICD_CHIPRx_PUP_BIT		BIT_64(1)
#define GICD_CHIPRx_SOCKET_STATE	BIT_64(0)
#define GICD_DCHIPR_PUP_BIT		BIT_32(0)
#define GICD_CHIPSR_RTS_MASK		(BIT_32(4) | BIT_32(5))

/* GIC600 GICD multichip related shifts */
#define GICD_CHIPRx_ADDR_SHIFT		16
#define GICD_CHIPSR_RTS_SHIFT		4
#define GICD_DCHIPR_RT_OWNER_SHIFT	4

/* Other shifts and masks remain the same between GIC-600 and GIC-700. */
#define GIC_700_SPI_BLOCK_MIN_SHIFT	9
#define GIC_700_SPI_BLOCKS_SHIFT	3
#define GIC_600_SPI_BLOCK_MIN_SHIFT	10
#define GIC_600_SPI_BLOCKS_SHIFT	5

#define GICD_CHIPSR_RTS_STATE_DISCONNECTED	U(0)
#define GICD_CHIPSR_RTS_STATE_UPDATING		U(1)
#define GICD_CHIPSR_RTS_STATE_CONSISTENT	U(2)

/* SPI interrupt id minimum and maximum range */
#define GIC600_SPI_ID_MIN		32
#define GIC600_SPI_ID_MAX		960

/* Number of retries for PUP update */
#define GICD_PUP_UPDATE_RETRIES		10000

#define SPI_MIN_INDEX			0
#define SPI_MAX_INDEX			1

#define SPI_BLOCK_MIN_VALUE(spi_id_min) \
			(((spi_id_min) - GIC600_SPI_ID_MIN) / \
			GIC600_SPI_ID_MIN)
#define SPI_BLOCKS_VALUE(spi_id_min, spi_id_max) \
			(((spi_id_max) - (spi_id_min) + 1) / \
			GIC600_SPI_ID_MIN)
#define GICD_CHIPR_VALUE_GIC_700(chip_addr, spi_block_min, spi_blocks) \
			(((chip_addr) << GICD_CHIPRx_ADDR_SHIFT) | \
			((spi_block_min) << GIC_700_SPI_BLOCK_MIN_SHIFT) | \
			((spi_blocks) << GIC_700_SPI_BLOCKS_SHIFT))
#define GICD_CHIPR_VALUE_GIC_600(chip_addr, spi_block_min, spi_blocks) \
			(((chip_addr) << GICD_CHIPRx_ADDR_SHIFT) | \
			((spi_block_min) << GIC_600_SPI_BLOCK_MIN_SHIFT) | \
			((spi_blocks) << GIC_600_SPI_BLOCKS_SHIFT))

/*
 * Multichip data assertion macros
 */
/* Set bits from 0 to ((spi_id_max + 1) / 32) */
#define SPI_BLOCKS_TILL_MAX(spi_id_max)	((1 << (((spi_id_max) + 1) >> 5)) - 1)
/* Set bits from 0 to (spi_id_min / 32) */
#define SPI_BLOCKS_TILL_MIN(spi_id_min)	((1 << ((spi_id_min) >> 5)) - 1)
/* Set bits from (spi_id_min / 32) to ((spi_id_max + 1) / 32) */
#define BLOCKS_OF_32(spi_id_min, spi_id_max) \
					SPI_BLOCKS_TILL_MAX(spi_id_max) ^ \
					SPI_BLOCKS_TILL_MIN(spi_id_min)

/*******************************************************************************
 * GIC-600 multichip operation related helper functions
 ******************************************************************************/
static inline uint32_t read_gicd_dchipr(uintptr_t base)
{
	return mmio_read_32(base + GICD_DCHIPR);
}

static inline uint64_t read_gicd_chipr_n(uintptr_t base, uint8_t n)
{
	return mmio_read_64(base + (GICD_CHIPR + (8U * n)));
}

static inline uint32_t read_gicd_chipsr(uintptr_t base)
{
	return mmio_read_32(base + GICD_CHIPSR);
}

static inline void write_gicd_dchipr(uintptr_t base, uint32_t val)
{
	mmio_write_32(base + GICD_DCHIPR, val);
}

static inline void write_gicd_chipr_n(uintptr_t base, uint8_t n, uint64_t val)
{
	mmio_write_64(base + (GICD_CHIPR + (8U * n)), val);
}

#endif /* GIC600_MULTICHIP_PRIVATE_H */
