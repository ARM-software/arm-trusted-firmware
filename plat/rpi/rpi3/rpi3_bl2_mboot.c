/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#include "./include/rpi3_measured_boot.h"

#include <drivers/auth/crypto_mod.h>
#include <drivers/gpio_spi.h>
#include <drivers/measured_boot/event_log/event_log.h>
#include <drivers/measured_boot/metadata.h>
#include <drivers/tpm/tpm2.h>
#include <drivers/tpm/tpm2_chip.h>
#include <drivers/tpm/tpm2_slb9670/slb9670_gpio.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <tools_share/tbbr_oid.h>

/* RPI3 table with platform specific image IDs, names and PCRs */
const event_log_metadata_t rpi3_event_log_metadata[] = {
	{ BL31_IMAGE_ID, MBOOT_BL31_IMAGE_STRING, PCR_0 },
	{ BL33_IMAGE_ID, MBOOT_BL33_IMAGE_STRING, PCR_0 },
	{ NT_FW_CONFIG_ID, MBOOT_NT_FW_CONFIG_STRING, PCR_0 },

	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

#if DISCRETE_TPM
extern struct tpm_chip_data tpm_chip_data;
#if (TPM_INTERFACE == FIFO_SPI)
extern struct gpio_spi_data tpm_rpi3_gpio_data;
struct spi_plat *spidev;
#endif

static void rpi3_bl2_tpm_early_interface_setup(void)
{
#if (TPM_INTERFACE == FIFO_SPI)
	tpm2_slb9670_gpio_init(&tpm_rpi3_gpio_data);

	spidev = gpio_spi_init(&tpm_rpi3_gpio_data);
#endif
}
#endif

static uint8_t *event_log_start;
static size_t event_log_size;

void bl2_plat_mboot_init(void)
{
	uint8_t *bl2_event_log_start;
	uint8_t *bl2_event_log_finish;

#if DISCRETE_TPM
	int rc;

	rpi3_bl2_tpm_early_interface_setup();
	rc = tpm_interface_init(&tpm_chip_data, 0);
	if (rc != 0) {
		ERROR("BL2: TPM interface init failed\n");
		panic();
	}
#endif

	rpi3_mboot_fetch_eventlog_info(&event_log_start, &event_log_size);
	bl2_event_log_start = event_log_start + event_log_size;
	bl2_event_log_finish = event_log_start + PLAT_ARM_EVENT_LOG_MAX_SIZE;
	event_log_init(bl2_event_log_start, bl2_event_log_finish);
}

void bl2_plat_mboot_finish(void)
{
	int rc;

	/* Event Log address in Non-Secure memory */
	uintptr_t ns_log_addr;

	/* Event Log filled size */
	size_t event_log_cur_size;

	event_log_cur_size = event_log_get_cur_size((uint8_t *)event_log_start);

	/* write the eventlog addr and size to NT_FW_CONFIG TPM entry */
	rc = rpi3_set_nt_fw_info(event_log_cur_size, &ns_log_addr);
	if (rc != 0) {
		ERROR("%s(): Unable to update %s_FW_CONFIG\n",
			__func__, "NT");
		/*
		 * fatal error due to Bl33 maintaining the assumption
		 * that the eventlog is successfully passed via
		 * NT_FW_CONFIG.
		 */
		panic();
	}

	/* Copy Event Log to Non-secure memory */
	(void)memcpy((void *)ns_log_addr, (const void *)event_log_start,
		     event_log_cur_size);

	/* Ensure that the Event Log is visible in Non-secure memory */
	flush_dcache_range(ns_log_addr, event_log_cur_size);

	/* Dump Event Log for user view */
	event_log_dump((uint8_t *)event_log_start, event_log_cur_size);

#if DISCRETE_TPM
	/* relinquish control of TPM locality 0 and close interface */
	rc = tpm_interface_close(&tpm_chip_data, 0);
	if (rc != 0) {
		ERROR("BL2: TPM interface close failed\n");
		panic();
	}
#endif
}

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	int rc = 0;

	unsigned char hash_data[CRYPTO_MD_MAX_SIZE];
	const event_log_metadata_t *metadata_ptr = rpi3_event_log_metadata;

	/* Measure the payload with algorithm selected by EventLog driver */
	rc = event_log_measure(image_data->image_base, image_data->image_size, hash_data);
	if (rc != 0) {
		return rc;
	}

#if DISCRETE_TPM
	rc = tpm_pcr_extend(&tpm_chip_data, 0, TPM_ALG_ID, hash_data, TCG_DIGEST_SIZE);
	if (rc != 0) {
		ERROR("BL2: TPM PCR-0 extend failed\n");
		panic();
	}
#endif

	while ((metadata_ptr->id != EVLOG_INVALID_ID) &&
		(metadata_ptr->id != image_id)) {
		metadata_ptr++;
	}
	assert(metadata_ptr->id != EVLOG_INVALID_ID);

	event_log_record(hash_data, EV_POST_CODE, metadata_ptr);

	return rc;
}
