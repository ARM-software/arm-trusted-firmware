/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MCE_H__
#define __MCE_H__

#include <mmio.h>
#include <tegra_def.h>

/*******************************************************************************
 * MCE commands
 ******************************************************************************/
typedef enum mce_cmd {
	MCE_CMD_ENTER_CSTATE = 0,
	MCE_CMD_UPDATE_CSTATE_INFO = 1,
	MCE_CMD_UPDATE_CROSSOVER_TIME = 2,
	MCE_CMD_READ_CSTATE_STATS = 3,
	MCE_CMD_WRITE_CSTATE_STATS = 4,
	MCE_CMD_IS_SC7_ALLOWED = 5,
	MCE_CMD_ONLINE_CORE = 6,
	MCE_CMD_CC3_CTRL = 7,
	MCE_CMD_ECHO_DATA = 8,
	MCE_CMD_READ_VERSIONS = 9,
	MCE_CMD_ENUM_FEATURES = 10,
	MCE_CMD_ROC_FLUSH_CACHE_TRBITS = 11,
	MCE_CMD_ENUM_READ_MCA = 12,
	MCE_CMD_ENUM_WRITE_MCA = 13,
	MCE_CMD_ROC_FLUSH_CACHE = 14,
	MCE_CMD_ROC_CLEAN_CACHE = 15,
	MCE_CMD_ENABLE_LATIC = 16,
	MCE_CMD_UNCORE_PERFMON_REQ = 17,
	MCE_CMD_MISC_CCPLEX = 18,
	MCE_CMD_IS_CCX_ALLOWED = 0xFE,
	MCE_CMD_MAX = 0xFF,
} mce_cmd_t;

#define MCE_CMD_MASK				0xFF

/*******************************************************************************
 * Timeout value used to powerdown a core
 ******************************************************************************/
#define MCE_CORE_SLEEP_TIME_INFINITE		0xFFFFFFFF

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
int mce_command_handler(mce_cmd_t cmd, uint64_t arg0, uint64_t arg1,
		uint64_t arg2);
int mce_update_reset_vector(void);
int mce_update_gsc_videomem(void);
int mce_update_gsc_tzdram(void);
int mce_update_gsc_tzram(void);
__dead2 void mce_enter_ccplex_state(uint32_t state_idx);
void mce_update_cstate_info(mce_cstate_info_t *cstate);
void mce_verify_firmware_version(void);

#endif /* __MCE_H__ */
