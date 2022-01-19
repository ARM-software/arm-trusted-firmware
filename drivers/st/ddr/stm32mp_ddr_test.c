/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/st/stm32mp_ddr_test.h>
#include <lib/mmio.h>

#include <platform_def.h>

#define DDR_PATTERN	0xAAAAAAAAU
#define DDR_ANTIPATTERN	0x55555555U

/*******************************************************************************
 * This function tests a simple read/write access to the DDR.
 * Note that the previous content is restored after test.
 * Returns 0 if success, and address value else.
 ******************************************************************************/
uint32_t stm32mp_ddr_test_rw_access(void)
{
	uint32_t saved_value = mmio_read_32(STM32MP_DDR_BASE);

	mmio_write_32(STM32MP_DDR_BASE, DDR_PATTERN);

	if (mmio_read_32(STM32MP_DDR_BASE) != DDR_PATTERN) {
		return (uint32_t)STM32MP_DDR_BASE;
	}

	mmio_write_32(STM32MP_DDR_BASE, saved_value);

	return 0U;
}

/*******************************************************************************
 * This function tests the DDR data bus wiring.
 * This is inspired from the Data Bus Test algorithm written by Michael Barr
 * in "Programming Embedded Systems in C and C++" book.
 * resources.oreilly.com/examples/9781565923546/blob/master/Chapter6/
 * File: memtest.c - This source code belongs to Public Domain.
 * Returns 0 if success, and address value else.
 ******************************************************************************/
uint32_t stm32mp_ddr_test_data_bus(void)
{
	uint32_t pattern;

	for (pattern = 1U; pattern != 0U; pattern <<= 1U) {
		mmio_write_32(STM32MP_DDR_BASE, pattern);

		if (mmio_read_32(STM32MP_DDR_BASE) != pattern) {
			return (uint32_t)STM32MP_DDR_BASE;
		}
	}

	return 0;
}

/*******************************************************************************
 * This function tests the DDR address bus wiring.
 * This is inspired from the Data Bus Test algorithm written by Michael Barr
 * in "Programming Embedded Systems in C and C++" book.
 * resources.oreilly.com/examples/9781565923546/blob/master/Chapter6/
 * File: memtest.c - This source code belongs to Public Domain.
 * size: size in bytes of the DDR memory device.
 * Returns 0 if success, and address value else.
 ******************************************************************************/
uint32_t stm32mp_ddr_test_addr_bus(uint64_t size)
{
	uint64_t addressmask = size - 1U;
	uint64_t offset;
	uint64_t testoffset = 0U;

	/* Write the default pattern at each of the power-of-two offsets. */
	for (offset = sizeof(uint32_t); (offset & addressmask) != 0U;
	     offset <<= 1U) {
		mmio_write_32(STM32MP_DDR_BASE + (uint32_t)offset,
			      DDR_PATTERN);
	}

	/* Check for address bits stuck high. */
	mmio_write_32(STM32MP_DDR_BASE + (uint32_t)testoffset,
		      DDR_ANTIPATTERN);

	for (offset = sizeof(uint32_t); (offset & addressmask) != 0U;
	     offset <<= 1U) {
		if (mmio_read_32(STM32MP_DDR_BASE + (uint32_t)offset) !=
		    DDR_PATTERN) {
			return (uint32_t)(STM32MP_DDR_BASE + offset);
		}
	}

	mmio_write_32(STM32MP_DDR_BASE + (uint32_t)testoffset, DDR_PATTERN);

	/* Check for address bits stuck low or shorted. */
	for (testoffset = sizeof(uint32_t); (testoffset & addressmask) != 0U;
	     testoffset <<= 1U) {
		mmio_write_32(STM32MP_DDR_BASE + (uint32_t)testoffset,
			      DDR_ANTIPATTERN);

		if (mmio_read_32(STM32MP_DDR_BASE) != DDR_PATTERN) {
			return STM32MP_DDR_BASE;
		}

		for (offset = sizeof(uint32_t); (offset & addressmask) != 0U;
		     offset <<= 1) {
			if ((mmio_read_32(STM32MP_DDR_BASE +
					  (uint32_t)offset) != DDR_PATTERN) &&
			    (offset != testoffset)) {
				return (uint32_t)(STM32MP_DDR_BASE + offset);
			}
		}

		mmio_write_32(STM32MP_DDR_BASE + (uint32_t)testoffset,
			      DDR_PATTERN);
	}

	return 0U;
}

/*******************************************************************************
 * This function checks the DDR size. It has to be run with Data Cache off.
 * This test is run before data have been put in DDR, and is only done for
 * cold boot. The DDR data can then be overwritten, and it is not useful to
 * restore its content.
 * Returns DDR computed size.
 ******************************************************************************/
uint32_t stm32mp_ddr_check_size(void)
{
	uint32_t offset = sizeof(uint32_t);

	mmio_write_32(STM32MP_DDR_BASE, DDR_PATTERN);

	while (offset < STM32MP_DDR_MAX_SIZE) {
		mmio_write_32(STM32MP_DDR_BASE + offset, DDR_ANTIPATTERN);
		dsb();

		if (mmio_read_32(STM32MP_DDR_BASE) != DDR_PATTERN) {
			break;
		}

		offset <<= 1U;
	}

	INFO("Memory size = 0x%x (%u MB)\n", offset, offset / (1024U * 1024U));

	return offset;
}
