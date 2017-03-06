/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MCE_H__
#define __MCE_H__

#include <mmio.h>
#include <tegra_def.h>

/*******************************************************************************
 * MCE commands
 ******************************************************************************/
typedef enum mce_cmd {
	MCE_CMD_ENTER_CSTATE = 0U,
	MCE_CMD_UPDATE_CSTATE_INFO = 1U,
	MCE_CMD_UPDATE_CROSSOVER_TIME = 2U,
	MCE_CMD_READ_CSTATE_STATS = 3U,
	MCE_CMD_WRITE_CSTATE_STATS = 4U,
	MCE_CMD_IS_SC7_ALLOWED = 5U,
	MCE_CMD_ONLINE_CORE = 6U,
	MCE_CMD_CC3_CTRL = 7U,
	MCE_CMD_ECHO_DATA = 8U,
	MCE_CMD_READ_VERSIONS = 9U,
	MCE_CMD_ENUM_FEATURES = 10U,
	MCE_CMD_ROC_FLUSH_CACHE_TRBITS = 11U,
	MCE_CMD_ENUM_READ_MCA = 12U,
	MCE_CMD_ENUM_WRITE_MCA = 13U,
	MCE_CMD_ROC_FLUSH_CACHE = 14U,
	MCE_CMD_ROC_CLEAN_CACHE = 15U,
	MCE_CMD_ENABLE_LATIC = 16U,
	MCE_CMD_UNCORE_PERFMON_REQ = 17U,
	MCE_CMD_MISC_CCPLEX = 18U,
	MCE_CMD_IS_CCX_ALLOWED = 0xFEU,
	MCE_CMD_MAX = 0xFFU,
} mce_cmd_t;

#define MCE_CMD_MASK				0xFFU

/*******************************************************************************
 * Timeout value used to powerdown a core
 ******************************************************************************/
#define MCE_CORE_SLEEP_TIME_INFINITE		0xFFFFFFFFU

/*******************************************************************************
 * Struct to prepare UPDATE_CSTATE_INFO request
 ******************************************************************************/
typedef struct mce_cstate_info {
	/* cluster cstate value */
	uint32_t cluster;
	/* ccplex cstate value */
	uint32_t ccplex;
	/* system cstate value */
	uint32_t system;
	/* force system state? */
	uint8_t system_state_force;
	/* wake mask value */
	uint32_t wake_mask;
	/* update the wake mask? */
	uint8_t update_wake_mask;
} mce_cstate_info_t;

/* public interfaces */
int mce_command_handler(uint64_t cmd, uint64_t arg0, uint64_t arg1,
		uint64_t arg2);
int mce_update_reset_vector(void);
int mce_update_gsc_videomem(void);
int mce_update_gsc_tzdram(void);
int mce_update_gsc_tzram(void);
__dead2 void mce_enter_ccplex_state(uint32_t state_idx);
void mce_update_cstate_info(const mce_cstate_info_t *cstate);
void mce_verify_firmware_version(void);

#endif /* __MCE_H__ */
