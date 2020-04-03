/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPI_H
#define SPI_H

#include <stdint.h>

#define SPI_XFER_BEGIN (1 << 0)	/* Assert CS before transfer */
#define SPI_XFER_END   (1 << 1)	/* De-assert CS after transfer */
#define SPI_XFER_QUAD  (1 << 2)

int spi_init(void);
int spi_claim_bus(void);
void spi_release_bus(void);
int spi_xfer(uint32_t bitlen, const void *dout, void *din, uint32_t flags);

#endif /* _SPI_H_ */
