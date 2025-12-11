/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <tpm2.h>
#include <tpm2_chip.h>

#include <common/desc_image_load.h>
#include <common/ep_info.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/delay_timer.h>
#include <drivers/gpio_spi.h>
#include <drivers/measured_boot/metadata.h>
#include <drivers/tpm/tpm2_slb9670/slb9670_gpio.h>
#include <event_measure.h>
#include <event_print.h>
#include <rpi_shared.h>

/* Event Log data */
uint8_t event_log[PLAT_ARM_EVENT_LOG_MAX_SIZE];

/* RPI3 table with platform specific image IDs, names and PCRs */
const event_log_metadata_t rpi3_event_log_metadata[] = {
	{ FW_CONFIG_ID, MBOOT_FW_CONFIG_STRING, PCR_0 },
	{ TB_FW_CONFIG_ID, MBOOT_TB_FW_CONFIG_STRING, PCR_0 },
	{ BL2_IMAGE_ID, MBOOT_BL2_IMAGE_STRING, PCR_0 },

	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

#if DISCRETE_TPM
extern struct tpm_chip_data tpm_chip_data;
#if (TPM_INTERFACE == FIFO_SPI)

#endif

static void rpi3_bl1_tpm_early_interface_setup(void)
{
#if TPM_INTERFACE_FIFO_SPI
	int rc;
	struct spi_plat *spidev;
	const struct tpm_timeout_ops timeout_ops = {
		.timeout_init_us = timeout_init_us,
		.timeout_elapsed = timeout_elapsed
	};
	const struct gpio_spi_config *tpm_rpi3_gpio_data =
		tpm2_slb9670_get_config();

	tpm2_slb9670_gpio_init(tpm_rpi3_gpio_data);

	tpm2_slb9670_reset_chip(tpm_rpi3_gpio_data);

	spidev = gpio_spi_init(tpm_rpi3_gpio_data);

	rc = tpm_interface_init(spidev, &timeout_ops, &tpm_chip_data, 0);
	if (rc != 0) {
		ERROR("BL1: TPM interface init failed\n");
		panic();
	}

#endif
}
#endif

void bl1_plat_mboot_init(void)
{
	tpm_alg_id algorithms[] = {
#ifdef TPM_ALG_ID
		TPM_ALG_ID
#else
		/*
		 * TODO: with MEASURED_BOOT=1 several algorithms now compiled into Mbed-TLS,
		 * we ought to query the backend to figure out what algorithms to use.
		 */
		TPM_ALG_SHA256,
		TPM_ALG_SHA384,
		TPM_ALG_SHA512,
#endif
	};
	int rc;

#if DISCRETE_TPM

	rpi3_bl1_tpm_early_interface_setup();
	rc = tpm_startup(&tpm_chip_data, TPM_SU_CLEAR);
	if (rc != 0) {
		ERROR("BL1: TPM Startup failed\n");
		panic();
	}
#endif

	rc = event_log_init_and_reg(event_log, event_log + sizeof(event_log),
				    0U, crypto_mod_tcg_hash);
	if (rc < 0) {
		ERROR("Failed to initialize event log (%d).\n", rc);
		panic();
	}

	rc = event_log_write_header(algorithms, ARRAY_SIZE(algorithms), 0, NULL,
				    0);
	if (rc < 0) {
		ERROR("Failed to write event log header (%d).\n", rc);
		panic();
	}
}

void bl1_plat_mboot_finish(void)
{
	size_t event_log_cur_size;
	image_desc_t *image_desc;
	entry_point_info_t *ep_info;

	event_log_cur_size = event_log_get_cur_size(event_log);
	image_desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(image_desc != NULL);

	/* Get the entry point info */
	ep_info = &image_desc->ep_info;
	ep_info->args.arg2 = (uint64_t) event_log;
	ep_info->args.arg3 = (uint32_t) event_log_cur_size;

#if DISCRETE_TPM
	int rc;

	/* relinquish control of TPM locality 0 and close interface */
	rc = tpm_interface_close(&tpm_chip_data, 0);
	if (rc != 0) {
		ERROR("BL1: TPM interface close failed\n");
		panic();
	}
#endif

	/* Dump Event Log for user view */
	event_log_dump((uint8_t *)event_log, event_log_get_cur_size(event_log));
}
