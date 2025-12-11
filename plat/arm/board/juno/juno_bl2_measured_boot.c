/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/tbbr/tbbr_img_def.h>
#if TRANSFER_LIST
#include <tpm_event_log.h>
#endif
#include <plat/arm/common/plat_arm.h>
#include <plat/common/common_def.h>

#include <drivers/auth/crypto_mod.h>
#include <drivers/measured_boot/metadata.h>
#include <event_measure.h>
#include <event_print.h>
#if defined(ARM_COT_cca)
#include <tools_share/cca_oid.h>
#else
#include <tools_share/tbbr_oid.h>
#endif /* ARM_COT_cca */

/* Event Log data */
static uint8_t *event_log_base;

/* table with platform specific image IDs, names and PCRs */
const event_log_metadata_t juno_event_log_metadata[] = {
	{ BL31_IMAGE_ID, MBOOT_BL31_IMAGE_STRING, PCR_0 },
	{ BL32_IMAGE_ID, MBOOT_BL32_IMAGE_STRING, PCR_0 },
	{ BL32_EXTRA1_IMAGE_ID, MBOOT_BL32_EXTRA1_IMAGE_STRING, PCR_0 },
	{ BL32_EXTRA2_IMAGE_ID, MBOOT_BL32_EXTRA2_IMAGE_STRING, PCR_0 },
	{ BL33_IMAGE_ID, MBOOT_BL33_IMAGE_STRING, PCR_0 },
	{ HW_CONFIG_ID, MBOOT_HW_CONFIG_STRING, PCR_0 },
	{ NT_FW_CONFIG_ID, MBOOT_NT_FW_CONFIG_STRING, PCR_0 },
	{ SCP_BL2_IMAGE_ID, MBOOT_SCP_BL2_IMAGE_STRING, PCR_0 },
	{ SOC_FW_CONFIG_ID, MBOOT_SOC_FW_CONFIG_STRING, PCR_0 },
	{ TOS_FW_CONFIG_ID, MBOOT_TOS_FW_CONFIG_STRING, PCR_0 },
	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

void bl2_plat_mboot_init(void)
{
#if TRANSFER_LIST
	uint8_t *event_log_start;
	uint8_t *event_log_finish;
	size_t bl1_event_log_size;
	struct transfer_list_entry *te;
	int rc;

	event_log_start = transfer_list_event_log_extend(
		secure_tl, PLAT_ARM_EVENT_LOG_MAX_SIZE);

	/*
	 * Retrieve the extend event log entry from the transfer list, the API above
	 * returns a cursor position rather than the base address - we need both to
	 * init the library.
	 */
	te = transfer_list_find(secure_tl, TL_TAG_TPM_EVLOG);

	event_log_base =
		transfer_list_entry_data(te) + EVENT_LOG_RESERVED_BYTES;
	event_log_finish = transfer_list_entry_data(te) + te->data_size;

	bl1_event_log_size = event_log_start - event_log_base;

	rc = event_log_init_and_reg((uint8_t *)event_log_base, event_log_finish,
				    bl1_event_log_size, crypto_mod_tcg_hash);
	if (rc < 0) {
		ERROR("Failed to initialize event log (%d).\n", rc);
		panic();
	}
#endif
}

int plat_mboot_measure_critical_data(unsigned int critical_data_id,
				     const void *base, size_t size)
{
	/* Nothing */
	return 0;
}

void bl2_plat_mboot_finish(void)
{
#if TRANSFER_LIST
	/* Event Log filled size */
	size_t event_log_cur_size;

	event_log_cur_size = event_log_get_cur_size((uint8_t *)event_log_base);

	/*
	 * Re-size the event log for the next stage and update the size to include
	 * the entire event log (i.e., not just what this stage has added.)
	 */
	event_log_base = transfer_list_event_log_finish(
		secure_tl, (uintptr_t)event_log_base + event_log_cur_size);
	if (event_log_base == NULL) {
		panic();
	}

	event_log_cur_size = event_log_get_cur_size((uint8_t *)event_log_base);
	event_log_dump(event_log_base, event_log_cur_size);
#endif /* TRANSFER_LIST */
}
