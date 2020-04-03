/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <endian.h>
#include <lib/mmio.h>

#include <platform_def.h>
#include <spi.h>

#include "iproc_qspi.h"

struct bcmspi_priv spi_cfg;

/* Redefined by platform to force appropriate information */
#pragma weak plat_spi_init
int plat_spi_init(uint32_t *max_hz)
{
	return 0;
}

/* Initialize & setup iproc qspi controller */
int iproc_qspi_setup(uint32_t bus, uint32_t cs, uint32_t max_hz, uint32_t mode)
{
	struct bcmspi_priv *priv = NULL;
	uint32_t spbr;

	priv = &spi_cfg;
	priv->spi_mode = mode;
	priv->state = QSPI_STATE_DISABLED;
	priv->bspi_hw = QSPI_BSPI_MODE_REG_BASE;
	priv->mspi_hw = QSPI_MSPI_MODE_REG_BASE;

	/* Initialize clock and platform specific */
	if (plat_spi_init(&max_hz) != 0)
		return -1;

	priv->max_hz = max_hz;

	/* MSPI: Basic hardware initialization */
	mmio_write_32(priv->mspi_hw + MSPI_SPCR1_LSB_REG, 0);
	mmio_write_32(priv->mspi_hw + MSPI_SPCR1_MSB_REG, 0);
	mmio_write_32(priv->mspi_hw + MSPI_NEWQP_REG, 0);
	mmio_write_32(priv->mspi_hw + MSPI_ENDQP_REG, 0);
	mmio_write_32(priv->mspi_hw + MSPI_SPCR2_REG, 0);

	/* MSPI: SCK configuration */
	spbr = (QSPI_AXI_CLK - 1) / (2 * priv->max_hz) + 1;
	spbr = MIN(spbr, SPBR_DIV_MAX);
	spbr = MAX(spbr, SPBR_DIV_MIN);
	mmio_write_32(priv->mspi_hw + MSPI_SPCR0_LSB_REG, spbr);

	/* MSPI: Mode configuration (8 bits by default) */
	priv->mspi_16bit = 0;
	mmio_write_32(priv->mspi_hw + MSPI_SPCR0_MSB_REG,
		      BIT(MSPI_SPCR0_MSB_REG_MSTR_SHIFT) |		/* Master */
		      MSPI_SPCR0_MSB_REG_16_BITS_PER_WD_SHIFT |		/* 16 bits per word */
		      (priv->spi_mode & MSPI_SPCR0_MSB_REG_MODE_MASK));	/* mode: CPOL / CPHA */

	/* Display bus info */
	VERBOSE("SPI: SPCR0_LSB: 0x%x\n",
		mmio_read_32(priv->mspi_hw + MSPI_SPCR0_LSB_REG));
	VERBOSE("SPI: SPCR0_MSB: 0x%x\n",
		mmio_read_32(priv->mspi_hw + MSPI_SPCR0_MSB_REG));
	VERBOSE("SPI: SPCR1_LSB: 0x%x\n",
		mmio_read_32(priv->mspi_hw + MSPI_SPCR1_LSB_REG));
	VERBOSE("SPI: SPCR1_MSB: 0x%x\n",
		mmio_read_32(priv->mspi_hw + MSPI_SPCR1_MSB_REG));
	VERBOSE("SPI: SPCR2: 0x%x\n",
		mmio_read_32(priv->mspi_hw + MSPI_SPCR2_REG));
	VERBOSE("SPI: CLK: %d\n", priv->max_hz);

	return 0;
}

void bcmspi_enable_bspi(struct bcmspi_priv *priv)
{
	if (priv->state != QSPI_STATE_BSPI) {
		/* Switch to BSPI */
		mmio_write_32(priv->bspi_hw + BSPI_MAST_N_BOOT_CTRL_REG, 0);

		priv->state = QSPI_STATE_BSPI;
	}
}

static int bcmspi_disable_bspi(struct bcmspi_priv *priv)
{
	uint32_t retry;

	if (priv->state == QSPI_STATE_MSPI)
		return 0;

	/* Switch to MSPI if not yet */
	if ((mmio_read_32(priv->bspi_hw + BSPI_MAST_N_BOOT_CTRL_REG) &
	     MSPI_CTRL_MASK) == 0) {
		retry = QSPI_RETRY_COUNT_US_MAX;
		do {
			if ((mmio_read_32(
				priv->bspi_hw + BSPI_BUSY_STATUS_REG) &
				BSPI_BUSY_MASK) == 0) {
				mmio_write_32(priv->bspi_hw +
				       BSPI_MAST_N_BOOT_CTRL_REG,
				       MSPI_CTRL_MASK);
				udelay(1);
				break;
			}
			udelay(1);
		} while (retry--);

		if ((mmio_read_32(priv->bspi_hw + BSPI_MAST_N_BOOT_CTRL_REG) &
		     MSPI_CTRL_MASK) != MSPI_CTRL_MASK) {
			ERROR("QSPI: Switching to QSPI error.\n");
			return -1;
		}
	}

	/* Update state */
	priv->state = QSPI_STATE_MSPI;

	return 0;
}

int iproc_qspi_claim_bus(void)
{
	struct bcmspi_priv *priv = &spi_cfg;

	/* Switch to MSPI by default */
	if (bcmspi_disable_bspi(priv) != 0)
		return -1;

	return 0;
}

void iproc_qspi_release_bus(void)
{
	struct bcmspi_priv *priv = &spi_cfg;

	/* Switch to BSPI by default */
	bcmspi_enable_bspi(priv);
}

static int mspi_xfer(struct bcmspi_priv *priv, uint32_t bytes,
		     const uint8_t *tx, uint8_t *rx, uint32_t flag)
{
	uint32_t retry;
	uint32_t mode = CDRAM_PCS0;

	if (flag & SPI_XFER_QUAD) {
		mode |= CDRAM_QUAD_MODE;
		VERBOSE("SPI: QUAD mode\n");

		if (!tx) {
			VERBOSE("SPI: 4 lane input\n");
			mode |= CDRAM_RBIT_INPUT;
		}
	}

	/* Use 8-bit queue for odd-bytes transfer */
	if (bytes & 1)
		priv->mspi_16bit = 0;
	else {
		priv->mspi_16bit = 1;
		mode |= CDRAM_BITS_EN;
	}

	while (bytes) {
		uint32_t chunk;
		uint32_t queues;
		uint32_t i;

		/* Separate code for 16bit and 8bit transfers for performance */
		if (priv->mspi_16bit) {
			VERBOSE("SPI: 16 bits xfer\n");
			/* Determine how many bytes to process this time */
			chunk = MIN(bytes, NUM_CDRAM_BYTES * 2);
			queues = (chunk - 1) / 2 + 1;
			bytes -= chunk;

			/* Fill CDRAMs */
			for (i = 0; i < queues; i++)
				mmio_write_32(priv->mspi_hw + MSPI_CDRAM_REG +
					      (i << 2), mode | CDRAM_CONT);

			/* Fill TXRAMs */
			for (i = 0; i < chunk; i++)
				if (tx)
					mmio_write_32(priv->mspi_hw +
						MSPI_TXRAM_REG +
						(i << 2), tx[i]);
		} else {
			VERBOSE("SPI: 8 bits xfer\n");
			/* Determine how many bytes to process this time */
			chunk = MIN(bytes, NUM_CDRAM_BYTES);
			queues = chunk;
			bytes -= chunk;

			/* Fill CDRAMs and TXRAMS */
			for (i = 0; i < chunk; i++) {
				mmio_write_32(priv->mspi_hw + MSPI_CDRAM_REG +
					      (i << 2), mode | CDRAM_CONT);
				if (tx)
					mmio_write_32(priv->mspi_hw +
						MSPI_TXRAM_REG +
						(i << 3), tx[i]);
			}
		}

		/* Advance pointers */
		if (tx)
			tx += chunk;

		/* Setup queue pointers */
		mmio_write_32(priv->mspi_hw + MSPI_NEWQP_REG, 0);
		mmio_write_32(priv->mspi_hw + MSPI_ENDQP_REG, queues - 1);

		/* Remove CONT on the last byte command */
		if (bytes == 0 && (flag & SPI_XFER_END))
			mmio_write_32(priv->mspi_hw + MSPI_CDRAM_REG +
				      ((queues - 1) << 2), mode);

		/* Kick off */
		mmio_write_32(priv->mspi_hw + MSPI_STATUS_REG, 0);
		if (bytes == 0 && (flag & SPI_XFER_END))
			mmio_write_32(priv->mspi_hw + MSPI_SPCR2_REG, MSPI_SPE);
		else
			mmio_write_32(priv->mspi_hw + MSPI_SPCR2_REG,
				      MSPI_SPE | MSPI_CONT_AFTER_CMD);

		/* Wait for completion */
		retry = QSPI_RETRY_COUNT_US_MAX;
		do {
			if (mmio_read_32(priv->mspi_hw + MSPI_STATUS_REG) &
			    MSPI_CMD_COMPLETE_MASK)
				break;
			udelay(1);
		} while (retry--);

		if ((mmio_read_32(priv->mspi_hw + MSPI_STATUS_REG) &
		     MSPI_CMD_COMPLETE_MASK) == 0) {
			ERROR("SPI: Completion timeout.\n");
			return -1;
		}

		/* Read data out */
		if (rx) {
			if (priv->mspi_16bit) {
				for (i = 0; i < chunk; i++) {
					rx[i] = mmio_read_32(priv->mspi_hw +
							MSPI_RXRAM_REG +
						       (i << 2))
						& 0xff;
				}
			} else {
				for (i = 0; i < chunk; i++) {
					rx[i] = mmio_read_32(priv->mspi_hw +
						       MSPI_RXRAM_REG +
						       (((i << 1) + 1) << 2))
						& 0xff;
				}
			}
			rx += chunk;
		}
	}

	return 0;
}

int iproc_qspi_xfer(uint32_t bitlen,
		    const void *dout, void *din, unsigned long flags)
{
	struct bcmspi_priv *priv;
	const uint8_t *tx = dout;
	uint8_t *rx = din;
	uint32_t bytes = bitlen / 8;
	int ret = 0;

	priv = &spi_cfg;

	if (priv->state == QSPI_STATE_DISABLED) {
		ERROR("QSPI: state disabled\n");
		return -1;
	}

	/* we can only do 8 bit transfers */
	if (bitlen % 8) {
		ERROR("QSPI: Only support 8 bit transfers (requested %d)\n",
			bitlen);
		return -1;
	}

	/* MSPI: Enable write lock at the beginning */
	if (flags & SPI_XFER_BEGIN) {
		/* Switch to MSPI if not yet */
		if (bcmspi_disable_bspi(priv) != 0) {
			ERROR("QSPI: Switch to MSPI failed\n");
			return -1;
		}

		mmio_write_32(priv->mspi_hw + MSPI_WRITE_LOCK_REG, 1);
	}

	/* MSPI: Transfer it */
	if (bytes)
		ret = mspi_xfer(priv, bytes, tx, rx, flags);

	/* MSPI: Disable write lock if it's done */
	if (flags & SPI_XFER_END)
		mmio_write_32(priv->mspi_hw + MSPI_WRITE_LOCK_REG, 0);

	return ret;
}
