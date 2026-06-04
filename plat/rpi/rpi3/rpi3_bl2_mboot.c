/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <drivers/auth/crypto_mod.h>
#include <drivers/delay_timer.h>
#include <drivers/gpio_spi.h>
#include <drivers/measured_boot/metadata.h>
#include <drivers/tpm/tpm2_slb9670/slb9670_gpio.h>
#include <event_measure.h>
#include <event_print.h>
#if DISCRETE_TPM
#include <tpm2.h>
#include <tpm2_chip.h>
#endif
#if TRANSFER_LIST
#include <tpm_event_log.h>
#include <transfer_list.h>
extern struct transfer_list_header *bl2_tl;
#endif
#include <tools_share/tbbr_oid.h>

#include "./include/rpi3_measured_boot.h"

/* RPI3 table with platform specific image IDs, names and PCRs */
const event_log_metadata_t rpi3_event_log_metadata[] = {
	{ BL31_IMAGE_ID, MBOOT_BL31_IMAGE_STRING, PCR_0 },
	{ BL33_IMAGE_ID, MBOOT_BL33_IMAGE_STRING, PCR_0 },
	{ NT_FW_CONFIG_ID, MBOOT_NT_FW_CONFIG_STRING, PCR_0 },
#if defined(SPD_opteed) || defined(SPD_spmd)
	{ BL32_IMAGE_ID, MBOOT_BL32_IMAGE_STRING, PCR_0 },
	{ BL32_EXTRA1_IMAGE_ID, MBOOT_BL32_EXTRA1_IMAGE_STRING, PCR_0 },
#endif
#if defined(SPD_spmd)
	{ TOS_FW_CONFIG_ID, MBOOT_TOS_FW_CONFIG_STRING, PCR_0 },
#endif

	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) } /* Terminator */
};

#if DISCRETE_TPM
extern struct tpm_chip_data tpm_chip_data;

static void rpi3_bl2_tpm_early_interface_setup(void)
{
#if TPM_INTERFACE_FIFO_SPI
	struct tpm_spi_plat *spidev;
	const struct tpm_timeout_ops timeout_ops = {
		.timeout_init_us = timeout_init_us,
		.timeout_elapsed = timeout_elapsed
	};

	const struct gpio_spi_config *tpm_rpi3_gpio_data =
		tpm2_slb9670_get_config();
	int rc;

	tpm2_slb9670_gpio_init(tpm_rpi3_gpio_data);

	spidev = gpio_spi_init(tpm_rpi3_gpio_data);

	rc = tpm_interface_init(spidev, &timeout_ops, &tpm_chip_data, 0);
	if (rc != 0) {
		ERROR("BL2: TPM interface init failed\n");
		panic();
	}

#endif
}
#endif

static uint8_t *event_log_start;
static size_t event_log_size;
static uint8_t *event_log_base;
static uint8_t *event_log_finish;

void bl2_plat_mboot_init(void)
{
	struct transfer_list_entry *te __unused;
	int rc;
#if DISCRETE_TPM
	rpi3_bl2_tpm_early_interface_setup();
#endif

#if TRANSFER_LIST
	if (bl2_tl != NULL &&
	    transfer_list_check_header(bl2_tl) != TL_OPS_NON) {
		event_log_start = transfer_list_event_log_extend(
			bl2_tl, PLAT_ARM_EVENT_LOG_MAX_SIZE);
		flush_dcache_range((uintptr_t)bl2_tl, bl2_tl->size);
		/*
	 * Retrieve the extend event log entry from the transfer list, the API above
	 * returns a cursor position rather than the base address - we need both to
	 * init the library.
	 */
		te = transfer_list_find(bl2_tl, TL_TAG_TPM_EVLOG);

		event_log_base =
			transfer_list_entry_data(te) + EVENT_LOG_RESERVED_BYTES;
		event_log_finish = transfer_list_entry_data(te) + te->data_size;

		event_log_size = event_log_start - event_log_base;
	} else {
		ERROR("BL2: Valid transfer list not found");
		panic();
	}
#else
	rpi3_mboot_fetch_eventlog_info(&event_log_start, &event_log_size);
	event_log_base = event_log_start;
	event_log_finish = event_log_start + PLAT_ARM_EVENT_LOG_MAX_SIZE;
#endif
	rc = event_log_init_and_reg(event_log_base, event_log_finish,
				    event_log_size, crypto_mod_tcg_hash);
	if (rc < 0) {
		ERROR("Failed to initialize event log (%d).\n", rc);
		panic();
	}
}

void bl2_plat_mboot_finish(void)
{
	int __maybe_unused rc;
	size_t event_log_cur_size;
#if TRANSFER_LIST
	struct transfer_list_header *ns_tl = NULL;
	uint8_t *cursor;
	uint8_t *base_after_finish;

	event_log_cur_size = event_log_get_cur_size(event_log_start);
	cursor = event_log_start + event_log_cur_size;
	base_after_finish =
		transfer_list_event_log_finish(bl2_tl, (uintptr_t)cursor);
	if (base_after_finish == NULL) {
		WARN("BL2: Failed to finalize TL Event Log\n");
	} else {
		flush_dcache_range((uintptr_t)bl2_tl, bl2_tl->size);
		transfer_list_update_checksum(bl2_tl);
		event_log_start = base_after_finish;
		event_log_cur_size = event_log_get_cur_size(event_log_start);
#ifdef FW_NS_HANDOFF_BASE
		/* Update contents in NS Transfer List at FW_NS_HANDOFF_BASE */
		ns_tl = transfer_list_relocate(
			bl2_tl, (void *)(uintptr_t)FW_NS_HANDOFF_BASE,
			bl2_tl->max_size);
		if (!ns_tl) {
			ERROR("Relocate TL to 0x%lx failed\n",
			      (unsigned long)FW_NS_HANDOFF_BASE);
			panic();
		}
#endif
	}
#else

	/* Event Log address in Non-Secure memory */
	uintptr_t ns_log_addr;

	event_log_cur_size = event_log_get_cur_size((uint8_t *)event_log_start);

	/* write the eventlog addr and size to NT_FW_CONFIG TPM entry */
	rc = rpi3_set_nt_fw_info(event_log_cur_size, &ns_log_addr);
	if (rc != 0) {
		ERROR("%s(): Unable to update %s_FW_CONFIG\n", __func__, "NT");
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
#endif /* TRANSFER_LIST */
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
