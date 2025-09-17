/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#if TRANSFER_LIST
#include <tpm_event_log.h>
#endif
#include <plat/arm/common/plat_arm.h>

#include <drivers/auth/crypto_mod.h>
#include <drivers/measured_boot/metadata.h>
#include <event_measure.h>
#include <event_print.h>
#include <tools_share/zero_oid.h>

static const struct event_log_hash_info crypto_hash_info = {
	.func = crypto_mod_calc_hash,
	.ids = (const uint32_t[]){ CRYPTO_MD_ID },
	.count = 1U,
};

/* Event Log data */
static uint8_t *event_log;

/* Juno table with platform specific image IDs, names and PCRs */
const event_log_metadata_t juno_event_log_metadata[] = {
	{ FW_CONFIG_ID, MBOOT_FW_CONFIG_STRING, PCR_0 },
	{ TB_FW_CONFIG_ID, MBOOT_TB_FW_CONFIG_STRING, PCR_0 },
	{ BL2_IMAGE_ID, MBOOT_BL2_IMAGE_STRING, PCR_0 },
	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) } /* Terminator */
};

void bl1_plat_mboot_init(void)
{
#if TRANSFER_LIST
	size_t event_log_max_size = PLAT_ARM_EVENT_LOG_MAX_SIZE;
	int rc;

	event_log =
		transfer_list_event_log_extend(secure_tl, event_log_max_size);
	assert(event_log != NULL);

	rc = event_log_init_and_reg(event_log, event_log + event_log_max_size,
				    &crypto_hash_info);
	if (rc < 0) {
		ERROR("Failed to initialize event log (%d).\n", rc);
		panic();
	}

	rc = event_log_write_header();
	if (rc < 0) {
		ERROR("Failed to write event log header (%d).\n", rc);
		panic();
	}
#endif
}

void bl1_plat_mboot_finish(void)
{
#if TRANSFER_LIST
	uint8_t *rc __unused;
	size_t event_log_cur_size = event_log_get_cur_size(event_log);

	rc = transfer_list_event_log_finish(
		secure_tl, (uintptr_t)event_log + event_log_cur_size);

	if (rc != NULL)
		return;

	/*
	 * Panic if we fail to set up the event log for the next stage.
	 * This is a fatal error because, on the Juno platform,
	 * BL2 software assumes that a valid event Log buffer exists and
	 * will use the same event Log buffer to append image
	 * measurements.
	 */
	panic();
#endif
}
