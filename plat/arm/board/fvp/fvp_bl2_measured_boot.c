/*
 * Copyright (c) 2021-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/tbbr/tbbr_img_def.h>
#if TRANSFER_LIST
#include <tpm_event_log.h>
#endif
#include <common/measured_boot.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/measured_boot/metadata.h>
#include <event_measure.h>
#include <event_print.h>
#if defined(ARM_COT_cca)
#include <tools_share/cca_oid.h>
#else
#include <tools_share/tbbr_oid.h>
#endif /* ARM_COT_cca */
#include <fvp_critical_data.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/common_def.h>

#if !TRANSFER_LIST && (defined(SPD_tspd) || defined(SPD_opteed) || defined(SPD_spmd))
CASSERT(ARM_EVENT_LOG_DRAM1_SIZE >= PLAT_ARM_EVENT_LOG_MAX_SIZE, \
	assert_res_eventlog_mem_insufficient);
#endif /* defined(SPD_tspd) || defined(SPD_opteed) || defined(SPD_spmd) */

/* Event Log data */
static const uint8_t *event_log_base;

/* FVP table with platform specific image IDs, names and PCRs */
const event_log_metadata_t fvp_event_log_metadata[] = {
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
	{ RMM_IMAGE_ID, MBOOT_RMM_IMAGE_STRING, PCR_0},

#if defined(SPD_spmd)
	{ SP_PKG1_ID, MBOOT_SP1_STRING, PCR_0 },
	{ SP_PKG2_ID, MBOOT_SP2_STRING, PCR_0 },
	{ SP_PKG3_ID, MBOOT_SP3_STRING, PCR_0 },
	{ SP_PKG4_ID, MBOOT_SP4_STRING, PCR_0 },
	{ SP_PKG5_ID, MBOOT_SP5_STRING, PCR_0 },
	{ SP_PKG6_ID, MBOOT_SP6_STRING, PCR_0 },
	{ SP_PKG7_ID, MBOOT_SP7_STRING, PCR_0 },
	{ SP_PKG8_ID, MBOOT_SP8_STRING, PCR_0 },
#endif

	{ CRITICAL_DATA_ID, EVLOG_CRITICAL_DATA_STRING, PCR_1 },

	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

void bl2_plat_mboot_init(void)
{
	uint8_t *event_log_start __unused;
	uint8_t *event_log_finish;
	size_t bl1_event_log_size __unused = 0;
	size_t event_log_max_size __unused = 0;
	struct transfer_list_entry *te __unused;
	int rc __unused;

#if TRANSFER_LIST
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
#else
	rc = arm_get_tb_fw_info((uint64_t *)&event_log_base,
				&bl1_event_log_size, &event_log_max_size);
	if (rc != 0) {
		ERROR("%s(): Unable to get Event Log info from TB_FW_CONFIG\n",
		      __func__);
		/*
		 * It is a fatal error because on FVP platform, BL2 software
		 * assumes that a valid Event Log buffer exist and it will use
		 * same Event Log buffer to append image measurements.
		 */
		panic();
	}

	/*
	 * BL1 and BL2 share the same Event Log buffer and that BL2 will
	 * append its measurements after BL1's
	 */
	event_log_finish =
		(uint8_t *)((uintptr_t)event_log_base + event_log_max_size);
#endif

	rc = event_log_init_and_reg((uint8_t *)event_log_base, event_log_finish,
				    bl1_event_log_size, crypto_mod_tcg_hash);
	if (rc < 0) {
		ERROR("Failed to initialize event log (%d).\n", rc);
		panic();
	}
}

int plat_mboot_measure_critical_data(unsigned int critical_data_id,
				     const void *base, size_t size)
{
	const event_log_metadata_t *metadata_ptr;
	int err;

	/*
	 * It is very unlikely that the critical data size would be
	 * bigger than 2^32 bytes
	 */
	assert(size < UINT32_MAX);
	assert(base != NULL);

	metadata_ptr = mboot_find_event_log_metadata(fvp_event_log_metadata,
						     critical_data_id);
	if (metadata_ptr == NULL) {
		return 0;
	}

	/* Calculate image hash and record data in Event Log */
	err = event_log_measure_and_record(metadata_ptr->pcr, (uintptr_t)base,
					   size, metadata_ptr->name,
					   strlen(metadata_ptr->name) + 1U);
	if (err != 0) {
		ERROR("%s%s critical data (%i)\n",
		      "Failed to ", "record",  err);
		return err;
	}

	return 0;
}

#if TRUSTED_BOARD_BOOT
static int fvp_populate_critical_data(struct fvp_critical_data *critical_data)
{
	char *nv_ctr_oids[MAX_NV_CTR_IDS] = {
		[TRUSTED_NV_CTR_ID] = TRUSTED_FW_NVCOUNTER_OID,
		[NON_TRUSTED_NV_CTR_ID] = NON_TRUSTED_FW_NVCOUNTER_OID,
	};

	for (int i = 0; i < MAX_NV_CTR_IDS; i++) {
		int rc = plat_get_nv_ctr(nv_ctr_oids[i],
					 &critical_data->nv_ctr[i]);
		if (rc != 0) {
			return rc;
		}
	}

	return 0;
}
#endif /* TRUSTED_BOARD_BOOT */

static int fvp_populate_and_measure_critical_data(void)
{
	int rc = 0;

/*
 * FVP platform only measures 'platform NV-counter' and hence its
 * measurement makes sense during Trusted-Boot flow only.
 */
#if TRUSTED_BOARD_BOOT
	struct fvp_critical_data populate_critical_data;

	rc = fvp_populate_critical_data(&populate_critical_data);
	if (rc == 0) {
		rc = plat_mboot_measure_critical_data(CRITICAL_DATA_ID,
						&populate_critical_data,
						sizeof(populate_critical_data));
	}
#endif /* TRUSTED_BOARD_BOOT */

	return rc;
}

void bl2_plat_mboot_finish(void)
{
	int rc;

	/* Event Log address in Non-Secure memory */
	uintptr_t ns_log_addr __unused;

	/* Event Log filled size */
	size_t event_log_cur_size;

	struct transfer_list_entry *te __maybe_unused;

	rc = fvp_populate_and_measure_critical_data();
	if (rc != 0) {
		panic();
	}

	event_log_cur_size = event_log_get_cur_size((uint8_t *)event_log_base);

#if TRANSFER_LIST
	/*
	 * Re-size the event log for the next stage and update the size to include
	 * the entire event log (i.e., not just what this stage has added.)
	 */
	event_log_base = transfer_list_event_log_finish(
		secure_tl, (uintptr_t)event_log_base + event_log_cur_size);

	/* Ensure changes are visible to the next stage. */
	flush_dcache_range((uintptr_t)secure_tl, secure_tl->size);

	event_log_cur_size = event_log_get_cur_size((uint8_t *)event_log_base);

	/* If there is DT_SPMC_MANIFEST, update event log information. */
	te = transfer_list_find(secure_tl, TL_TAG_DT_SPMC_MANIFEST);
	if (te != NULL) {
		te = transfer_list_find(secure_tl, TL_TAG_TPM_EVLOG);
		assert(te != NULL && te->data_size > 0);

		rc = arm_set_tos_fw_info((uintptr_t)transfer_list_entry_data(te),
					 te->data_size);
		if (rc != 0) {
			WARN("%s(): Unable to update %s_FW_CONFIG\n",
			      __func__, "TOS");
		}

		transfer_list_update_checksum(secure_tl);
	}
#else
#if defined(SPD_tspd) || defined(SPD_opteed) || defined(SPD_spmd)
	/* Copy Event Log to TZC secured DRAM memory */
	(void)memcpy((void *)ARM_EVENT_LOG_DRAM1_BASE,
		     (const void *)event_log_base,
		     event_log_cur_size);

	/* Ensure that the Event Log is visible in TZC secured DRAM memory */
	flush_dcache_range(ARM_EVENT_LOG_DRAM1_BASE, event_log_cur_size);
#endif /* defined(SPD_tspd) || defined(SPD_opteed) || defined(SPD_spmd) */

	rc = arm_set_nt_fw_info(
#ifdef SPD_opteed
			    (uintptr_t)ARM_EVENT_LOG_DRAM1_BASE,
#endif
			    event_log_cur_size, &ns_log_addr);
	if (rc != 0) {
		ERROR("%s(): Unable to update %s_FW_CONFIG\n",
		      __func__, "NT");
		/*
		 * It is a fatal error because on FVP secure world software
		 * assumes that a valid event log exists and will use it to
		 * record the measurements into the fTPM.
		 * Note: In FVP platform, OP-TEE uses nt_fw_config to get the
		 * secure Event Log buffer address.
		 */
		panic();
	}

	/* Copy Event Log to Non-secure memory */
	(void)memcpy((void *)ns_log_addr, (const void *)event_log_base,
		     event_log_cur_size);

	/* Ensure that the Event Log is visible in Non-secure memory */
	flush_dcache_range(ns_log_addr, event_log_cur_size);

#if defined(SPD_tspd) || defined(SPD_spmd)
	/* Set Event Log data in TOS_FW_CONFIG */
	rc = arm_set_tos_fw_info((uintptr_t)ARM_EVENT_LOG_DRAM1_BASE,
				 event_log_cur_size);
	if (rc != 0) {
		ERROR("%s(): Unable to update %s_FW_CONFIG\n",
		      __func__, "TOS");
		panic();
	}
#endif /* defined(SPD_tspd) || defined(SPD_spmd) */
#endif /* TRANSFER_LIST */

	event_log_dump((uint8_t *)event_log_base, event_log_cur_size);
}
