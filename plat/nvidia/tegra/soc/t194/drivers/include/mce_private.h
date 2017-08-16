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
#define CLUSTER_CSTATE_MASK				0x7UL
#define CLUSTER_CSTATE_SHIFT			0X0UL
#define CLUSTER_CSTATE_UPDATE_BIT		(1UL << 7)
#define CCPLEX_CSTATE_MASK				0x3UL
#define CCPLEX_CSTATE_SHIFT				8UL
#define CCPLEX_CSTATE_UPDATE_BIT		(1UL << 15)
#define SYSTEM_CSTATE_MASK				0xFUL
#define SYSTEM_CSTATE_SHIFT				16UL
#define SYSTEM_CSTATE_UPDATE_BIT		(1UL << 23)
#define CSTATE_WAKE_MASK_UPDATE_BIT		(1UL << 31)
#define CSTATE_WAKE_MASK_SHIFT			32UL
#define CSTATE_WAKE_MASK_CLEAR			0xFFFFFFFFUL

/*******************************************************************************
 * Auto-CC3 control macros
 ******************************************************************************/
#define MCE_AUTO_CC3_FREQ_MASK			0xFFUL
#define MCE_AUTO_CC3_FREQ_SHIFT			0UL
#define MCE_AUTO_CC3_ENABLE_BIT			(1UL << 31)

/*******************************************************************************
 * Core ID mask (bits 3:0 in the online request)
 ******************************************************************************/
#define MCE_CORE_ID_MASK				0xFUL

/*******************************************************************************
 * Cache control macros
 ******************************************************************************/
#define CACHE_CLEAN_SET					(1UL << 0)
#define CACHE_CLEAN_INVAL_SET			(1UL << 1)
#define CACHE_CLEAN_INVAL_TR_SET		(1UL << 2)

/*******************************************************************************
 * C-state statistics macros
 ******************************************************************************/
#define MCE_STAT_ID_SHIFT			16UL

/* declarations for NVG handler functions */
uint64_t nvg_get_version(void);
int32_t nvg_enable_power_perf_mode(void);
int32_t nvg_disable_power_perf_mode(void);
int32_t nvg_enable_power_saver_modes(void);
int32_t nvg_disable_power_saver_modes(void);
void nvg_set_wake_time(uint32_t wake_time);
void nvg_update_cstate_info(uint32_t cluster, uint32_t ccplex,
		uint32_t system, uint32_t wake_mask, uint8_t update_wake_mask);
int32_t nvg_update_crossover_time(uint32_t type, uint32_t time);
int32_t nvg_set_cstate_stat_query_value(uint64_t data);
uint64_t nvg_get_cstate_stat_query_value(void);
int32_t nvg_is_sc7_allowed(void);
int32_t nvg_online_core(uint32_t core);
int32_t nvg_cc3_ctrl(uint32_t freq, uint8_t enable);
int32_t nvg_update_ccplex_gsc(uint32_t gsc_idx);
int32_t nvg_roc_flush_cache(void);
int32_t nvg_roc_clean_cache(void);
int32_t nvg_roc_clean_cache_trbits(void);
int32_t nvg_enter_cstate(uint32_t state, uint32_t wake_time);

void nvg_set_request_data(uint64_t req, uint64_t data);
void nvg_set_request(uint64_t req);
uint64_t nvg_get_result(void);
uint64_t nvg_cache_clean(void);
uint64_t nvg_cache_clean_inval(void);
uint64_t nvg_cache_inval_all(void);

#endif /* __MCE_PRIVATE_H__ */
