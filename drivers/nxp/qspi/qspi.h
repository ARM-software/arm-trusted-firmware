/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef QSPI_H
#define QSPI_H

#include <endian.h>
#include <lib/mmio.h>

#define CHS_QSPI_MCR			0x01550000
#define CHS_QSPI_64LE			0xC

#ifdef NXP_QSPI_BE
#define qspi_in32(a)           bswap32(mmio_read_32((uintptr_t)(a)))
#define qspi_out32(a, v)       mmio_write_32((uintptr_t)(a), bswap32(v))
#elif defined(NXP_QSPI_LE)
#define qspi_in32(a)           mmio_read_32((uintptr_t)(a))
#define qspi_out32(a, v)       mmio_write_32((uintptr_t)(a), (v))
#else
#error Please define CCSR QSPI register endianness
#endif

int qspi_io_setup(uintptr_t nxp_qspi_flash_addr,
		  size_t nxp_qspi_flash_size,
		  uintptr_t fip_offset);
#endif /* __QSPI_H__ */
