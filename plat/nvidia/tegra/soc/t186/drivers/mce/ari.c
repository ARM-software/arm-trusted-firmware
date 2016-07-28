/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <denver.h>
#include <mmio.h>
#include <mce.h>
#include <sys/errno.h>
#include <t18x_ari.h>

/*******************************************************************************
 * Register offsets for ARI request/results
 ******************************************************************************/
#define ARI_REQUEST			0x0
#define ARI_REQUEST_EVENT_MASK		0x4
#define ARI_STATUS			0x8
#define ARI_REQUEST_DATA_LO		0xC
#define ARI_REQUEST_DATA_HI		0x10
#define ARI_RESPONSE_DATA_LO		0x14
#define ARI_RESPONSE_DATA_HI		0x18

/* Status values for the current request */
#define ARI_REQ_PENDING			1
#define ARI_REQ_ONGOING			3
#define ARI_REQUEST_VALID_BIT		(1 << 8)
#define ARI_EVT_MASK_STANDBYWFI_BIT	(1 << 7)

/*******************************************************************************
 * ARI helper functions
 ******************************************************************************/
static inline uint32_t ari_read_32(uint32_t ari_base, uint32_t reg)
{
	return mmio_read_32(ari_base + reg);
}

static inline void ari_write_32(uint32_t ari_base, uint32_t val, uint32_t reg)
{
	mmio_write_32(ari_base + reg, val);
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

static int ari_request_wait(uint32_t ari_base, uint32_t evt_mask, uint32_t req,
		uint32_t lo, uint32_t hi)
{
	int status;

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
	if (evt_mask)
		return 0;

	/* NOTE: add timeout check if needed */
	status = ari_read_32(ari_base, ARI_STATUS);
	while (status & (ARI_REQ_ONGOING | ARI_REQ_PENDING))
		status = ari_read_32(ari_base, ARI_STATUS);

	return 0;
}

int ari_enter_cstate(uint32_t ari_base, uint32_t state, uint32_t wake_time)
{
	/* check for allowed power state */
	if (state != TEGRA_ARI_CORE_C0 && state != TEGRA_ARI_CORE_C1 &&
	    state != TEGRA_ARI_CORE_C6 && state != TEGRA_ARI_CORE_C7) {
		ERROR("%s: unknown cstate (%d)\n", __func__, state);
		return EINVAL;
	}

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/* Enter the cstate, to be woken up after wake_time (TSC ticks) */
	return ari_request_wait(ari_base, ARI_EVT_MASK_STANDBYWFI_BIT,
		TEGRA_ARI_ENTER_CSTATE, state, wake_time);
}

int ari_update_cstate_info(uint32_t ari_base, uint32_t cluster, uint32_t ccplex,
	uint32_t system, uint8_t sys_state_force, uint32_t wake_mask,
	uint8_t update_wake_mask)
{
	uint32_t val = 0;

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/* update CLUSTER_CSTATE? */
	if (cluster)
		val |= (cluster & CLUSTER_CSTATE_MASK) |
			CLUSTER_CSTATE_UPDATE_BIT;

	/* update CCPLEX_CSTATE? */
	if (ccplex)
		val |= (ccplex & CCPLEX_CSTATE_MASK) << CCPLEX_CSTATE_SHIFT |
			CCPLEX_CSTATE_UPDATE_BIT;

	/* update SYSTEM_CSTATE? */
	if (system)
		val |= ((system & SYSTEM_CSTATE_MASK) << SYSTEM_CSTATE_SHIFT) |
		       ((sys_state_force << SYSTEM_CSTATE_FORCE_UPDATE_SHIFT) |
			SYSTEM_CSTATE_UPDATE_BIT);

	/* update wake mask value? */
	if (update_wake_mask)
		val |= CSTATE_WAKE_MASK_UPDATE_BIT;

	/* set the updated cstate info */
	return ari_request_wait(ari_base, 0, TEGRA_ARI_UPDATE_CSTATE_INFO, val,
			wake_mask);
}

int ari_update_crossover_time(uint32_t ari_base, uint32_t type, uint32_t time)
{
	/* sanity check crossover type */
	if ((type == TEGRA_ARI_CROSSOVER_C1_C6) ||
	    (type > TEGRA_ARI_CROSSOVER_CCP3_SC1))
		return EINVAL;

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/* update crossover threshold time */
	return ari_request_wait(ari_base, 0, TEGRA_ARI_UPDATE_CROSSOVER,
			type, time);
}

uint64_t ari_read_cstate_stats(uint32_t ari_base, uint32_t state)
{
	int ret;

	/* sanity check crossover type */
	if (state == 0)
		return EINVAL;

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	ret = ari_request_wait(ari_base, 0, TEGRA_ARI_CSTATE_STATS, state, 0);
	if (ret != 0)
		return EINVAL;

	return (uint64_t)ari_get_response_low(ari_base);
}

int ari_write_cstate_stats(uint32_t ari_base, uint32_t state, uint32_t stats)
{
	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/* write the cstate stats */
	return ari_request_wait(ari_base, 0, TEGRA_ARI_WRITE_CSTATE_STATS, state,
			stats);
}

uint64_t ari_enumeration_misc(uint32_t ari_base, uint32_t cmd, uint32_t data)
{
	uint64_t resp;
	int ret;

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/* ARI_REQUEST_DATA_HI is reserved for commands other than 'ECHO' */
	if (cmd != TEGRA_ARI_MISC_ECHO)
		data = 0;

	ret = ari_request_wait(ari_base, 0, TEGRA_ARI_MISC, cmd, data);
	if (ret)
		return (uint64_t)ret;

	/* get the command response */
	resp = ari_get_response_low(ari_base);
	resp |= ((uint64_t)ari_get_response_high(ari_base) << 32);

	return resp;
}

int ari_is_ccx_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time)
{
	int ret;

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	ret = ari_request_wait(ari_base, 0, TEGRA_ARI_IS_CCX_ALLOWED, state & 0x7,
			wake_time);
	if (ret) {
		ERROR("%s: failed (%d)\n", __func__, ret);
		return 0;
	}

	/* 1 = CCx allowed, 0 = CCx not allowed */
	return (ari_get_response_low(ari_base) & 0x1);
}

int ari_is_sc7_allowed(uint32_t ari_base, uint32_t state, uint32_t wake_time)
{
	int ret;

	/* check for allowed power state */
	if (state != TEGRA_ARI_CORE_C0 && state != TEGRA_ARI_CORE_C1 &&
	    state != TEGRA_ARI_CORE_C6 && state != TEGRA_ARI_CORE_C7) {
		ERROR("%s: unknown cstate (%d)\n", __func__, state);
		return EINVAL;
	}

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	ret = ari_request_wait(ari_base, 0, TEGRA_ARI_IS_SC7_ALLOWED, state,
			wake_time);
	if (ret) {
		ERROR("%s: failed (%d)\n", __func__, ret);
		return 0;
	}

	/* 1 = SC7 allowed, 0 = SC7 not allowed */
	return !!ari_get_response_low(ari_base);
}

int ari_online_core(uint32_t ari_base, uint32_t core)
{
	int cpu = read_mpidr() & MPIDR_CPU_MASK;
	int cluster = (read_mpidr() & MPIDR_CLUSTER_MASK) >>
			MPIDR_AFFINITY_BITS;
	int impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;

	/* construct the current CPU # */
	cpu |= (cluster << 2);

	/* sanity check target core id */
	if ((core >= MCE_CORE_ID_MAX) || (cpu == core)) {
		ERROR("%s: unsupported core id (%d)\n", __func__, core);
		return EINVAL;
	}

	/*
	 * The Denver cluster has 2 CPUs only - 0, 1.
	 */
	if (impl == DENVER_IMPL && ((core == 2) || (core == 3))) {
		ERROR("%s: unknown core id (%d)\n", __func__, core);
		return EINVAL;
	}

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	return ari_request_wait(ari_base, 0, TEGRA_ARI_ONLINE_CORE, core, 0);
}

int ari_cc3_ctrl(uint32_t ari_base, uint32_t freq, uint32_t volt, uint8_t enable)
{
	int val;

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
		(enable ? MCE_AUTO_CC3_ENABLE_BIT : 0));

	return ari_request_wait(ari_base, 0, TEGRA_ARI_CC3_CTRL, val, 0);
}

int ari_reset_vector_update(uint32_t ari_base, uint32_t lo, uint32_t hi)
{
	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/*
	 * Need to program the CPU reset vector one time during cold boot
	 * and SC7 exit
	 */
	ari_request_wait(ari_base, 0, TEGRA_ARI_COPY_MISCREG_AA64_RST, lo, hi);

	return 0;
}

int ari_roc_flush_cache_trbits(uint32_t ari_base)
{
	/* clean the previous response state */
	ari_clobber_response(ari_base);

	return ari_request_wait(ari_base, 0, TEGRA_ARI_ROC_FLUSH_CACHE_TRBITS,
			0, 0);
}

int ari_roc_flush_cache(uint32_t ari_base)
{
	/* clean the previous response state */
	ari_clobber_response(ari_base);

	return ari_request_wait(ari_base, 0, TEGRA_ARI_ROC_FLUSH_CACHE_ONLY,
			0, 0);
}

int ari_roc_clean_cache(uint32_t ari_base)
{
	/* clean the previous response state */
	ari_clobber_response(ari_base);

	return ari_request_wait(ari_base, 0, TEGRA_ARI_ROC_CLEAN_CACHE_ONLY,
			0, 0);
}

uint64_t ari_read_write_mca(uint32_t ari_base, mca_cmd_t cmd, uint64_t *data)
{
	mca_arg_t mca_arg;
	int ret;

	/* Set data (write) */
	mca_arg.data = data ? *data : 0ull;

	/* Set command */
	ari_write_32(ari_base, cmd.input.low, ARI_RESPONSE_DATA_LO);
	ari_write_32(ari_base, cmd.input.high, ARI_RESPONSE_DATA_HI);

	ret = ari_request_wait(ari_base, 0, TEGRA_ARI_MCA, mca_arg.arg.low,
			mca_arg.arg.high);
	if (!ret) {
		mca_arg.arg.low = ari_get_response_low(ari_base);
		mca_arg.arg.high = ari_get_response_high(ari_base);
		if (!mca_arg.err.finish)
			return (uint64_t)mca_arg.err.error;

		if (data) {
			mca_arg.arg.low = ari_get_request_low(ari_base);
			mca_arg.arg.high = ari_get_request_high(ari_base);
			*data = mca_arg.data;
		}
	}

	return 0;
}

int ari_update_ccplex_gsc(uint32_t ari_base, uint32_t gsc_idx)
{
	/* sanity check GSC ID */
	if (gsc_idx > TEGRA_ARI_GSC_VPR_IDX)
		return EINVAL;

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/*
	 * The MCE code will read the GSC carveout value, corrseponding to
	 * the ID, from the MC registers and update the internal GSC registers
	 * of the CCPLEX.
	 */
	ari_request_wait(ari_base, 0, TEGRA_ARI_UPDATE_CCPLEX_GSC, gsc_idx, 0);

	return 0;
}

void ari_enter_ccplex_state(uint32_t ari_base, uint32_t state_idx)
{
	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/*
	 * The MCE will shutdown or restart the entire system
	 */
	(void)ari_request_wait(ari_base, 0, TEGRA_ARI_MISC_CCPLEX, state_idx, 0);
}

int ari_read_write_uncore_perfmon(uint32_t ari_base,
		uncore_perfmon_req_t req, uint64_t *data)
{
	int ret;
	uint32_t val;

	/* clean the previous response state */
	ari_clobber_response(ari_base);

	/* sanity check input parameters */
	if (req.perfmon_command.cmd == UNCORE_PERFMON_CMD_READ && !data) {
		ERROR("invalid parameters\n");
		return EINVAL;
	}

	/*
	 * For "write" commands get the value that has to be written
	 * to the uncore perfmon registers
	 */
	val = (req.perfmon_command.cmd == UNCORE_PERFMON_CMD_WRITE) ?
		*data : 0;

	ret = ari_request_wait(ari_base, 0, TEGRA_ARI_PERFMON, val, req.data);
	if (ret)
		return ret;

	/* read the command status value */
	req.perfmon_status.val = ari_get_response_high(ari_base) &
				 UNCORE_PERFMON_RESP_STATUS_MASK;

	/*
	 * For "read" commands get the data from the uncore
	 * perfmon registers
	 */
	if ((req.perfmon_status.val == 0) && (req.perfmon_command.cmd ==
	     UNCORE_PERFMON_CMD_READ))
		*data = ari_get_response_low(ari_base);

	return (int)req.perfmon_status.val;
}
