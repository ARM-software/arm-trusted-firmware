/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/* error codes */
#ifndef XSPI_ERROR_CODES_H
#define XSPI_ERROR_CODES_H

#include <errno.h>

typedef enum {
	XSPI_SUCCESS                     = 0,
	XSPI_READ_FAIL			 = ELAST + 1,
	XSPI_ERASE_FAIL,
	XSPI_IP_READ_FAIL,
	XSPI_AHB_READ_FAIL,
	XSPI_IP_WRITE_FAIL,
	XSPI_AHB_WRITE_FAIL,
	XSPI_BLOCK_TIMEOUT,
	XSPI_UNALIGN_ADDR,
	XSPI_UNALIGN_SIZE,
} XSPI_STATUS_CODES;
#undef ELAST
#define ELAST XSPI_STATUS_CODES.XSPI_UNALIGN_SIZE
#endif
