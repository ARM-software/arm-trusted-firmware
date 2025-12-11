/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/delay_timer.h>
#include <drivers/gpio.h>
#include <drivers/tpm/tpm2_slb9670/slb9670_gpio.h>

/*
 * Infineon SLB9670 Chip Reset Parameters
 */
#define t_WRST	2 /* Warm Reset Time (us) */
#define t_RSTIN	60 /* Reset Inactive Time (ms) */

/*
 * RPi3 GPIO pin configuration for TPM via bit-bang SPI
 * References: https://pinout.xyz/pinout/spi
 * - docs/design_documents/measured_boot_dtpm_poc.rst
 */
const struct gpio_spi_config tpm_rpi3_gpio_data = { .cs_gpio = 7,
						    .sclk_gpio = 11,
						    .mosi_gpio = 10,
						    .miso_gpio = 9,
						    .reset_gpio = 24,
						    .spi_mode = 0,
						    .spi_max_clock = 1000000 };

/*
 * When RST is asserted at certain points in time, then this
 * triggers the TPM's security functions, in the case where
 * multiple resets need to be asserted, there must be a wait
 * of at least t_RSTIN between the resets
 *
 * In most cases this is not needed since RST is only being asserted
 * once, ie for TPM initialization at the beginning of TFA.
 */
void tpm2_slb9670_reset_chip(const struct gpio_spi_config *tpm_gpio_data)
{
	/*
	 * since we don't know the value of the pin before it was init to 1
	 * it is best to assume the state was 0, and account for that by
	 * adding an initial RST inactive delay
	 */
	mdelay(t_RSTIN);
	/* pull #RST pin to active low for 2us */
	gpio_set_value(tpm_gpio_data->reset_gpio, 0);
	udelay(t_WRST);
	/* wait 60ms after warm reset before sending TPM commands */
	gpio_set_value(tpm_gpio_data->reset_gpio, 1);
	mdelay(t_RSTIN);
}

/*
 * init GPIO pins for the Infineon slb9670 TPM
 */
void tpm2_slb9670_gpio_init(const struct gpio_spi_config *tpm_gpio_data)
{
	gpio_set_value(tpm_gpio_data->reset_gpio, 1);
	gpio_set_direction(tpm_gpio_data->reset_gpio, GPIO_DIR_OUT);
}

const struct gpio_spi_config *tpm2_slb9670_get_config(void)
{
	return &tpm_rpi3_gpio_data;
}
