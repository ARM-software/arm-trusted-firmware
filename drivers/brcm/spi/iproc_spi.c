/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <spi.h>

#include "iproc_qspi.h"

int spi_init(void)
{
	return iproc_qspi_setup(IPROC_QSPI_BUS, IPROC_QSPI_CS,
				IPROC_QSPI_CLK_SPEED, IPROC_QSPI_MODE0);
}

int spi_claim_bus(void)
{
	return iproc_qspi_claim_bus();
}

void spi_release_bus(void)
{
	iproc_qspi_release_bus();
}

int spi_xfer(uint32_t bitlen, const void *dout,
	     void *din, uint32_t flags)
{
	return iproc_qspi_xfer(bitlen, dout, din, flags);
}
