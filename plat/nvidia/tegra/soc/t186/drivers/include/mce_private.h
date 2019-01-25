/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCE_PRIVATE_H
#define MCE_PRIVATE_H

#include <lib/mmio.h>

#include <tegra_def.h>

/*******************************************************************************
 * Macros to prepare CSTATE info request
 ******************************************************************************/
/* Description of the parameters for UPDATE_CSTATE_INFO request */
#define CLUSTER_CSTATE_MASK			ULL(0x7)
#define CLUSTER_CSTATE_SHIFT			U(0)
#define CLUSTER_CSTATE_UPDATE_BIT		(ULL(1) << 7)
#define CCPLEX_CSTATE_MASK			ULL(0x3)
#define CCPLEX_CSTATE_SHIFT			ULL(8)
#define CCPLEX_CSTATE_UPDATE_BIT		(ULL(1) << 15)
#define SYSTEM_CSTATE_MASK			ULL(0xF)
#define SYSTEM_CSTATE_SHIFT			ULL(16)
#define SYSTEM_CSTATE_FORCE_UPDATE_SHIFT	ULL(22)
#define SYSTEM_CSTATE_FORCE_UPDATE_BIT		(ULL(1) << 22)
#define SYSTEM_CSTATE_UPDATE_BIT		(ULL(1) << 23)
#define CSTATE_WAKE_MASK_UPDATE_BIT		(ULL(1) << 31)
#define CSTATE_WAKE_MASK_SHIFT			ULL(32)
#define CSTATE_WAKE_MASK_CLEAR			U(0xFFFFFFFF)

/*******************************************************************************
 * Auto-CC3 control macros
 ******************************************************************************/
#define MCE_AUTO_CC3_FREQ_MASK			U(0x1FF)
#define MCE_AUTO_CC3_FREQ_SHIFT			U(0)
#define MCE_AUTO_CC3_VTG_MASK			U(0x7F)
#define MCE_AUTO_CC3_VTG_SHIFT			U(16)
#define MCE_AUTO_CC3_ENABLE_BIT			(U(1) << 31)

/*******************************************************************************
 * Macros for the 'IS_SC7_ALLOWED' command
 ******************************************************************************/
#define MCE_SC7_ALLOWED_MASK			U(0x7)
#define MCE_SC7_WAKE_TIME_SHIFT			U(32)

/*******************************************************************************
 * Macros for 'read/write ctats' commands
 ******************************************************************************/
#define MCE_CSTATE_STATS_TYPE_SHIFT		ULL(32)
#define MCE_CSTATE_WRITE_DATA_LO_MASK		U(0xF)

/*******************************************************************************
 * Macros for 'update crossover threshold' command
 ******************************************************************************/
#define MCE_CROSSOVER_THRESHOLD_TIME_SHIFT	U(32)

/*******************************************************************************
 * MCA argument macros
 ******************************************************************************/
#define MCA_ARG_ERROR_MASK			U(0xFF)
#define MCA_ARG_FINISH_SHIFT			U(24)
#define MCA_ARG_FINISH_MASK			U(0xFF)

/*******************************************************************************
 * Uncore PERFMON ARI macros
 ******************************************************************************/
#define UNCORE_PERFMON_CMD_READ			U(0)
#define UNCORE_PERFMON_CMD_WRITE		U(1)

#define UNCORE_PERFMON_CMD_MASK			U(0xFF)
#define UNCORE_PERFMON_UNIT_GRP_MASK		U(0xF)
#define UNCORE_PERFMON_SELECTOR_MASK		U(0xF)
#define UNCORE_PERFMON_REG_MASK			U(0xFF)
#define UNCORE_PERFMON_CTR_MASK			U(0xFF)
#define UNCORE_PERFMON_RESP_STATUS_MASK		U(0xFF)

/*******************************************************************************
 * Structure populated by arch specific code to export routines which perform
 * common low level MCE functions
 ******************************************************************************/
typedef struct arch_mce_ops {
	/*
	 * This ARI request sets up the MCE to start execution on assertion
	 * of STANDBYWFI, update the core power state and expected wake time,
	 * then determine the proper power state to enter.
	 */
	int32_t (*enter_cstate)(uint32_t ari_base, uint32_t state,
			    uint32_t wake_time);
	/*
	 * This ARI request allows updating of the CLUSTER_CSTATE,
	 * CCPLEX_CSTATE, and SYSTEM_CSTATE register values.
	 */
	int32_t (*update_cstate_info)(uint32_t ari_base,
				  uint32_t cluster,
				  uint32_t ccplex,
				  uint32_t system,
				  uint8_t sys_state_force,
				  uint32_t wake_mask,
				  uint8_t update_wake_mask);
	/*
	 * This ARI request allows updating of power state crossover
	 * threshold times. An index value specifies which crossover
	 * state is being updated.
	 */
	int32_t (*update_crossover_time)(uint32_t ari_base,
				     uint32_t type,
				     uint32_t time);
	/*
	 * This ARI request allows read access to statistical information
	 * related to power states.
	 */
	uint64_t (*read_cstate_stats)(uint32_t ari_base,
				     uint32_t state);
	/*
	 * This ARI request allows write access to statistical information
	 * related to power states.
	 */
	int32_t (*write_cstate_stats)(uint32_t ari_base,
				  uint32_t state,
				  uint32_t stats);
	/*
	 * This ARI request allows the CPU to understand the features
	 * supported by the MCE firmware.
	 */
	uint64_t (*call_enum_misc)(uint32_t ari_base, uint32_t cmd,
				   uint32_t data);
	/*
	 * This ARI request allows querying the CCPLEX to determine if
	 * the CCx state is allowed given a target core C-state and wake
	 * time. If the CCx state is allowed, the response indicates CCx
	 * must be entered. If the CCx state is not allowed, the response
	 * indicates CC6/CC7 can't be entered
	 */
	int32_t (*is_ccx_allowed)(uint32_t ari_base, uint32_t state,
			      uint32_t wake_time);
	/*
	 * This ARI request allows querying the CCPLEX to determine if
	 * the SC7 state is allowed given a target core C-state and wake
	 * time. If the SC7 state is allowed, all cores but the associated
	 * core are offlined (WAKE_EVENTS are set to 0) and the response
	 * indicates SC7 must be entered. If the SC7 state is not allowed,
	 * the response indicates SC7 can't be entered
	 */
	int32_t (*is_sc7_allowed)(uint32_t ari_base, uint32_t state,
			      uint32_t wake_time);
	/*
	 * This ARI request allows a core to bring another offlined core
	 * back online to the C0 state. Note that a core is offlined by
	 * entering a C-state where the WAKE_MASK is all 0.
	 */
	int32_t (*online_core)(uint32_t ari_base, uint32_t cpuid);
	/*
	 * This ARI request allows the CPU to enable/disable Auto-CC3 idle
	 * state.
	 */
	int32_t (*cc3_ctrl)(uint32_t ari_base,
			uint32_t freq,
			uint32_t volt,
			uint8_t enable);
	/*
	 * This ARI request allows updating the reset vector register for
	 * D15 and A57 CPUs.
	 */
	int32_t (*update_reset_vector)(uint32_t ari_base);
	/*
	 * This ARI request instructs the ROC to flush A57 data caches in
	 * order to maintain coherency with the Denver cluster.
	 */
	int32_t (*roc_flush_cache)(uint32_t ari_base);
	/*
	 * This ARI request instructs the ROC to flush A57 data caches along
	 * with the caches covering ARM code in order to maintain coherency
	 * with the Denver cluster.
	 */
	int32_t (*roc_flush_cache_trbits)(uint32_t ari_base);
	/*
	 * This ARI request instructs the ROC to clean A57 data caches along
	 * with the caches covering ARM code in order to maintain coherency
	 * with the Denver cluster.
	 */
	int32_t (*roc_clean_cache)(uint32_t ari_base);
	/*
	 * This ARI request reads/writes the Machine Check Arch. (MCA)
	 * registers.
	 */
	uint64_t (*read_write_mca)(uint32_t ari_base,
			      uint64_t cmd,
			      uint64_t *data);
	/*
	 * Some MC GSC (General Security Carveout) register values are
	 * expected to be changed by TrustZone secure ARM code after boot.
	 * Since there is no hardware mechanism for the CCPLEX to know
	 * that an MC GSC register has changed to allow it to update its
	 * own internal GSC register, there needs to be a mechanism that
	 * can be used by ARM code to cause the CCPLEX to update its GSC
	 * register value. This ARI request allows updating the GSC register
	 * value for a certain carveout in the CCPLEX.
	 */
	int32_t (*update_ccplex_gsc)(uint32_t ari_base, uint32_t gsc_idx);
	/*
	 * This ARI request instructs the CCPLEX to either shutdown or
	 * reset the entire system
	 */
	void (*enter_ccplex_state)(uint32_t ari_base, uint32_t state_idx);
	/*
	 * This ARI request reads/writes data from/to Uncore PERFMON
	 * registers
	 */
	int32_t (*read_write_uncore_perfmon)(uint32_t ari_base,
			uint64_t req, uint64_t *data);
	/*
	 * This ARI implements ARI_MISC_CCPLEX commands. This can be
	 * used to enable/disable coresight clock gating.
	 */
	void (*misc_ccplex)(uint32_t ari_base, uint32_t index,
			uint32_t value);
} arch_mce_ops_t;

/* declarations for ARI/NVG handler functions */
int32_t ari_enter_cstate(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int32_t ari_update_cstate_info(uint32_t ari_base, uint32_t cluster, uint32_t ccplex,
	uint32_t system, uint8_t sys_state_force, uint32_t wake_mask,
	uint8_t update_wake_mask);
int32_t ari_update_crossover_time(uint32_t ari_base, uint32_t type, uint32_t time);
uint64_t ari_read_cstate_stats(uint32_t ari_base, uint32_t state);
int32_t ari_write_cstate_stats(uint32_t ari_base, uint32_t state, uint32_t stats);
uint64_t ari_enumeration_misc(uint32_t ari_base, uint32_t cmd, uint32_t data);
int32_t ari_is_ccx_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int32_t ari_is_sc7_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int32_t ari_online_core(uint32_t ari_base, uint32_t core);
int32_t ari_cc3_ctrl(uint32_t ari_base, uint32_t freq, uint32_t volt, uint8_t enable);
int32_t ari_reset_vector_update(uint32_t ari_base);
int32_t ari_roc_flush_cache_trbits(uint32_t ari_base);
int32_t ari_roc_flush_cache(uint32_t ari_base);
int32_t ari_roc_clean_cache(uint32_t ari_base);
uint64_t ari_read_write_mca(uint32_t ari_base, uint64_t cmd, uint64_t *data);
int32_t ari_update_ccplex_gsc(uint32_t ari_base, uint32_t gsc_idx);
void ari_enter_ccplex_state(uint32_t ari_base, uint32_t state_idx);
int32_t ari_read_write_uncore_perfmon(uint32_t ari_base,
		uint64_t req, uint64_t *data);
void ari_misc_ccplex(uint32_t ari_base, uint32_t index, uint32_t value);

int32_t nvg_enter_cstate(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int32_t nvg_update_cstate_info(uint32_t ari_base, uint32_t cluster, uint32_t ccplex,
		uint32_t system, uint8_t sys_state_force, uint32_t wake_mask,
		uint8_t update_wake_mask);
int32_t nvg_update_crossover_time(uint32_t ari_base, uint32_t type, uint32_t time);
uint64_t nvg_read_cstate_stats(uint32_t ari_base, uint32_t state);
int32_t nvg_write_cstate_stats(uint32_t ari_base, uint32_t state, uint32_t stats);
int32_t nvg_is_ccx_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int32_t nvg_is_sc7_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time);
int32_t nvg_online_core(uint32_t ari_base, uint32_t core);
int32_t nvg_cc3_ctrl(uint32_t ari_base, uint32_t freq, uint32_t volt, uint8_t enable);

extern void nvg_set_request_data(uint64_t req, uint64_t data);
extern void nvg_set_request(uint64_t req);
extern uint64_t nvg_get_result(void);
#endif /* MCE_PRIVATE_H */
