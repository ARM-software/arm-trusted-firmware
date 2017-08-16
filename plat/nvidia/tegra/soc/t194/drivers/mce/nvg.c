/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <denver.h>
#include <errno.h>
#include <lib/mmio.h>
#include <mce_private.h>
#include <platform_def.h>
#include <t194_nvg.h>
#include <tegra_private.h>

#define	ID_AFR0_EL1_CACHE_OPS_SHIFT	12
#define	ID_AFR0_EL1_CACHE_OPS_MASK	0xFU
/*
 * Reports the major and minor version of this interface.
 *
 * NVGDATA[0:31]: SW(R) Minor Version
 * NVGDATA[32:63]: SW(R) Major Version
 */
uint64_t nvg_get_version(void)
{
	nvg_set_request(TEGRA_NVG_CHANNEL_VERSION);

	return (uint64_t)nvg_get_result();
}

/*
 * Enable the perf per watt mode.
 *
 * NVGDATA[0]: SW(RW), 1 = enable perf per watt mode
 */
int32_t nvg_enable_power_perf_mode(void)
{
	nvg_set_request_data(TEGRA_NVG_CHANNEL_POWER_PERF, 1U);

	return 0;
}

/*
 * Disable the perf per watt mode.
 *
 * NVGDATA[0]: SW(RW), 0 = disable perf per watt mode
 */
int32_t nvg_disable_power_perf_mode(void)
{
	nvg_set_request_data(TEGRA_NVG_CHANNEL_POWER_PERF, 0U);

	return 0;
}

/*
 * Enable the battery saver mode.
 *
 * NVGDATA[2]: SW(RW), 1 = enable battery saver mode
 */
int32_t nvg_enable_power_saver_modes(void)
{
	nvg_set_request_data(TEGRA_NVG_CHANNEL_POWER_MODES, 1U);

	return 0;
}

/*
 * Disable the battery saver mode.
 *
 * NVGDATA[2]: SW(RW), 0 = disable battery saver mode
 */
int32_t nvg_disable_power_saver_modes(void)
{
	nvg_set_request_data(TEGRA_NVG_CHANNEL_POWER_MODES, 0U);

	return 0;
}

/*
 * Set the expected wake time in TSC ticks for the next low-power state the
 * core enters.
 *
 * NVGDATA[0:31]: SW(RW), WAKE_TIME
 */
void nvg_set_wake_time(uint32_t wake_time)
{
	/* time (TSC ticks) until the core is expected to get a wake event */
	nvg_set_request_data(TEGRA_NVG_CHANNEL_WAKE_TIME, (uint64_t)wake_time);
}

/*
 * This request allows updating of CLUSTER_CSTATE, CCPLEX_CSTATE and
 * SYSTEM_CSTATE values.
 *
 * NVGDATA[0:2]: SW(RW), CLUSTER_CSTATE
 * NVGDATA[7]: SW(W), update cluster flag
 * NVGDATA[8:9]: SW(RW), CG_CSTATE
 * NVGDATA[15]: SW(W), update ccplex flag
 * NVGDATA[16:19]: SW(RW), SYSTEM_CSTATE
 * NVGDATA[23]: SW(W), update system flag
 * NVGDATA[31]: SW(W), update wake mask flag
 * NVGDATA[32:63]: SW(RW), WAKE_MASK
 */
void nvg_update_cstate_info(uint32_t cluster, uint32_t ccplex,
		uint32_t system, uint32_t wake_mask, uint8_t update_wake_mask)
{
	uint64_t val = 0;

	/* update CLUSTER_CSTATE? */
	if (cluster != 0U) {
		val |= ((uint64_t)cluster & CLUSTER_CSTATE_MASK) |
				CLUSTER_CSTATE_UPDATE_BIT;
	}

	/* update CCPLEX_CSTATE? */
	if (ccplex != 0U) {
		val |= (((uint64_t)ccplex & CCPLEX_CSTATE_MASK) << CCPLEX_CSTATE_SHIFT) |
				CCPLEX_CSTATE_UPDATE_BIT;
	}

	/* update SYSTEM_CSTATE? */
	if (system != 0U) {
		val |= (((uint64_t)system & SYSTEM_CSTATE_MASK) << SYSTEM_CSTATE_SHIFT) |
				SYSTEM_CSTATE_UPDATE_BIT;
	}

	/* update wake mask value? */
	if (update_wake_mask != 0U) {
		val |= CSTATE_WAKE_MASK_UPDATE_BIT;
	}

	/* set the wake mask */
	val |= ((uint64_t)wake_mask & CSTATE_WAKE_MASK_CLEAR) << CSTATE_WAKE_MASK_SHIFT;

	/* set the updated cstate info */
	nvg_set_request_data(TEGRA_NVG_CHANNEL_CSTATE_INFO, val);
}

/*
 * Indices gives MTS the crossover point in TSC ticks for when it becomes
 * no longer viable to enter the named state
 *
 * Type 0 : NVGDATA[0:31]: C6 Lower bound
 * Type 1 : NVGDATA[0:31]: CC6 Lower bound
 * Type 2 : NVGDATA[0:31]: CG7 Lower bound
 */
int32_t nvg_update_crossover_time(uint32_t type, uint32_t time)
{
	int32_t ret = 0;

	switch (type) {
	case TEGRA_NVG_CROSSOVER_C6:
		nvg_set_request_data(TEGRA_NVG_CHANNEL_CROSSOVER_C6_LOWER_BOUND,
			(uint64_t)time);
		break;

	case TEGRA_NVG_CROSSOVER_CC6:
		nvg_set_request_data(TEGRA_NVG_CHANNEL_CROSSOVER_CC6_LOWER_BOUND,
			(uint64_t)time);
		break;

	case TEGRA_NVG_CROSSOVER_CG7:
		nvg_set_request_data(TEGRA_NVG_CHANNEL_CROSSOVER_CG7_LOWER_BOUND,
			(uint64_t)time);
		break;

	default:
		ERROR("%s: unknown crossover type (%d)\n", __func__, type);
		ret = EINVAL;
		break;
	}

	return ret;
}

/*
 * These NVG calls allow ARM SW to access CSTATE statistical information
 *
 * NVGDATA[0:3]: SW(RW) Core/cluster/cg id
 * NVGDATA[16:31]: SW(RW) Stat id
 */
int32_t nvg_set_cstate_stat_query_value(uint64_t data)
{
	int32_t ret = 0;

	/* sanity check stat id and core id*/
	if ((data >> MCE_STAT_ID_SHIFT) >
		(uint64_t)NVG_STAT_QUERY_C7_RESIDENCY_SUM) {
		ERROR("%s: unknown stat id (%d)\n", __func__,
			(uint32_t)(data >> MCE_STAT_ID_SHIFT));
		ret = EINVAL;
	} else if ((data & MCE_CORE_ID_MASK) > (uint64_t)PLATFORM_CORE_COUNT) {
		ERROR("%s: unknown core id (%d)\n", __func__,
			(uint32_t)(data & MCE_CORE_ID_MASK));
		ret = EINVAL;
	} else {
		nvg_set_request_data(TEGRA_NVG_CHANNEL_CSTATE_STAT_QUERY_REQUEST, data);
	}

	return ret;
}

/*
 * The read-only value associated with the CSTATE_STAT_QUERY_REQUEST
 *
 * NVGDATA[0:63]: SW(R) Stat count
 */
uint64_t nvg_get_cstate_stat_query_value(void)
{
	nvg_set_request(TEGRA_NVG_CHANNEL_CSTATE_STAT_QUERY_VALUE);

	return (uint64_t)nvg_get_result();
}

/*
 * Return a non-zero value if the CCPLEX is able to enter SC7
 *
 * NVGDATA[0]: SW(R), Is allowed result
 */
int32_t nvg_is_sc7_allowed(void)
{
	/* issue command to check if SC7 is allowed */
	nvg_set_request(TEGRA_NVG_CHANNEL_IS_SC7_ALLOWED);

	/* 1 = SC7 allowed, 0 = SC7 not allowed */
	return (int32_t)nvg_get_result();
}

/*
 * Wake an offlined logical core. Note that a core is offlined by entering
 * a C-state where the WAKE_MASK is all 0.
 *
 * NVGDATA[0:3]: SW(W) logical core to online
 */
int32_t nvg_online_core(uint32_t core)
{
	int32_t ret = 0;

	/* sanity check the core ID value */
	if (core > (uint32_t)PLATFORM_CORE_COUNT) {
		ERROR("%s: unknown core id (%d)\n", __func__, core);
		ret = EINVAL;
	} else {
		/* get a core online */
		nvg_set_request_data(TEGRA_NVG_CHANNEL_ONLINE_CORE,
								(uint64_t)core & MCE_CORE_ID_MASK);
	}

	return ret;
}

/*
 * Enables and controls the voltage/frequency hint for CC3. CC3 is disabled
 * by default.
 *
 * NVGDATA[7:0] SW(RW) frequency request
 * NVGDATA[31:31] SW(RW) enable bit
 */
int32_t nvg_cc3_ctrl(uint32_t freq, uint8_t enable)
{
	uint64_t val = 0;

	/*
	 * If the enable bit is cleared, Auto-CC3 will be disabled by setting
	 * the SW visible frequency request registers for all non
	 * floorswept cores valid independent of StandbyWFI and disabling
	 * the IDLE frequency request register. If set, Auto-CC3
	 * will be enabled by setting the ARM SW visible frequency
	 * request registers for all non floorswept cores to be enabled by
	 * StandbyWFI or the equivalent signal, and always keeping the IDLE
	 * frequency request register enabled.
	 */
	if (enable != 0U) {
		val = ((uint64_t)freq & MCE_AUTO_CC3_FREQ_MASK) | MCE_AUTO_CC3_ENABLE_BIT;
	}
	nvg_set_request_data(TEGRA_NVG_CHANNEL_CC3_CTRL, val);

	return 0;
}

/*
 * MC GSC (General Security Carveout) register values are expected to be
 * changed by TrustZone ARM code after boot.
 *
 * NVGDATA[0:15] SW(R) GSC enun
 */
int32_t nvg_update_ccplex_gsc(uint32_t gsc_idx)
{
	int32_t ret = 0;

	/* sanity check GSC ID */
	if (gsc_idx > (uint32_t)TEGRA_NVG_GSC_VPR_IDX) {
		ERROR("%s: unknown gsc_idx (%d)\n", __func__, gsc_idx);
		ret = EINVAL;
	} else {
		nvg_set_request_data(TEGRA_NVG_CHANNEL_UPDATE_CCPLEX_GSC,
								(uint64_t)gsc_idx);
	}

	return ret;
}

/*
 * Cache clean operation for all CCPLEX caches.
 */
int32_t nvg_roc_clean_cache(void)
{
	int32_t ret = 0;

	/* check if cache flush through mts is supported */
	if (((read_id_afr0_el1() >> ID_AFR0_EL1_CACHE_OPS_SHIFT) &
			ID_AFR0_EL1_CACHE_OPS_MASK) == 1U) {
		if (nvg_cache_clean() == 0U) {
			ERROR("%s: failed\n", __func__);
			ret = EINVAL;
		}
	} else {
		ret = EINVAL;
	}
	return ret;
}

/*
 * Cache clean and invalidate operation for all CCPLEX caches.
 */
int32_t nvg_roc_flush_cache(void)
{
	int32_t ret = 0;

	/* check if cache flush through mts is supported */
	if (((read_id_afr0_el1() >> ID_AFR0_EL1_CACHE_OPS_SHIFT) &
			ID_AFR0_EL1_CACHE_OPS_MASK) == 1U) {
		if (nvg_cache_clean_inval() == 0U) {
			ERROR("%s: failed\n", __func__);
			ret = EINVAL;
		}
	} else {
		ret = EINVAL;
	}
	return ret;
}

/*
 * Cache clean and invalidate, clear TR-bit operation for all CCPLEX caches.
 */
int32_t nvg_roc_clean_cache_trbits(void)
{
	int32_t ret = 0;

	/* check if cache flush through mts is supported */
	if (((read_id_afr0_el1() >> ID_AFR0_EL1_CACHE_OPS_SHIFT) &
			ID_AFR0_EL1_CACHE_OPS_MASK) == 1U) {
		if (nvg_cache_inval_all() == 0U) {
			ERROR("%s: failed\n", __func__);
			ret = EINVAL;
		}
	} else {
		ret = EINVAL;
	}
	return ret;
}

/*
 * Set the power state for a core
 */
int32_t nvg_enter_cstate(uint32_t state, uint32_t wake_time)
{
	int32_t ret = 0;
	uint64_t val = 0ULL;

	/* check for allowed power state */
	if ((state != (uint32_t)TEGRA_NVG_CORE_C0) &&
		(state != (uint32_t)TEGRA_NVG_CORE_C1) &&
	    (state != (uint32_t)TEGRA_NVG_CORE_C6) &&
		(state != (uint32_t)TEGRA_NVG_CORE_C7))
	{
		ERROR("%s: unknown cstate (%d)\n", __func__, state);
		ret = EINVAL;
	} else {
		/* time (TSC ticks) until the core is expected to get a wake event */
		nvg_set_wake_time(wake_time);

		/* set the core cstate */
		val = read_actlr_el1() & ~ACTLR_EL1_PMSTATE_MASK;
		write_actlr_el1(val | (uint64_t)state);
	}

	return ret;
}
