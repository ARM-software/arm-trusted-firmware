/*
 * Copyright (c) 2021, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch.h>
#include <arch_helpers.h>
#include <drivers/arm/gic600ae_fmu.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#define GICFMU_IDLE_TIMEOUT_US		U(2000000)

/* Macro to write 32-bit FMU registers */
#define GIC_FMU_WRITE_32(base, reg, val) \
	do { \
		/* \
		 * This register receives the unlock key that is required for \
		 * writes to FMU registers to be successful. \
		 */ \
		mmio_write_32(base + GICFMU_KEY, 0xBE); \
		/* Perform the actual write */ \
		mmio_write_32((base) + (reg), (val)); \
	} while (false)

/* Macro to write 64-bit FMU registers */
#define GIC_FMU_WRITE_64(base, reg, n, val) \
	do { \
		/* \
		 * This register receives the unlock key that is required for \
		 * writes to FMU registers to be successful. \
		 */ \
		mmio_write_32(base + GICFMU_KEY, 0xBE); \
		/* \
		 * APB bus is 32-bit wide; so split the 64-bit write into \
		 * two 32-bit writes \
		 */ \
		mmio_write_32((base) + reg##_LO + (n * 64), (val)); \
		mmio_write_32((base) + reg##_HI + (n * 64), (val)); \
	} while (false)

/* Helper function to wait until FMU is ready to accept the next command */
static void wait_until_fmu_is_idle(uintptr_t base)
{
	uint32_t timeout_count = GICFMU_IDLE_TIMEOUT_US;
	uint64_t status;

	/* wait until status is 'busy' */
	do {
		status = (gic_fmu_read_status(base) & BIT(0));

		if (timeout_count-- == 0U) {
			ERROR("GIC600 AE FMU is not responding\n");
			panic();
		}

		udelay(1U);

	} while (status == U(0));
}

#define GIC_FMU_WRITE_ON_IDLE_32(base, reg, val) \
	do { \
		/* Wait until FMU is ready */ \
		wait_until_fmu_is_idle(base); \
		/* Actual register write */ \
		GIC_FMU_WRITE_32(base, reg, val); \
		/* Wait until FMU is ready */ \
		wait_until_fmu_is_idle(base); \
	} while (false)

#define GIC_FMU_WRITE_ON_IDLE_64(base, reg, n, val) \
	do { \
		/* Wait until FMU is ready */ \
		wait_until_fmu_is_idle(base); \
		/* Actual register write */ \
		GIC_FMU_WRITE_64(base, reg, n, val); \
		/* Wait until FMU is ready */ \
		wait_until_fmu_is_idle(base); \
	} while (false)

/*******************************************************************************
 * GIC FMU functions for accessing the Fault Management Unit registers
 ******************************************************************************/

/*
 * Accessors to read the Error Record Feature Register bits corresponding
 * to an error record 'n'
 */
uint64_t gic_fmu_read_errfr(uintptr_t base, unsigned int n)
{
	/*
	 * APB bus is 32-bit wide; so split the 64-bit read into
	 * two 32-bit reads
	 */
	uint64_t reg_val = (uint64_t)mmio_read_32(base + GICFMU_ERRFR_LO + n * 64U);

	reg_val |= ((uint64_t)mmio_read_32(base + GICFMU_ERRFR_HI + n * 64U) << 32);
	return reg_val;
}

/*
 * Accessors to read the Error Record Control Register bits corresponding
 * to an error record 'n'
 */
uint64_t gic_fmu_read_errctlr(uintptr_t base, unsigned int n)
{
	/*
	 * APB bus is 32-bit wide; so split the 64-bit read into
	 * two 32-bit reads
	 */
	uint64_t reg_val = (uint64_t)mmio_read_32(base + GICFMU_ERRCTLR_LO + n * 64U);

	reg_val |= ((uint64_t)mmio_read_32(base + GICFMU_ERRCTLR_HI + n * 64U) << 32);
	return reg_val;
}

/*
 * Accessors to read the Error Record Primary Status Register bits
 * corresponding to an error record 'n'
 */
uint64_t gic_fmu_read_errstatus(uintptr_t base, unsigned int n)
{
	/*
	 * APB bus is 32-bit wide; so split the 64-bit read into
	 * two 32-bit reads
	 */
	uint64_t reg_val = (uint64_t)mmio_read_32(base + GICFMU_ERRSTATUS_LO + n * 64U);

	reg_val |= ((uint64_t)mmio_read_32(base + GICFMU_ERRSTATUS_HI + n * 64U) << 32);
	return reg_val;
}

/*
 * Accessors to read the Error Group Status Register
 */
uint64_t gic_fmu_read_errgsr(uintptr_t base)
{
	/*
	 * APB bus is 32-bit wide; so split the 64-bit read into
	 * two 32-bit reads
	 */
	uint64_t reg_val = (uint64_t)mmio_read_32(base + GICFMU_ERRGSR_LO);

	reg_val |= ((uint64_t)mmio_read_32(base + GICFMU_ERRGSR_HI) << 32);
	return reg_val;
}

/*
 * Accessors to read the Ping Control Register
 */
uint32_t gic_fmu_read_pingctlr(uintptr_t base)
{
	return mmio_read_32(base + GICFMU_PINGCTLR);
}

/*
 * Accessors to read the Ping Now Register
 */
uint32_t gic_fmu_read_pingnow(uintptr_t base)
{
	return mmio_read_32(base + GICFMU_PINGNOW);
}

/*
 * Accessors to read the Ping Mask Register
 */
uint64_t gic_fmu_read_pingmask(uintptr_t base)
{
	/*
	 * APB bus is 32-bit wide; so split the 64-bit read into
	 * two 32-bit reads
	 */
	uint64_t reg_val = (uint64_t)mmio_read_32(base + GICFMU_PINGMASK_LO);

	reg_val |= ((uint64_t)mmio_read_32(base + GICFMU_PINGMASK_HI) << 32);
	return reg_val;
}

/*
 * Accessors to read the FMU Status Register
 */
uint32_t gic_fmu_read_status(uintptr_t base)
{
	return mmio_read_32(base + GICFMU_STATUS);
}

/*
 * Accessors to read the Error Record ID Register
 */
uint32_t gic_fmu_read_erridr(uintptr_t base)
{
	return mmio_read_32(base + GICFMU_ERRIDR);
}

/*
 * Accessors to write a 64 bit value to the Error Record Control Register
 */
void gic_fmu_write_errctlr(uintptr_t base, unsigned int n, uint64_t val)
{
	GIC_FMU_WRITE_64(base, GICFMU_ERRCTLR, n, val);
}

/*
 * Accessors to write a 64 bit value to the Error Record Primary Status
 * Register
 */
void gic_fmu_write_errstatus(uintptr_t base, unsigned int n, uint64_t val)
{
	/* Wait until FMU is ready before writing */
	GIC_FMU_WRITE_ON_IDLE_64(base, GICFMU_ERRSTATUS, n, val);
}

/*
 * Accessors to write a 32 bit value to the Ping Control Register
 */
void gic_fmu_write_pingctlr(uintptr_t base, uint32_t val)
{
	GIC_FMU_WRITE_32(base, GICFMU_PINGCTLR, val);
}

/*
 * Accessors to write a 32 bit value to the Ping Now Register
 */
void gic_fmu_write_pingnow(uintptr_t base, uint32_t val)
{
	/* Wait until FMU is ready before writing */
	GIC_FMU_WRITE_ON_IDLE_32(base, GICFMU_PINGNOW, val);
}

/*
 * Accessors to write a 32 bit value to the Safety Mechanism Enable Register
 */
void gic_fmu_write_smen(uintptr_t base, uint32_t val)
{
	/* Wait until FMU is ready before writing */
	GIC_FMU_WRITE_ON_IDLE_32(base, GICFMU_SMEN, val);
}

/*
 * Accessors to write a 32 bit value to the Safety Mechanism Inject Error
 * Register
 */
void gic_fmu_write_sminjerr(uintptr_t base, uint32_t val)
{
	/* Wait until FMU is ready before writing */
	GIC_FMU_WRITE_ON_IDLE_32(base, GICFMU_SMINJERR, val);
}

/*
 * Accessors to write a 64 bit value to the Ping Mask Register
 */
void gic_fmu_write_pingmask(uintptr_t base, uint64_t val)
{
	GIC_FMU_WRITE_64(base, GICFMU_PINGMASK, 0, val);
}
