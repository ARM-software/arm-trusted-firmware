/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include <stdio.h>

#include <common/debug.h>
#include <flash_info.h>
#include "fspi.h"
#include <fspi_api.h>

/*
 * The macros are defined to be used as test vector for testing fspi.
 */
#define	SIZE_BUFFER			0x250

/*
 * You may choose fspi_swap based on core endianness and flexspi IP/AHB
 * buffer endianness set in MCR.
 */
#define fspi_swap32(A)			(A)

void fspi_test(uint32_t fspi_test_addr, uint32_t size, int extra)
{
	uint32_t buffer[SIZE_BUFFER];
	uint32_t count = 1;
	uint32_t failed, i;

	NOTICE("-------------------------- %d----------------------------------\n", count++);
	INFO("Sector Erase size: 0x%08x, size: %d\n", F_SECTOR_ERASE_SZ, size);
	/* Test Sector Erase */
	xspi_sector_erase(fspi_test_addr - fspi_test_addr % F_SECTOR_ERASE_SZ,
			  F_SECTOR_ERASE_SZ);

	/* Test Erased data using IP read */
	xspi_ip_read((fspi_test_addr), buffer, size * 4);

	failed = 0;
	for (i = 0; i < size; i++)
		if (fspi_swap32(0xffffffff) != buffer[i]) {
			failed = 1;
			break;
		}

	if (failed == 0) {
		NOTICE("[%d]: Success Erase: data in buffer[%d] 0x%08x\n", __LINE__, i-3, buffer[i-3]);
	} else {
		ERROR("Erase: Failed  -->xxx with buffer[%d]=0x%08x\n", i, buffer[i]);
	}

	for (i = 0; i < SIZE_BUFFER; i++)
		buffer[i] = 0x12345678;

	/* Write data from buffer to flash */
	xspi_write(fspi_test_addr, (void *)buffer, (size * 4 + extra));
	/* Check written data using IP read */
	xspi_ip_read(fspi_test_addr, buffer, (size * 4 + extra));
	failed = 0;
	for (i = 0; i < size; i++)
		if (fspi_swap32(0x12345678) != buffer[i]) {
			failed = 1;
			break;
		}

	if (failed == 0) {
		NOTICE("[%d]: Success IpWrite with IP READ in buffer[%d] 0x%08x\n", __LINE__, i-3, buffer[i-3]);
	} else {
		ERROR("Write: Failed  -->xxxx with IP READ in buffer[%d]=0x%08x\n", i, buffer[i]);
		return;
	}

	/* xspi_read may use AHB read */
	xspi_read((fspi_test_addr), buffer, (size * 4 + extra));
	failed = 0;
	for (i = 0; i < size; i++)
		if (fspi_swap32(0x12345678) != buffer[i]) {
			failed = 1;
			break;
		}

	if (failed == 0) {
		NOTICE("[%d]: Success IpWrite with AHB OR IP READ on buffer[%d] 0x%08x\n", __LINE__, i-3, buffer[i-3]);
	} else {
		ERROR("Write: Failed  -->xxxx with AHB READ on buffer[%d]=0x%08x\n", i, buffer[i]);
		return;
	}
}
