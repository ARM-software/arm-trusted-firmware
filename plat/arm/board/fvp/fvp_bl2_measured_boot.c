/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/measured_boot/event_log/event_log.h>
#include <drivers/measured_boot/rss/rss_measured_boot.h>
#include <tools_share/tbbr_oid.h>
#include <fvp_critical_data.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/common_def.h>

/* Event Log data */
static uint64_t event_log_base;

/* FVP table with platform specific image IDs, names and PCRs */
const event_log_metadata_t fvp_event_log_metadata[] = {
	{ BL31_IMAGE_ID, EVLOG_BL31_STRING, PCR_0 },
	{ BL32_IMAGE_ID, EVLOG_BL32_STRING, PCR_0 },
	{ BL32_EXTRA1_IMAGE_ID, EVLOG_BL32_EXTRA1_STRING, PCR_0 },
	{ BL32_EXTRA2_IMAGE_ID, EVLOG_BL32_EXTRA2_STRING, PCR_0 },
	{ BL33_IMAGE_ID, EVLOG_BL33_STRING, PCR_0 },
	{ HW_CONFIG_ID, EVLOG_HW_CONFIG_STRING, PCR_0 },
	{ NT_FW_CONFIG_ID, EVLOG_NT_FW_CONFIG_STRING, PCR_0 },
	{ SCP_BL2_IMAGE_ID, EVLOG_SCP_BL2_STRING, PCR_0 },
	{ SOC_FW_CONFIG_ID, EVLOG_SOC_FW_CONFIG_STRING, PCR_0 },
	{ TOS_FW_CONFIG_ID, EVLOG_TOS_FW_CONFIG_STRING, PCR_0 },
	{ RMM_IMAGE_ID, EVLOG_RMM_STRING, PCR_0},

	{ CRITICAL_DATA_ID, EVLOG_CRITICAL_DATA_STRING, PCR_1 },

	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

/* FVP table with platform specific image IDs and metadata. Intentionally not a
 * const struct, some members might set by bootloaders during trusted boot.
 */
struct rss_mboot_metadata fvp_rss_mboot_metadata[] = {
	{
		.id = BL31_IMAGE_ID,
		.slot = U(9),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_BL31_STRING,
		.lock_measurement = true },
	{
		.id = HW_CONFIG_ID,
		.slot = U(10),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_HW_CONFIG_STRING,
		.lock_measurement = true },
	{
		.id = SOC_FW_CONFIG_ID,
		.slot = U(11),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_SOC_FW_CONFIG_STRING,
		.lock_measurement = true },
	{
		.id = RMM_IMAGE_ID,
		.slot = U(12),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_RMM_STRING,
		.lock_measurement = true },
	{
		.id = RSS_MBOOT_INVALID_ID }
};

void bl2_plat_mboot_init(void)
{
	uint8_t *event_log_start;
	uint8_t *event_log_finish;
	size_t bl1_event_log_size;
	int rc;

	rc = arm_get_tb_fw_info(&event_log_base, &bl1_event_log_size);
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
	event_log_start = (uint8_t *)((uintptr_t)event_log_base +
				      bl1_event_log_size);
	event_log_finish = (uint8_t *)((uintptr_t)event_log_base +
				       PLAT_ARM_EVENT_LOG_MAX_SIZE);

	event_log_init((uint8_t *)event_log_start, event_log_finish);

	rss_measured_boot_init();
}

int plat_mboot_measure_critical_data(unsigned int critical_data_id,
				     const void *base, size_t size)
{
	/*
	 * It is very unlikely that the critical data size would be
	 * bigger than 2^32 bytes
	 */
	assert(size < UINT32_MAX);
	assert(base != NULL);

	/* Calculate image hash and record data in Event Log */
	int err = event_log_measure_and_record((uintptr_t)base, (uint32_t)size,
					       critical_data_id);
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
	uintptr_t ns_log_addr;

	/* Event Log filled size */
	size_t event_log_cur_size;

	rc = fvp_populate_and_measure_critical_data();
	if (rc != 0) {
		panic();
	}

	event_log_cur_size = event_log_get_cur_size((uint8_t *)event_log_base);

	rc = arm_set_nt_fw_info(
#ifdef SPD_opteed
			    (uintptr_t)event_log_base,
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
	rc = arm_set_tos_fw_info((uintptr_t)event_log_base,
				 event_log_cur_size);
	if (rc != 0) {
		ERROR("%s(): Unable to update %s_FW_CONFIG\n",
		      __func__, "TOS");
		panic();
	}
#endif /* defined(SPD_tspd) || defined(SPD_spmd) */

	dump_event_log((uint8_t *)event_log_base, event_log_cur_size);
}
