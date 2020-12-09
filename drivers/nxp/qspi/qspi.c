/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <qspi.h>

int qspi_io_setup(uintptr_t nxp_qspi_flash_addr,
		  size_t nxp_qspi_flash_size,
		  uintptr_t fip_offset)
{
	uint32_t qspi_mcr_val = qspi_in32(CHS_QSPI_MCR);

	/* Enable and change endianness of QSPI IP */
	qspi_out32(CHS_QSPI_MCR, (qspi_mcr_val | CHS_QSPI_64LE));

	/* Adding QSPI Memory Map in XLAT Table */
	mmap_add_region(nxp_qspi_flash_addr, nxp_qspi_flash_addr,
			nxp_qspi_flash_size, MT_MEMORY | MT_RW);

	return 0;
}
