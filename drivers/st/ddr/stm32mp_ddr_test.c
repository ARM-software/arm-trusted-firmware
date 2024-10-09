/*
 * Copyright (C) 2022-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/st/stm32mp_ddr_test.h>
#include <lib/mmio.h>

#include <platform_def.h>

#ifdef __aarch64__
#define DDR_PATTERN	0xAAAAAAAAAAAAAAAAUL
#define DDR_ANTIPATTERN	0x5555555555555555UL
#else /* !__aarch64__ */
#define DDR_PATTERN	0xAAAAAAAAU
#define DDR_ANTIPATTERN	0x55555555U
#endif /* __aarch64__ */

static void mmio_write_pattern(uintptr_t addr, u_register_t value)
{
#ifdef __aarch64__
	mmio_write_64(addr, (uint64_t)value);
#else /* !__aarch64__ */
	mmio_write_32(addr, (uint32_t)value);
#endif /* __aarch64__ */
}

static u_register_t mmio_read_pattern(uintptr_t addr)
{
#ifdef __aarch64__
	return (u_register_t)mmio_read_64(addr);
#else /* !__aarch64__ */
	return (u_register_t)mmio_read_32(addr);
#endif /* __aarch64__ */
}

/*******************************************************************************
 * This function tests a simple read/write access to the DDR.
 * Note that the previous content is restored after test.
 * Returns 0 if success, and address value else.
 ******************************************************************************/
uintptr_t stm32mp_ddr_test_rw_access(void)
{
	u_register_t saved_value = mmio_read_pattern(STM32MP_DDR_BASE);

	mmio_write_pattern(STM32MP_DDR_BASE, DDR_PATTERN);

	if (mmio_read_pattern(STM32MP_DDR_BASE) != DDR_PATTERN) {
		return STM32MP_DDR_BASE;
	}

	mmio_write_pattern(STM32MP_DDR_BASE, saved_value);

	return 0UL;
}

/*******************************************************************************
 * This function tests the DDR data bus wiring.
 * This is inspired from the Data Bus Test algorithm written by Michael Barr
 * in "Programming Embedded Systems in C and C++" book.
 * resources.oreilly.com/examples/9781565923546/blob/master/Chapter6/
 * File: memtest.c - This source code belongs to Public Domain.
 * Returns 0 if success, and address value else.
 ******************************************************************************/
uintptr_t stm32mp_ddr_test_data_bus(void)
{
	u_register_t pattern;

	for (pattern = 1U; pattern != 0U; pattern <<= 1U) {
		mmio_write_pattern(STM32MP_DDR_BASE, pattern);

		if (mmio_read_pattern(STM32MP_DDR_BASE) != pattern) {
			return STM32MP_DDR_BASE;
		}
	}

	return 0UL;
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
uintptr_t stm32mp_ddr_test_addr_bus(size_t size)
{
	size_t addressmask = size - 1U;
	size_t offset;
	size_t testoffset = 0U;

	/* Write the default pattern at each of the power-of-two offsets. */
	for (offset = sizeof(u_register_t); (offset & addressmask) != 0U;
	     offset <<= 1U) {
		mmio_write_pattern(STM32MP_DDR_BASE + offset, DDR_PATTERN);
	}

	/* Check for address bits stuck high. */
	mmio_write_pattern(STM32MP_DDR_BASE + testoffset, DDR_ANTIPATTERN);

	for (offset = sizeof(u_register_t); (offset & addressmask) != 0U;
	     offset <<= 1U) {
		if (mmio_read_pattern(STM32MP_DDR_BASE + offset) != DDR_PATTERN) {
			return STM32MP_DDR_BASE + offset;
		}
	}

	mmio_write_pattern(STM32MP_DDR_BASE + testoffset, DDR_PATTERN);

	/* Check for address bits stuck low or shorted. */
	for (testoffset = sizeof(u_register_t); (testoffset & addressmask) != 0U;
	     testoffset <<= 1U) {
		mmio_write_pattern(STM32MP_DDR_BASE + testoffset, DDR_ANTIPATTERN);

		if (mmio_read_pattern(STM32MP_DDR_BASE) != DDR_PATTERN) {
			return STM32MP_DDR_BASE;
		}

		for (offset = sizeof(u_register_t); (offset & addressmask) != 0U;
		     offset <<= 1U) {
			if ((mmio_read_pattern(STM32MP_DDR_BASE + offset) != DDR_PATTERN) &&
			    (offset != testoffset)) {
				return STM32MP_DDR_BASE + offset;
			}
		}

		mmio_write_pattern(STM32MP_DDR_BASE + testoffset, DDR_PATTERN);
	}

	return 0UL;
}

/*******************************************************************************
 * This function checks the DDR size. It has to be run with Data Cache off.
 * This test is run before data have been put in DDR, and is only done for
 * cold boot. The DDR data can then be overwritten, and it is not useful to
 * restore its content.
 * Returns DDR computed size.
 ******************************************************************************/
size_t stm32mp_ddr_check_size(void)
{
	size_t offset = sizeof(u_register_t);

	mmio_write_pattern(STM32MP_DDR_BASE, DDR_PATTERN);

	while (offset < STM32MP_DDR_MAX_SIZE) {
		mmio_write_pattern(STM32MP_DDR_BASE + offset, DDR_ANTIPATTERN);
		dsb();

		if (mmio_read_pattern(STM32MP_DDR_BASE) != DDR_PATTERN) {
			break;
		}

		offset <<= 1U;
	}

	return offset;
}
