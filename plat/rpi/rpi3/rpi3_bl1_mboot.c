/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#if DISCRETE_TPM
#include <tpm2.h>
#include <tpm2_chip.h>
#endif

#include <common/desc_image_load.h>
#include <common/ep_info.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/delay_timer.h>
#include <drivers/gpio_spi.h>
#include <drivers/measured_boot/metadata.h>
#include <drivers/tpm/tpm2_slb9670/slb9670_gpio.h>
#include <event_measure.h>
#include <event_print.h>
#if TRANSFER_LIST
#include <tpm_event_log.h>
#include <transfer_list.h>
#endif
#include <rpi_shared.h>

/* Event Log data */
#if TRANSFER_LIST
static uint8_t *event_log;
#else
uint8_t event_log[PLAT_ARM_EVENT_LOG_MAX_SIZE];
#endif

/* RPI3 table with platform specific image IDs, names and PCRs */
const event_log_metadata_t rpi3_event_log_metadata[] = {
	{ FW_CONFIG_ID, MBOOT_FW_CONFIG_STRING, PCR_0 },
	{ TB_FW_CONFIG_ID, MBOOT_TB_FW_CONFIG_STRING, PCR_0 },
	{ BL2_IMAGE_ID, MBOOT_BL2_IMAGE_STRING, PCR_0 },

	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

#if DISCRETE_TPM
extern struct tpm_chip_data tpm_chip_data;

/* RPI3 measured boot metadata currently extends only PCR 0. */
#define RPI3_REQUIRED_TPM_PCR_MASK	(U(1) << PCR_0)

static void rpi3_bl1_tpm_early_interface_setup(void)
{
#if TPM_INTERFACE_FIFO_SPI
	int rc;
	struct tpm_spi_plat *spidev;
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

#if DISCRETE_TPM
static bool pcr_cb(uint16_t hash_alg, const uint8_t *pcr_select,
		   uint8_t sizeof_select, tpm_pcr_bank_ctx_t *ctx)
{
	uint32_t select = 0;
	for (int i = 0; i < sizeof_select; i++) {
		select = (select << 8) + pcr_select[i];
	}
	INFO("PCR bank for alg=0x%04x: 0x%08x\n", hash_alg, select);
	if (hash_alg == TPM_ALG_ID) {
		ctx->flags = select;
	}
	return 0;
}

#if RPI3_PROVISION_TPM
#if !DEBUG
#error DEBUG flag must be set to enable RPI3_PROVISION_TPM
#endif

static int allocate_pcr_bank(uint16_t hash_alg)
{
	size_t i;
	int rc;
	bool success = false;
	uint32_t max_pcr = 0;
	uint32_t size_needed = 0;
	uint32_t size_available = 0;

	tpm_pcr_allocate_bank_t banks[] = {
		{ .hash_alg = TPM_ALG_SHA1, .pcr_select = { 0 } },
		{ .hash_alg = TPM_ALG_SHA256, .pcr_select = { 0 } },
		{ .hash_alg = TPM_ALG_SHA384, .pcr_select = { 0 } },
		{ .hash_alg = TPM_ALG_NULL }
	};
	for (i = 0; i < ARRAY_SIZE(banks); i++) {
		if (banks[i].hash_alg == hash_alg) {
			memset(banks[i].pcr_select, 0xFF, TPM_PCR_SELECT_SIZE);
			break;
		}
		if (banks[i].hash_alg == TPM_ALG_NULL) {
			ERROR("PCR bank 0x%04x not found\n", hash_alg);
			return -1;
		}
	}
	rc = tpm_pcr_allocate_auth_password(&tpm_chip_data, NULL, 0, banks,
					    &success, &max_pcr, &size_needed,
					    &size_available);

	if (rc != TPM_SUCCESS) {
		ERROR("PCR allocate failure\n");
		return rc;
	}
	INFO("PCR allocate success=%s max_pcr=%u size_needed=%u "
	     "size_available=%u\n",
	     success ? "yes" : "no", max_pcr, size_needed, size_available);
	return success ? TPM_SUCCESS : TPM_ERR_RESPONSE;
}
#endif /* RPI3_PROVISION_TPM */
#endif /* DISCRETE_TPM */

void bl1_plat_mboot_init(void)
{
	size_t event_log_max_size __unused;
	tpm_alg_id algorithms[] = {
#ifdef TPM_ALG_ID
		TPM_ALG_ID
#else
		/*
		 * TODO: with MEASURED_BOOT=1 several algorithms now compiled into Mbed-TLS,
		 * we ought to query the backend to figure out what algorithms to use.
		 */
		EVLOG_TPM_ALG_SHA256,
		EVLOG_TPM_ALG_SHA384,
		EVLOG_TPM_ALG_SHA512,
#endif
	};
	int rc;

#if DISCRETE_TPM
	tpm_pcr_bank_ctx_t ctx = { 0 };
	rpi3_bl1_tpm_early_interface_setup();
	rc = tpm_startup(&tpm_chip_data, TPM_SU_CLEAR);
	if (rc != 0) {
		ERROR("BL1: TPM Startup failed\n");
		panic();
	}
	tpm_alg_query_t alg_query[] = {
		{ .alg_id = TPM_ALG_SHA256 },
		{ .alg_id = EVLOG_TPM_ALG_SHA384 },
		{ .alg_id = EVLOG_TPM_ALG_SHA512 },
		{ .alg_id = TPM_ALG_NULL },
	};
	rc = tpm_getcap_query_algs(&tpm_chip_data, alg_query);
	if (rc < 0) {
		ERROR("Failed to query TPM algs (%d).\n", rc);
		panic();
	}

	for (int i = 0; i < ARRAY_SIZE(alg_query); i++) {
		if (alg_query[i].enabled) {
			INFO("Hash 0x%04x enabled\n", alg_query[i].alg_id);
		} else {
			INFO("Hash 0x%04x disabled\n", alg_query[i].alg_id);
		}
	}
	rc = tpm_for_each_pcr_bank(&tpm_chip_data, pcr_cb, &ctx);
	if (rc < 0) {
		ERROR("Failed to query TPM PCR banks (%d).\n", rc);
		panic();
	}

	if ((ctx.flags & RPI3_REQUIRED_TPM_PCR_MASK) !=
	    RPI3_REQUIRED_TPM_PCR_MASK) {
#if RPI3_PROVISION_TPM
		WARN("Reallocating TPM PCRs\n");
		rc = allocate_pcr_bank(TPM_ALG_ID);
		if (rc < 0) {
			ERROR("Failed to provision TPM PCR banks (%d).\n", rc);
			panic();
		}

#else
		ERROR("Required PCRs missing for bank 0x%04x\n", TPM_ALG_ID);
		ERROR("Change MBOOT_TPM_HASH_ALG or provision TPM\n");
		panic();
#endif
	}

#endif

#if TRANSFER_LIST
	event_log_max_size = PLAT_ARM_EVENT_LOG_MAX_SIZE;
	event_log = transfer_list_event_log_extend(secure_tl, event_log_max_size);
	assert(event_log != NULL);
	rc = event_log_init_and_reg(event_log, event_log + event_log_max_size,
					0U, crypto_mod_tcg_hash);
#else
	rc = event_log_init_and_reg(event_log, event_log + sizeof(event_log),
				    0U, crypto_mod_tcg_hash);
#endif
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
	uint8_t *rc_ptr __unused;

	event_log_cur_size = event_log_get_cur_size(event_log);
	image_desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(image_desc != NULL);

	/* Get the entry point info */
	ep_info = &image_desc->ep_info;
#if TRANSFER_LIST
	/* Finalize event log TE size and set TL handoff args */
	rc_ptr = transfer_list_event_log_finish(
		secure_tl, (uintptr_t)event_log + event_log_cur_size);
	if (rc_ptr == NULL) {
		ERROR("BL1: Failed to finalize Event Log TL entry\n");
		panic();
	}
	/* Ensure changes are visible to the next stage. */
	flush_dcache_range((uintptr_t)secure_tl, secure_tl->size);
	ep_info->args.arg3 = (uint64_t)secure_tl;
#else
	ep_info->args.arg2 = (uint64_t) event_log;
	ep_info->args.arg3 = (uint32_t) event_log_cur_size;
#endif

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
