/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <denver.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <mce_private.h>
#include <t18x_ari.h>

/*******************************************************************************
 * Register offsets for ARI request/results
 ******************************************************************************/
#define ARI_REQUEST			0x0U
#define ARI_REQUEST_EVENT_MASK		0x4U
#define ARI_STATUS			0x8U
#define ARI_REQUEST_DATA_LO		0xCU
#define ARI_REQUEST_DATA_HI		0x10U
#define ARI_RESPONSE_DATA_LO		0x14U
#define ARI_RESPONSE_DATA_HI		0x18U

/* Status values for the current request */
#define ARI_REQ_PENDING			1U
#define ARI_REQ_ONGOING			3U
#define ARI_REQUEST_VALID_BIT		(1U << 8)
#define ARI_EVT_MASK_STANDBYWFI_BIT	(1U << 7)

/* default timeout (us) to wait for ARI completion */
#define ARI_MAX_RETRY_COUNT		U(2000000)

/*******************************************************************************
 * ARI helper functions
 ******************************************************************************/
static inline uint32_t ari_read_32(uint32_t ari_base, uint32_t reg)
{
	return mmio_read_32((uint64_t)ari_base + (uint64_t)reg);
}

static inline void ari_write_32(uint32_t ari_base, uint32_t val, uint32_t reg)
{
	mmio_write_32((uint64_t)ari_base + (uint64_t)reg, val);
}

static inline uint32_t ari_get_request_low(uint32_t ari_base)
{
	return ari_read_32(ari_base, ARI_REQUEST_DATA_LO);
}

static inline uint32_t ari_get_request_high(uint32_t ari_base)
{
	return ari_read_32(ari_base, ARI_REQUEST_DATA_HI);
}

static inline uint32_t ari_get_response_low(uint32_t ari_base)
{
	return ari_read_32(ari_base, ARI_RESPONSE_DATA_LO);
}

static inline uint32_t ari_get_response_high(uint32_t ari_base)
{
	return ari_read_32(ari_base, ARI_RESPONSE_DATA_HI);
}

static inline void ari_clobber_response(uint32_t ari_base)
{
	ari_write_32(ari_base, 0, ARI_RESPONSE_DATA_LO);
	ari_write_32(ari_base, 0, ARI_RESPONSE_DATA_HI);
}

static int32_t ari_request_wait(uint32_t ari_base, uint32_t evt_mask, uint32_t req,
		uint32_t lo, uint32_t hi)
{
	uint32_t retries = (uint32_t)ARI_MAX_RETRY_COUNT;
	uint32_t status;
	int32_t ret = 0;

	/* program the request, event_mask, hi and lo registers */
	ari_write_32(ari_base, lo, ARI_REQUEST_DATA_LO);
	ari_write_32(ari_base, hi, ARI_REQUEST_DATA_HI);
	ari_write_32(ari_base, evt_mask, ARI_REQUEST_EVENT_MASK);
	ari_write_32(ari_base, req | ARI_REQUEST_VALID_BIT, ARI_REQUEST);

	/*
	 * For commands that have an event trigger, we should bypass
	 * ARI_STATUS polling, since MCE is waiting for SW to trigger
	 * the event.
	 */
	if (evt_mask != 0U) {
		ret = 0;
	} else {
		/* For shutdown/reboot commands, we dont have to check for timeouts */
		if ((req == TEGRA_ARI_MISC_CCPLEX) &&
		    ((lo == TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_POWER_OFF) ||
		     (lo == TEGRA_ARI_MISC_CCPLEX_SHUTDOWN_REBOOT))) {
				ret = 0;
		} else {
			/*
			 * Wait for the command response for not more than the timeout
			 */
			while (retries != 0U) {

				/* read the command status */
				status = ari_read_32(ari_base, ARI_STATUS);
				if ((status & (ARI_REQ_ONGOING | ARI_REQ_PENDING)) == 0U) {
					break;
				}

				/* delay 1 us */
				udelay(1);

				/* decrement the retry count */
				retries--;
			}

			/* assert if the command timed out */
			if (retries == 0U) {
				ERROR("ARI request timed out: req %d on CPU %d\n",
					req, plat_my_core_pos());
				assert(retries != 0U);
			}
		}
	}

	return ret;
}

int32_t ari_enter_cstate(uint32_t ari_base, uint32_t state, uint32_t wake_time)
{
	int32_t ret = 0;

	/* check for allowed power state */
	if ((state != TEGRA_ARI_CORE_C0) &&
	    (state != TEGRA_ARI_CORE_C1) &&
	    (state != TEGRA_ARI_CORE_C6) &&
	    (state != TEGRA_ARI_CORE_C7)) {
		ERROR("%s: unknown cstate (%d)\n", __func__, state);
		ret = EINVAL;
	} else {
		/* clean the previous response state */
		ari_clobber_response(ari_base);

		/* Enter the cstate, to be woken up after wake_time (TSC ticks) */
		ret = ari_request_wait(ari_base, ARI_EVT_MASK_STANDBYWFI_BIT,
			(uint32_t)TEGRA_ARI_ENTER_CSTATE, state, wake_time);
	}

	return ret;
}

int32_t ari_update_cstate_info(uint32_t ari_base, uint32_t cluster, uint32_t ccplex,
	uint32_t system, uint8_t sys_state_force, uint32_t wake_mask,
	uint8_t update_wake_mask)
{
	uint64_t val = 0U;

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/* update CLUSTER_CSTATE? */
	if (cluster != 0U) {
		val |= (cluster & CLUSTER_CSTATE_MASK) |
			CLUSTER_CSTATE_UPDATE_BIT;
	}

	/* update CCPLEX_CSTATE? */
	if (ccplex != 0U) {
		val |= ((ccplex & CCPLEX_CSTATE_MASK) << CCPLEX_CSTATE_SHIFT) |
			CCPLEX_CSTATE_UPDATE_BIT;
	}

	/* update SYSTEM_CSTATE? */
	if (system != 0U) {
		val |= ((system & SYSTEM_CSTATE_MASK) << SYSTEM_CSTATE_SHIFT) |
		       (((uint64_t)sys_state_force << SYSTEM_CSTATE_FORCE_UPDATE_SHIFT) |
			SYSTEM_CSTATE_UPDATE_BIT);
	}

	/* update wake mask value? */
	if (update_wake_mask != 0U) {
		val |= CSTATE_WAKE_MASK_UPDATE_BIT;
	}

	/* set the updated cstate info */
	return ari_request_wait(ari_base, 0U, (uint32_t)TEGRA_ARI_UPDATE_CSTATE_INFO,
				(uint32_t)val, wake_mask);
}

int32_t ari_update_crossover_time(uint32_t ari_base, uint32_t type, uint32_t time)
{
	int32_t ret = 0;

	/* sanity check crossover type */
	if ((type == TEGRA_ARI_CROSSOVER_C1_C6) ||
	    (type > TEGRA_ARI_CROSSOVER_CCP3_SC1)) {
		ret = EINVAL;
	} else {
		/* clean the previous response state */
		ari_clobber_response(ari_base);

		/* update crossover threshold time */
		ret = ari_request_wait(ari_base, 0U,
				(uint32_t)TEGRA_ARI_UPDATE_CROSSOVER, type, time);
	}

	return ret;
}

uint64_t ari_read_cstate_stats(uint32_t ari_base, uint32_t state)
{
	int32_t ret;
	uint64_t result;

	/* sanity check crossover type */
	if (state == 0U) {
		result = EINVAL;
	} else {
		/* clean the previous response state */
		ari_clobber_response(ari_base);

		ret = ari_request_wait(ari_base, 0U,
				(uint32_t)TEGRA_ARI_CSTATE_STATS, state, 0U);
		if (ret != 0) {
			result = EINVAL;
		} else {
			result = (uint64_t)ari_get_response_low(ari_base);
		}
	}
	return result;
}

int32_t ari_write_cstate_stats(uint32_t ari_base, uint32_t state, uint32_t stats)
{
	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/* write the cstate stats */
	return ari_request_wait(ari_base, 0U, (uint32_t)TEGRA_ARI_WRITE_CSTATE_STATS,
			state, stats);
}

uint64_t ari_enumeration_misc(uint32_t ari_base, uint32_t cmd, uint32_t data)
{
	uint64_t resp;
	int32_t ret;
	uint32_t local_data = data;

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/* ARI_REQUEST_DATA_HI is reserved for commands other than 'ECHO' */
	if (cmd != TEGRA_ARI_MISC_ECHO) {
		local_data = 0U;
	}

	ret = ari_request_wait(ari_base, 0U, (uint32_t)TEGRA_ARI_MISC, cmd, local_data);
	if (ret != 0) {
		resp = (uint64_t)ret;
	} else {
		/* get the command response */
		resp = ari_get_response_low(ari_base);
		resp |= ((uint64_t)ari_get_response_high(ari_base) << 32);
	}

	return resp;
}

int32_t ari_is_ccx_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time)
{
	int32_t ret;
	uint32_t result;

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	ret = ari_request_wait(ari_base, 0U, (uint32_t)TEGRA_ARI_IS_CCX_ALLOWED,
			state & 0x7U, wake_time);
	if (ret != 0) {
		ERROR("%s: failed (%d)\n", __func__, ret);
		result = 0U;
	} else {
		result = ari_get_response_low(ari_base) & 0x1U;
	}

	/* 1 = CCx allowed, 0 = CCx not allowed */
	return (int32_t)result;
}

int32_t ari_is_sc7_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time)
{
	int32_t ret, result;

	/* check for allowed power state */
	if ((state != TEGRA_ARI_CORE_C0) && (state != TEGRA_ARI_CORE_C1) &&
	    (state != TEGRA_ARI_CORE_C6) && (state != TEGRA_ARI_CORE_C7)) {
		ERROR("%s: unknown cstate (%d)\n", __func__, state);
		result = EINVAL;
	} else {
		/* clean the previous response state */
		ari_clobber_response(ari_base);

		ret = ari_request_wait(ari_base, 0U,
			(uint32_t)TEGRA_ARI_IS_SC7_ALLOWED, state, wake_time);
		if (ret != 0) {
			ERROR("%s: failed (%d)\n", __func__, ret);
			result = 0;
		} else {
			/* 1 = SC7 allowed, 0 = SC7 not allowed */
			result = (ari_get_response_low(ari_base) != 0U) ? 1 : 0;
		}
	}

	return result;
}

int32_t ari_online_core(uint32_t ari_base, uint32_t core)
{
	uint64_t cpu = read_mpidr() & (MPIDR_CPU_MASK);
	uint64_t cluster = (read_mpidr() & (MPIDR_CLUSTER_MASK)) >>
			   (MPIDR_AFFINITY_BITS);
	uint64_t impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;
	int32_t ret;

	/* construct the current CPU # */
	cpu |= (cluster << 2);

	/* sanity check target core id */
	if ((core >= MCE_CORE_ID_MAX) || (cpu == (uint64_t)core)) {
		ERROR("%s: unsupported core id (%d)\n", __func__, core);
		ret = EINVAL;
	} else {
		/*
		 * The Denver cluster has 2 CPUs only - 0, 1.
		 */
		if ((impl == DENVER_IMPL) && ((core == 2U) || (core == 3U))) {
			ERROR("%s: unknown core id (%d)\n", __func__, core);
			ret = EINVAL;
		} else {
			/* clean the previous response state */
			ari_clobber_response(ari_base);
			ret = ari_request_wait(ari_base, 0U,
				(uint32_t)TEGRA_ARI_ONLINE_CORE, core, 0U);
		}
	}

	return ret;
}

int32_t ari_cc3_ctrl(uint32_t ari_base, uint32_t freq, uint32_t volt, uint8_t enable)
{
	uint32_t val;

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/*
	 * If the enable bit is cleared, Auto-CC3 will be disabled by setting
	 * the SW visible voltage/frequency request registers for all non
	 * floorswept cores valid independent of StandbyWFI and disabling
	 * the IDLE voltage/frequency request register. If set, Auto-CC3
	 * will be enabled by setting the ARM SW visible voltage/frequency
	 * request registers for all non floorswept cores to be enabled by
	 * StandbyWFI or the equivalent signal, and always keeping the IDLE
	 * voltage/frequency request register enabled.
	 */
	val = (((freq & MCE_AUTO_CC3_FREQ_MASK) << MCE_AUTO_CC3_FREQ_SHIFT) |\
		((volt & MCE_AUTO_CC3_VTG_MASK) << MCE_AUTO_CC3_VTG_SHIFT) |\
		((enable != 0U) ? MCE_AUTO_CC3_ENABLE_BIT : 0U));

	return ari_request_wait(ari_base, 0U,
			(uint32_t)TEGRA_ARI_CC3_CTRL, val, 0U);
}

int32_t ari_reset_vector_update(uint32_t ari_base)
{
	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/*
	 * Need to program the CPU reset vector one time during cold boot
	 * and SC7 exit
	 */
	(void)ari_request_wait(ari_base, 0U,
			(uint32_t)TEGRA_ARI_COPY_MISCREG_AA64_RST, 0U, 0U);

	return 0;
}

int32_t ari_roc_flush_cache_trbits(uint32_t ari_base)
{
	/* clean the previous response state */
	ari_clobber_response(ari_base);

	return ari_request_wait(ari_base, 0U,
			(uint32_t)TEGRA_ARI_ROC_FLUSH_CACHE_TRBITS, 0U, 0U);
}

int32_t ari_roc_flush_cache(uint32_t ari_base)
{
	/* clean the previous response state */
	ari_clobber_response(ari_base);

	return ari_request_wait(ari_base, 0U,
			(uint32_t)TEGRA_ARI_ROC_FLUSH_CACHE_ONLY, 0U, 0U);
}

int32_t ari_roc_clean_cache(uint32_t ari_base)
{
	/* clean the previous response state */
	ari_clobber_response(ari_base);

	return ari_request_wait(ari_base, 0U,
			(uint32_t)TEGRA_ARI_ROC_CLEAN_CACHE_ONLY, 0U, 0U);
}

uint64_t ari_read_write_mca(uint32_t ari_base, uint64_t cmd, uint64_t *data)
{
	uint64_t mca_arg_data, result = 0;
	uint32_t resp_lo, resp_hi;
	uint32_t mca_arg_err, mca_arg_finish;
	int32_t ret;

	/* Set data (write) */
	mca_arg_data = (data != NULL) ? *data : 0ULL;

	/* Set command */
	ari_write_32(ari_base, (uint32_t)cmd, ARI_RESPONSE_DATA_LO);
	ari_write_32(ari_base, (uint32_t)(cmd >> 32U), ARI_RESPONSE_DATA_HI);

	ret = ari_request_wait(ari_base, 0U, (uint32_t)TEGRA_ARI_MCA,
			       (uint32_t)mca_arg_data,
			       (uint32_t)(mca_arg_data >> 32U));
	if (ret == 0) {
		resp_lo = ari_get_response_low(ari_base);
		resp_hi = ari_get_response_high(ari_base);

		mca_arg_err = resp_lo & MCA_ARG_ERROR_MASK;
		mca_arg_finish = (resp_hi >> MCA_ARG_FINISH_SHIFT) &
				 MCA_ARG_FINISH_MASK;

		if (mca_arg_finish == 0U) {
			result = (uint64_t)mca_arg_err;
		} else {
			if (data != NULL) {
				resp_lo = ari_get_request_low(ari_base);
				resp_hi = ari_get_request_high(ari_base);
				*data = ((uint64_t)resp_hi << 32U) |
					 (uint64_t)resp_lo;
			}
		}
	}

	return result;
}

int32_t ari_update_ccplex_gsc(uint32_t ari_base, uint32_t gsc_idx)
{
	int32_t ret = 0;
	/* sanity check GSC ID */
	if (gsc_idx > TEGRA_ARI_GSC_VPR_IDX) {
		ret = EINVAL;
	} else {
		/* clean the previous response state */
		ari_clobber_response(ari_base);

		/*
		 * The MCE code will read the GSC carveout value, corrseponding to
		 * the ID, from the MC registers and update the internal GSC registers
		 * of the CCPLEX.
		 */
		(void)ari_request_wait(ari_base, 0U,
				(uint32_t)TEGRA_ARI_UPDATE_CCPLEX_GSC, gsc_idx, 0U);
	}

	return ret;
}

void ari_enter_ccplex_state(uint32_t ari_base, uint32_t state_idx)
{
	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/*
	 * The MCE will shutdown or restart the entire system
	 */
	(void)ari_request_wait(ari_base, 0U,
			(uint32_t)TEGRA_ARI_MISC_CCPLEX, state_idx, 0U);
}

int32_t ari_read_write_uncore_perfmon(uint32_t ari_base, uint64_t req,
		uint64_t *data)
{
	int32_t ret, result;
	uint32_t val, req_status;
	uint8_t req_cmd;

	req_cmd = (uint8_t)(req & UNCORE_PERFMON_CMD_MASK);

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/* sanity check input parameters */
	if ((req_cmd == UNCORE_PERFMON_CMD_READ) && (data == NULL)) {
		ERROR("invalid parameters\n");
		result = EINVAL;
	} else {
		/*
		 * For "write" commands get the value that has to be written
		 * to the uncore perfmon registers
		 */
		val = (req_cmd == UNCORE_PERFMON_CMD_WRITE) ?
			(uint32_t)*data : 0U;

		ret = ari_request_wait(ari_base, 0U,
			(uint32_t)TEGRA_ARI_PERFMON, val, (uint32_t)req);
		if (ret != 0) {
			result = ret;
		} else {
			/* read the command status value */
			req_status = ari_get_response_high(ari_base) &
					 UNCORE_PERFMON_RESP_STATUS_MASK;

			/*
			 * For "read" commands get the data from the uncore
			 * perfmon registers
			 */
			req_status &= UNCORE_PERFMON_RESP_STATUS_MASK;
			if ((req_status == 0U) && (req_cmd == UNCORE_PERFMON_CMD_READ)) {
				*data = ari_get_response_low(ari_base);
			}
			result = (int32_t)req_status;
		}
	}

	return result;
}

void ari_misc_ccplex(uint32_t ari_base, uint32_t index, uint32_t value)
{
	/*
	 * This invokes the ARI_MISC_CCPLEX commands. This can be
	 * used to enable/disable coresight clock gating.
	 */

	if ((index > TEGRA_ARI_MISC_CCPLEX_EDBGREQ) ||
		((index == TEGRA_ARI_MISC_CCPLEX_CORESIGHT_CG_CTRL) &&
		(value > 1U))) {
		ERROR("%s: invalid parameters \n", __func__);
	} else {
		/* clean the previous response state */
		ari_clobber_response(ari_base);
		(void)ari_request_wait(ari_base, 0U,
			(uint32_t)TEGRA_ARI_MISC_CCPLEX, index, value);
	}
}
