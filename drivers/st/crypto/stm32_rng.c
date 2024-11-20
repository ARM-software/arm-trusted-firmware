/*
 * Copyright (c) 2022-2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <common/fdt_wrappers.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32_rng.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/mmio.h>
#include <libfdt.h>

#include <platform_def.h>

#if STM32_RNG_VER == 2
#define DT_RNG_COMPAT		"st,stm32-rng"
#endif
#if STM32_RNG_VER == 4
#define DT_RNG_COMPAT		"st,stm32mp13-rng"
#endif
#define RNG_CR			0x00U
#define RNG_SR			0x04U
#define RNG_DR			0x08U
#if STM32_RNG_VER == 4
#define RNG_HTCR		0x10U
#endif

#define RNG_CR_RNGEN		BIT(2)
#define RNG_CR_IE		BIT(3)
#define RNG_CR_CED		BIT(5)
#define RNG_CR_CLKDIV		GENMASK(19, 16)
#define RNG_CR_CLKDIV_SHIFT	16U
#define RNG_CR_CONDRST		BIT(30)

#define RNG_SR_DRDY		BIT(0)
#define RNG_SR_CECS		BIT(1)
#define RNG_SR_SECS		BIT(2)
#define RNG_SR_CEIS		BIT(5)
#define RNG_SR_SEIS		BIT(6)

#define RNG_TIMEOUT_US		100000U
#define RNG_TIMEOUT_STEP_US	10U

#define TIMEOUT_US_1MS		1000U

#define RNG_NIST_CONFIG_A	0x00F40F00U
#define RNG_NIST_CONFIG_B	0x01801000U
#define RNG_NIST_CONFIG_C	0x00F00D00U
#define RNG_NIST_CONFIG_MASK	GENMASK(25, 8)

#if STM32_RNG_VER == 4
#if STM32_RNG_VER_MINOR == 2
#define RNG_HTCFG_CONFIG	0x000072ACU /* Reset value */
#else
#define RNG_HTCFG_CONFIG	0x0000AAC7U
#endif
#endif

#define RNG_MAX_NOISE_CLK_FREQ	48000000U

struct stm32_rng_instance {
	uintptr_t base;
	unsigned long clock;
};

static struct stm32_rng_instance stm32_rng;

static void seed_error_recovery(void)
{
	uint8_t i __maybe_unused;

	/* Recommended by the SoC reference manual */
	mmio_clrbits_32(stm32_rng.base + RNG_SR, RNG_SR_SEIS);
	dmbsy();

#if STM32_RNG_VER == 2
	/* No Auto-reset on version 2, need to clean FIFO */
	for (i = 12U; i != 0U; i--) {
		(void)mmio_read_32(stm32_rng.base + RNG_DR);
	}

	dmbsy();
#endif

	if ((mmio_read_32(stm32_rng.base + RNG_SR) & RNG_SR_SEIS) != 0U) {
		ERROR("RNG noise\n");
		panic();
	}
}

static uint32_t stm32_rng_clock_freq_restrain(void)
{
	unsigned long clock_rate;
	uint32_t clock_div = 0U;

	clock_rate = clk_get_rate(stm32_rng.clock);

	/*
	 * Get the exponent to apply on the CLKDIV field in RNG_CR register
	 * No need to handle the case when clock-div > 0xF as it is physically
	 * impossible
	 */
	while ((clock_rate >> clock_div) > RNG_MAX_NOISE_CLK_FREQ) {
		clock_div++;
	}

	VERBOSE("RNG clk rate : %lu\n", clk_get_rate(stm32_rng.clock) >> clock_div);

	return clock_div;
}

static int stm32_rng_enable(void)
{
	uint32_t sr;
	uint64_t timeout;
	uint32_t clock_div __maybe_unused;

#if STM32_RNG_VER == 2
	mmio_write_32(stm32_rng.base + RNG_CR, RNG_CR_RNGEN | RNG_CR_CED);
#endif
#if STM32_RNG_VER == 4
	/* Reset internal block and disable CED bit */
	clock_div = stm32_rng_clock_freq_restrain();

	/* Update configuration fields */
	mmio_clrsetbits_32(stm32_rng.base + RNG_CR, RNG_NIST_CONFIG_MASK,
			   RNG_NIST_CONFIG_A | RNG_CR_CONDRST | RNG_CR_CED);

	mmio_clrsetbits_32(stm32_rng.base + RNG_CR, RNG_CR_CLKDIV,
			   (clock_div << RNG_CR_CLKDIV_SHIFT));

	mmio_write_32(stm32_rng.base + RNG_HTCR, RNG_HTCFG_CONFIG);

	mmio_clrsetbits_32(stm32_rng.base + RNG_CR, RNG_CR_CONDRST, RNG_CR_RNGEN);
#endif
	timeout = timeout_init_us(RNG_TIMEOUT_US);
	sr = mmio_read_32(stm32_rng.base + RNG_SR);
	while ((sr & RNG_SR_DRDY) == 0U) {
		if (timeout_elapsed(timeout)) {
			WARN("Timeout waiting\n");
			return -ETIMEDOUT;
		}

		if ((sr & (RNG_SR_SECS | RNG_SR_SEIS)) != 0U) {
			seed_error_recovery();
			timeout = timeout_init_us(RNG_TIMEOUT_US);
		}

		udelay(RNG_TIMEOUT_STEP_US);
		sr = mmio_read_32(stm32_rng.base + RNG_SR);
	}

	VERBOSE("Init RNG done\n");

	return 0;
}

static int check_data_validity(void)
{
	int nb_tries = RNG_TIMEOUT_US / RNG_TIMEOUT_STEP_US;
	uint32_t status = mmio_read_32(stm32_rng.base + RNG_SR);

	/* Exit if data is ready without any seed error */
	if ((status & (RNG_SR_SECS | RNG_SR_SEIS | RNG_SR_DRDY)) != RNG_SR_DRDY) {
		do {

			if ((status & (RNG_SR_SECS | RNG_SR_SEIS)) != 0U) {
				seed_error_recovery();
			}

			udelay(RNG_TIMEOUT_STEP_US);
			nb_tries--;
			if (nb_tries == 0) {
				return -ETIMEDOUT;
			}

			status = mmio_read_32(stm32_rng.base + RNG_SR);
		} while ((status & RNG_SR_DRDY) == 0U);
	}

	return 0;
}

/*
 * stm32_rng_read - Read a number of random bytes from RNG
 * out: pointer to the output buffer
 * size: number of bytes to be read
 * Return 0 on success, non-0 on failure
 */
int stm32_rng_read(uint8_t *out, uint32_t size)
{
	uint8_t *buf = out;
	size_t len = size;
	uint32_t data32;
	int rc = 0;
	unsigned int count;

	if (stm32_rng.base == 0U) {
		return -EPERM;
	}

	while (len != 0U) {
		rc = check_data_validity();
		if (rc != 0) {
			goto bail;
		}

		count = 4U;
		while (len != 0U) {
			if ((mmio_read_32(stm32_rng.base + RNG_SR) & RNG_SR_DRDY) == 0U) {
				break;
			}

			data32 = mmio_read_32(stm32_rng.base + RNG_DR);

			while (data32 == 0U) {
				rc = check_data_validity();
				if (rc != 0) {
					goto bail;
				}

				data32 = mmio_read_32(stm32_rng.base + RNG_DR);
			}

			count--;

			(void)memcpy(buf, (uint8_t *)&data32, MIN(len, sizeof(uint32_t)));
			buf += MIN(len, sizeof(uint32_t));
			len -= MIN(len, sizeof(uint32_t));

			if (count == 0U) {
				break;
			}
		}
	}

bail:
	if (rc != 0) {
		(void)memset(out, 0, buf - out);
	}

	return rc;
}

/*
 * stm32_rng_select: Select a specified RNG instance from its base address.
 * This function only works if the driver is uninitialized.
 */
void stm32_rng_select(uintptr_t rng_base)
{
	if ((stm32_rng.base == 0U) || (stm32_rng.clock == 0U)) {
		/* RNG instance is selected once */
		stm32_rng.base = rng_base;
	}
}

/*
 * stm32_rng_init: Initialize rng from DT
 * return 0 on success, negative value on failure
 */
int stm32_rng_init(void)
{
	struct stm32_rng_instance rng = {0, 0};
	void *fdt;
	int node;
	int success = 0;
	int disabled = 0;

	if (stm32_rng.base != 0U) {
		if (stm32_rng.clock != 0U) {
			/* Driver is already initialized */
			return 0;
		}

		rng.base = stm32_rng.base;
	}

	if (fdt_get_address(&fdt) == 0) {
		panic();
	}

	fdt_for_each_compatible_node(fdt, node, DT_RNG_COMPAT) {
		struct dt_node_info dt_rng;
		int ret;

		dt_fill_device_info(&dt_rng, node);

		VERBOSE("Setting up rng@%x, status: %x\n", dt_rng.base, dt_rng.status);

		/*
		 * All the valid RNG peripherals available in device tree are
		 * activated but only the last RNG found is assigned to the RNG
		 * instance of the driver.
		 */
		if ((dt_rng.status == DT_DISABLED) || (dt_rng.base == 0U)) {
			disabled++;
			continue;
		}

		stm32_rng.base = dt_rng.base;

		if (dt_rng.clock < 0) {
			panic();
		}

		stm32_rng.clock = (unsigned long)dt_rng.clock;
		clk_enable(stm32_rng.clock);

		if (stm32_rng.base == rng.base) {
			/* Set the rng instance as the rng to use by TF-A */
			rng.clock = stm32_rng.clock;
		}

		if (dt_rng.reset >= 0) {

			ret = stm32mp_reset_assert((unsigned long)dt_rng.reset, TIMEOUT_US_1MS);
			if (ret != 0) {
				panic();
			}

			udelay(20);

			ret = stm32mp_reset_deassert((unsigned long)dt_rng.reset, TIMEOUT_US_1MS);
			if (ret != 0) {
				panic();
			}
		}

		ret = stm32_rng_enable();
		if (ret != 0) {
			ERROR("Failed to enable rng@%x\n", dt_rng.base);
		} else {
			success++;
		}
	}

	if ((success == 0) && (disabled > 0)) {
		WARN("%s: No RNG found in device tree.\n", __func__);
		return 0;
	} else if (success > 0) {
		if ((rng.clock != 0U) && (rng.base != 0U)) {
			stm32_rng = rng;
		}
		return 0;
	} else {
		return -ENODEV;
	}
}
