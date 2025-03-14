/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#include <common/desc_image_load.h>
#include <common/ep_info.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/gpio_spi.h>
#include <drivers/measured_boot/event_log/event_log.h>
#include <drivers/measured_boot/metadata.h>
#include <drivers/tpm/tpm2.h>
#include <drivers/tpm/tpm2_chip.h>
#include <drivers/tpm/tpm2_slb9670/slb9670_gpio.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

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
extern struct gpio_spi_data tpm_rpi3_gpio_data;
struct spi_plat *spidev;
#endif

static void rpi3_bl1_tpm_early_interface_setup(void)
{
#if (TPM_INTERFACE == FIFO_SPI)
	tpm2_slb9670_gpio_init(&tpm_rpi3_gpio_data);

	tpm2_slb9670_reset_chip(&tpm_rpi3_gpio_data);

	spidev = gpio_spi_init(&tpm_rpi3_gpio_data);
#endif
}
#endif

void bl1_plat_mboot_init(void)
{
#if DISCRETE_TPM
	int rc;

	rpi3_bl1_tpm_early_interface_setup();
	rc = tpm_interface_init(&tpm_chip_data, 0);
	if (rc != 0) {
		ERROR("BL1: TPM interface init failed\n");
		panic();
	}
	rc = tpm_startup(&tpm_chip_data, TPM_SU_CLEAR);
	if (rc != 0) {
		ERROR("BL1: TPM Startup failed\n");
		panic();
	}
#endif

	event_log_init(event_log, event_log + sizeof(event_log));
	event_log_write_header();
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
}

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	int rc = 0;
	unsigned char hash_data[CRYPTO_MD_MAX_SIZE];
	const event_log_metadata_t *metadata_ptr = rpi3_event_log_metadata;

	rc = event_log_measure(image_data->image_base, image_data->image_size, hash_data);
	if (rc != 0) {
		return rc;
	}

#if DISCRETE_TPM
	rc = tpm_pcr_extend(&tpm_chip_data, 0, TPM_ALG_ID, hash_data, TCG_DIGEST_SIZE);
	if (rc != 0) {
		ERROR("BL1: TPM PCR-0 extend failed\n");
		panic();
	}
#endif

	while ((metadata_ptr->id != EVLOG_INVALID_ID) &&
		(metadata_ptr->id != image_id)) {
		metadata_ptr++;
	}
	assert(metadata_ptr->id != EVLOG_INVALID_ID);

	event_log_record(hash_data, EV_POST_CODE, metadata_ptr);

	/* Dump Event Log for user view */
	event_log_dump((uint8_t *)event_log, event_log_get_cur_size(event_log));

	return rc;
}
