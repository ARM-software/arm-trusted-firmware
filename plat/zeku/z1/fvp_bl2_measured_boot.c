/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/measured_boot/event_log/event_log.h>
#include <plat/arm/common/plat_arm.h>

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
	{ INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
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
}

void bl2_plat_mboot_finish(void)
{
	int rc;

	/* Event Log address in Non-Secure memory */
	uintptr_t ns_log_addr;

	/* Event Log filled size */
	size_t event_log_cur_size;

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
