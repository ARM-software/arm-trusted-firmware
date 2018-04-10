/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MCE_PRIVATE_H__
#define __MCE_PRIVATE_H__

#include <mmio.h>
#include <tegra_def.h>

/*******************************************************************************
 * Macros to prepare CSTATE info request
 ******************************************************************************/
/* Description of the parameters for UPDATE_CSTATE_INFO request */
#define CLUSTER_CSTATE_MASK			0x7
#define CLUSTER_CSTATE_SHIFT			0
#define CLUSTER_CSTATE_UPDATE_BIT		(1 << 7)
#define CCPLEX_CSTATE_MASK			0x3
#define CCPLEX_CSTATE_SHIFT			8
#define CCPLEX_CSTATE_UPDATE_BIT		(1 << 15)
#define SYSTEM_CSTATE_MASK			0xF
#define SYSTEM_CSTATE_SHIFT			16
#define SYSTEM_CSTATE_FORCE_UPDATE_SHIFT	22
#define SYSTEM_CSTATE_FORCE_UPDATE_BIT		(1 << 22)
#define SYSTEM_CSTATE_UPDATE_BIT		(1 << 23)
#define CSTATE_WAKE_MASK_UPDATE_BIT		(1 << 31)
#define CSTATE_WAKE_MASK_SHIFT			32
#define CSTATE_WAKE_MASK_CLEAR			0xFFFFFFFF

/*******************************************************************************
 * Auto-CC3 control macros
 ******************************************************************************/
#define MCE_AUTO_CC3_FREQ_MASK			0x1FF
#define MCE_AUTO_CC3_FREQ_SHIFT			0
#define MCE_AUTO_CC3_VTG_MASK			0x7F
#define MCE_AUTO_CC3_VTG_SHIFT			16
#define MCE_AUTO_CC3_ENABLE_BIT			(1 << 31)

/*******************************************************************************
 * Macros for the 'IS_SC7_ALLOWED' command
 ******************************************************************************/
#define MCE_SC7_ALLOWED_MASK			0x7
#define MCE_SC7_WAKE_TIME_SHIFT			32

/*******************************************************************************
 * Macros for 'read/write ctats' commands
 ******************************************************************************/
#define MCE_CSTATE_STATS_TYPE_SHIFT		32
#define MCE_CSTATE_WRITE_DATA_LO_MASK		0xF

/*******************************************************************************
 * Macros for 'update crossover threshold' command
 ******************************************************************************/
#define MCE_CROSSOVER_THRESHOLD_TIME_SHIFT	32

/*******************************************************************************
 * Timeout value used to powerdown a core
 ******************************************************************************/
#define MCE_CORE_SLEEP_TIME_INFINITE		0xFFFFFFFF

/*******************************************************************************
 * MCA command struct
 ******************************************************************************/
typedef union mca_cmd {
	struct command {
		uint8_t cmd;
		uint8_t idx;
		uint8_t subidx;
	} command;
	struct input {
		uint32_t low;
		uint32_t high;
	} input;
	uint64_t data;
} mca_cmd_t;

/*******************************************************************************
 * MCA argument struct
 ******************************************************************************/
typedef union mca_arg {
	struct err {
		uint64_t error:8;
		uint64_t unused:48;
		uint64_t finish:8;
	} err;
	struct arg {
		uint32_t low;
		uint32_t high;
	} arg;
	uint64_t data;
} mca_arg_t;

/*******************************************************************************
 * Uncore PERFMON ARI struct
 ******************************************************************************/
typedef union uncore_perfmon_req {
	struct perfmon_command {
		/*
		 * Commands: 0 = READ, 1 = WRITE
		 */
		uint64_t cmd:8;
		/*
		 * The unit group: L2=0, L3=1, ROC=2, MC=3, IOB=4
		 */
		uint64_t grp:4;
		/*
		 * Unit selector: Selects the unit instance, with 0 = Unit
		 * = (number of units in group) - 1.
		 */
		uint64_t unit:4;
		/*
		 * Selects the uncore perfmon register to access
		 */
		uint64_t reg:8;
		/*
		 * Counter number. Selects which counter to use for
		 * registers NV_PMEVCNTR and NV_PMEVTYPER.
		 */
		uint64_t counter:8;
	} perfmon_command;
	struct perfmon_status {
		/*
		 * Resulting command status
		 */
		uint64_t val:8;
		uint64_t unused:24;
	} perfmon_status;
	uint64_t data;
} uncore_perfmon_req_t;

#define UNCORE_PERFMON_CMD_READ			0
#define UNCORE_PERFMON_CMD_WRITE		1

#define UNCORE_PERFMON_CMD_MASK			0xFF
#define UNCORE_PERFMON_UNIT_GRP_MASK		0xF
#define UNCORE_PERFMON_SELECTOR_MASK		0xF
#define UNCORE_PERFMON_REG_MASK			0xFF
#define UNCORE_PERFMON_CTR_MASK			0xFF
#define UNCORE_PERFMON_RESP_STATUS_MASK		0xFF

/* declarations for NVG handler functions */
int nvg_enter_cstate(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int nvg_update_cstate_info(uint32_t ari_base, uint32_t cluster, uint32_t ccplex,
		uint32_t system, uint8_t sys_state_force, uint32_t wake_mask,
		uint8_t update_wake_mask);
int nvg_update_crossover_time(uint32_t ari_base, uint32_t type, uint32_t time);
uint64_t nvg_read_cstate_stats(uint32_t ari_base, uint32_t state);
int nvg_write_cstate_stats(uint32_t ari_base, uint32_t state, uint32_t val);
int nvg_is_ccx_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int nvg_is_sc7_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int nvg_online_core(uint32_t ari_base, uint32_t core);
int nvg_cc3_ctrl(uint32_t ari_base, uint32_t freq, uint32_t volt, uint8_t enable);

#endif /* __MCE_PRIVATE_H__ */
