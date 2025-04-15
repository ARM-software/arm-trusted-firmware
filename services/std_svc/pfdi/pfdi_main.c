/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <services/pfdi.h>
#include <services/pfdi_svc.h>

static struct plat_pfdi_func_desc plat_pfdi_func_desc;
static const struct plat_pfdi_func_desc *plat_pfdi_func_desc_ptr =
	&plat_pfdi_func_desc;

#ifndef PFDI_CPU_OFF_RETRY
/* 0 = wait forever; >0 = retry count (retries * PFDI_OFF_RETRY_US µs) */
#define PFDI_CPU_OFF_RETRY	U(10)
#endif
#ifndef PFDI_OFF_RETRY_US
#define PFDI_OFF_RETRY_US	U(10)
#endif
#ifndef PFDI_RESULT_TIMEOUT
#define PFDI_RESULT_TIMEOUT	U(1000000)
#endif

static volatile bool pfdi_oor_complete[PLATFORM_CORE_COUNT];
static bool pfdi_oor_done[PLATFORM_CORE_COUNT];

/**
 * PFDI Force error records
 */
typedef struct {
	bool enabled;
	uint32_t fid;
	int64_t error_id;
} force_err_inject_t;

/* Update number of FORCE_ERROR slots supported per core when new feature is added */
#define PFDI_FORCE_ERR_SLOTS_PER_CORE  8U
static force_err_inject_t error_state[PLATFORM_CORE_COUNT][PFDI_FORCE_ERR_SLOTS_PER_CORE];

void pfdi_register_plat_func_desc(const struct plat_pfdi_func_desc *desc)
{
	assert(desc != NULL);

	if (desc->name != NULL) {
		assert((plat_pfdi_func_desc.name == NULL) ||
				(plat_pfdi_func_desc.name == desc->name));
		plat_pfdi_func_desc.name = desc->name;
	}

	if (desc->force_plat_err != NULL) {
		assert((plat_pfdi_func_desc.force_plat_err == NULL) ||
			(plat_pfdi_func_desc.force_plat_err == desc->force_plat_err));
		plat_pfdi_func_desc.force_plat_err = desc->force_plat_err;
	}

	if (desc->check_plat_err != NULL) {
		assert((plat_pfdi_func_desc.check_plat_err == NULL) ||
			(plat_pfdi_func_desc.check_plat_err == desc->check_plat_err));
		plat_pfdi_func_desc.check_plat_err = desc->check_plat_err;
	}

	if (desc->post_run != NULL) {
		assert((plat_pfdi_func_desc.post_run == NULL) ||
			(plat_pfdi_func_desc.post_run == desc->post_run));
		plat_pfdi_func_desc.post_run = desc->post_run;
	}
}

const struct plat_pfdi_func_desc *pfdi_get_plat_func_desc(void)
{
	return plat_pfdi_func_desc_ptr;
}

static force_err_inject_t *pfdi_find_or_alloc_force_slot(uint32_t core, uint32_t fid)
{
	force_err_inject_t *free_slot = NULL;

	for (unsigned int i = 0; i < PFDI_FORCE_ERR_SLOTS_PER_CORE; i++) {
		force_err_inject_t *s = &error_state[core][i];

		/* If already have an entry for this fid, update it */
		if (s->enabled && s->fid == fid) {
			return s;
		}

		/* Track first free slot */
		if (!s->enabled && free_slot == NULL) {
			free_slot = s;
		}
	}

	return free_slot;
}

/*
 * Wait until the CPU is OFF.
 * Policy: return only on OFF; panic on PSCI error or bounded-timeout.
 */
static void wait_cpu_off(u_register_t mpidr, int cpu_num)
{
	unsigned int retries = PFDI_CPU_OFF_RETRY;
	int state = AFF_STATE_ON;

	for (;;) {
		state = psci_affinity_info(mpidr, MPIDR_AFFLVL0);

		if (state == AFF_STATE_OFF) {
			return;
		}
		if (state < 0) {
			ERROR("PFDI: CPU %d (mpidr=0x%lx) PSCI error %d)\n",
					cpu_num, mpidr, state);
			panic();
		}

		/* Optional timeout: only counts down if nonzero, Platform can
		 * set PFDI_CPU_OFF_RETRY == 0 for infinite wait
		 */
		if (retries && --retries == 0) {
			ERROR("PFDI: timeout waiting for Core %d to go OFF (state=%d)\n",
				cpu_num, state);
			panic();
		}

		udelay(PFDI_OFF_RETRY_US);
	}
}

void pfdi_init(void)
{
	pfdi_status_t pfdi_status;
	bool secondary_failure = false;

	assert(pfdi_func_desc.name != NULL);
	assert(pfdi_func_desc.run != NULL);
	assert(pfdi_func_desc.count != NULL);
	assert(pfdi_func_desc.result != NULL);

	NOTICE("PFDI: Initializing Platform Fault Detection Interface.\n");
	NOTICE("PFDI: Running OoR tests on primary core.\n");

	pfdi_status = pfdi_pe_oor_test_run();
	if (pfdi_status != 0) {
		ERROR("PFDI: OoR tests on primary core failed.\n");
		panic();
	} else {
		NOTICE("PFDI: OoR tests on primary core succeeded.\n");
	}

	NOTICE("PFDI: Running OoR tests on secondary cores.\n");

	for (unsigned int cpu_id = 0U; cpu_id < PLATFORM_CORE_COUNT; cpu_id++) {
		uint64_t ft_id = UINT64_MAX;
		unsigned int retry = 0U;
		int psci_ret;
		u_register_t mpidr;

		if (cpu_id == plat_my_core_pos()) {
			continue;
		}

		mpidr = plat_pfdi_mpidr_by_core_pos(cpu_id);
		if (mpidr == INVALID_MPID) {
			ERROR("PFDI: Invalid MPIDR for core position %u.\n", cpu_id);
			secondary_failure = true;
			continue;
		}

		/*
		 * PSCI CPU_ON requires a valid NS entrypoint, even though the
		 * secondary is expected to run OoR PFDI from EL3 and power
		 * itself back off from pfdi_enable() before any NS handoff.
		 */
		psci_ret = psci_cpu_on_by_core_pos(cpu_id, mpidr,
						   plat_get_ns_image_entrypoint(),
						   0U);
		if (psci_ret != PSCI_E_SUCCESS) {
			ERROR("PFDI: Failed to turn on core %u.\n", cpu_id);
			secondary_failure = true;
			continue;
		}

		do {
			retry++;
			udelay(PFDI_OFF_RETRY_US);
		} while (!pfdi_oor_complete[cpu_id] &&
			(retry < PFDI_RESULT_TIMEOUT));

		if (!pfdi_oor_complete[cpu_id]) {
			ERROR("PFDI: OoR tests on core %u timed out.\n", cpu_id);
			secondary_failure = true;
		} else {
			dmbish();
			pfdi_status = pfdi_func_desc.result(cpu_id, &ft_id);

			if (pfdi_status == PFDI_RET_NOT_RUN)
				INFO("PFDI: OoR tests on core %u skipped; no test parts.\n",
					cpu_id);
			else if (pfdi_status != 0) {
				ERROR("PFDI: OoR tests on core %u failed at test %" PRIu64 ".\n",
					cpu_id, ft_id);
				secondary_failure = true;
			} else {
				INFO("PFDI: OoR tests on core %u succeeded.\n", cpu_id);
			}
		}
		wait_cpu_off(mpidr, cpu_id);
	}

	if (secondary_failure) {
		ERROR("PFDI: One or more secondary-core OoR tests failed.\n");
	} else {
		NOTICE("PFDI: All OoR tests completed successfully.\n");
	}
}

int64_t pfdi_consume_force_error(uint32_t fid)
{
	uint32_t core = plat_my_core_pos();
	const struct plat_pfdi_func_desc *plat_desc = pfdi_get_plat_func_desc();

	for (unsigned int i = 0; i < PFDI_FORCE_ERR_SLOTS_PER_CORE; i++) {
		force_err_inject_t *state = &error_state[core][i];

		if (state->enabled && state->fid == fid) {
			int64_t err = state->error_id;

			state->enabled = false;
			state->fid = 0U;
			state->error_id = (int64_t)PFDI_SMCC_RESERVED_ERROR_ID;

			if (plat_desc->check_plat_err != NULL) {
				int64_t validated_err = plat_desc->check_plat_err(fid, err);

				if (validated_err != err) {
					WARN("PFDI: validation mismatch for fid 0x%x\n", fid);
					WARN("PFDI: original=%" PRId64 ", validated=%" PRId64 "\n",
					     err, validated_err);
				}
			}
			return err;
		}
	}

	return PFDI_SMCC_RESERVED_ERROR_ID;
}

pfdi_status_t pfdi_pe_oor_test_run(void)
{
	pfdi_status_t rc = 0;
	uint64_t ft_id, tc_size;
	unsigned int core;

	core = plat_my_core_pos();
	/* Already executed on this core */
	if (pfdi_oor_done[core]) {
		return 0;
	}

	pfdi_oor_done[core] = true;

	/*
	 * Check whether OoR PFDI has run before, regardless of whether it
	 * succeeded or failed the last time.
	 */
	rc = pfdi_pe_test_result(&ft_id);
	if (rc != PFDI_RET_NOT_RUN) {
		goto exit;
	}
	rc = pfdi_pe_test_part_count(&tc_size);
	if (rc != 0) {
		goto exit;
	}

	if (tc_size == 0U) {
		rc = PFDI_SMCC_RET_SUCCESS;
		goto exit;
	}

	rc = pfdi_pe_test_run(0UL, tc_size - 1UL, PFDI_OOR_MODE, &ft_id);

exit:
	/*
	 * Publish the stored OoR result before exposing completion to another PE.
	 */
	dsbishst();
	pfdi_oor_complete[core] = true;

	return rc;
}

pfdi_status_t pfdi_pe_test_part_count(uint64_t *tc_size)
{
	pfdi_status_t rc = 0;

	if (tc_size == NULL) {
		return PFDI_RET_INVALID_PARAMETERS;
	}
	rc = pfdi_func_desc.count(tc_size);
	if (rc == PFDI_RET_TEST_COUNT_ZERO) {
		*tc_size = 0U;
		return PFDI_SMCC_RET_SUCCESS;
	}

	return rc;
}

static pfdi_status_t pfdi_validate_test_run_args_internal(int64_t start,
	int64_t end,
	uint64_t mode,
	uint64_t *test_count,
	bool *full_range)
{
	pfdi_status_t rc;
	uint64_t tc;
	bool run_all;

	if (!is_valid_mode(mode)) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	if ((start < -1) || (end < -1)) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	run_all = (start == -1) && (end == -1);
	if (((start == -1) || (end == -1)) && !run_all) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	rc = pfdi_pe_test_part_count(&tc);
	if (rc != 0) {
		return rc;
	}

	if (!run_all) {
		if (start > end) {
			return PFDI_RET_INVALID_PARAMETERS;
		}

		if (((uint64_t)start >= tc) ||
			((uint64_t)end >= tc)) {
			return PFDI_RET_INVALID_PARAMETERS;
		}
	}

	if (test_count != NULL) {
		*test_count = tc;
	}

	if (full_range != NULL) {
		*full_range = run_all;
	}

	return PFDI_SMCC_RET_SUCCESS;
}

pfdi_status_t pfdi_pe_test_run_validate(int64_t start, int64_t end, uint64_t mode)
{
	return pfdi_validate_test_run_args_internal(start, end, mode, NULL, NULL);
}

pfdi_status_t pfdi_pe_test_run(uint64_t start, uint64_t end, uint64_t mode,
				uint64_t *ft_id)
{
	pfdi_status_t rc = 0;
	uint64_t test_count;
	const struct plat_pfdi_func_desc *plat_desc = pfdi_get_plat_func_desc();
	bool full_range;
	int64_t start_id = (int64_t)start;
	int64_t end_id = (int64_t)end;

	if (ft_id == NULL) {
		ERROR("PFDI: Invalid parameters: start=%" PRIu64
			", end=%" PRIu64 ", mode=%" PRIu64 "\n",
			start, end, mode);
		rc = PFDI_RET_INVALID_PARAMETERS;
		goto exit;
	}

	rc = pfdi_validate_test_run_args_internal(start_id, end_id, mode,
		&test_count, &full_range);
	if (rc != 0) {
		ERROR("PFDI: Invalid parameters: start=%" PRIu64
			", end=%" PRIu64 ", mode=%" PRIu64 "\n",
			start, end, mode);
		goto exit;
	}

	if (full_range) {
		if (test_count == 0U) {
			rc =  0;
			goto exit;
		}

		start = 0U;
		end = test_count - 1U;
	}

	rc = pfdi_func_desc.run(start, end, mode, ft_id);

exit:
	if (plat_desc->post_run != NULL) {
		plat_desc->post_run(rc, start, end, mode, ft_id);
	}

	return rc;
}

pfdi_status_t pfdi_pe_test_id(uint64_t *lib_version)
{
	if (lib_version == NULL) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	/*
	 * A zero/invalid test library version means this platform does not have
	 * usable metadata to report for PFDI_PE_TEST_ID.
	 */
	if (((PFDI_TEST_LIB_VERSION & PFDI_VERSION_ALLOWED_MASK) == UINT64_C(0)) ||
		((PFDI_TEST_LIB_VERSION & PFDI_UPPER_RESERVED_MASK) != UINT64_C(0)) ||
		((PFDI_TEST_LIB_VERSION & PFDI_RESERVED_MASK) != UINT64_C(0)) ||
		(PFDI_CFG_VENDOR != PFDI_VENDOR_ARM)) {
		*lib_version = 0ULL;
		return PFDI_RET_UNKNOWN;
	}

	*lib_version = 0ULL;

	pack_vendor_id(lib_version);

	if (*lib_version == 0ULL) {
		return PFDI_RET_UNKNOWN;
	}

	return (pfdi_status_t)0;
}

pfdi_status_t pfdi_pe_test_result(uint64_t *ft_id)
{
	uint64_t cpu_num;

	if (ft_id == NULL) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	*ft_id = UINT64_MAX;
	cpu_num = plat_my_core_pos();

	return pfdi_func_desc.result(cpu_num, ft_id);
}

pfdi_status_t pfdi_version(uint64_t *pfdi_version)
{
	if (pfdi_version == NULL) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	*pfdi_version = PFDI_VENDOR_VERSION;

	return (pfdi_status_t)0;
}

pfdi_status_t pfdi_pe_fw_check(void)
{
	/* No firmware checks implemented yet */
	return (pfdi_status_t)0;
}

pfdi_status_t pfdi_pe_features(uint32_t fid)
{
	if (!is_pfdi_fid(fid)) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	if (!IS_FEATURE_SUPPORTED(fid)) {
		return PFDI_SMCC_RET_NOT_SUPPORTED;
	}

	return (pfdi_status_t)0;
}

pfdi_status_t pfdi_pe_force_error_validate(uint32_t fid, int64_t error_id)
{
	if (!is_pfdi_fid(fid)) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	if (!IS_FEATURE_SUPPORTED(fid)) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	if (!is_valid_force_error_id(error_id)) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	return PFDI_SMCC_RET_SUCCESS;
}

pfdi_status_t pfdi_pe_force_error(uint32_t fid, int64_t error_id)
{
	uint32_t core = plat_my_core_pos();
	force_err_inject_t *state;
	const struct plat_pfdi_func_desc *plat_desc = pfdi_get_plat_func_desc();
	pfdi_status_t rc;

	rc = pfdi_pe_force_error_validate(fid, error_id);
	if (rc != 0) {
		return rc;
	}

	/* allocate/find slot for this fid on this core */
	state = pfdi_find_or_alloc_force_slot(core, fid);
	if (state == NULL) {
		return PFDI_RET_ERROR;
	}

	if (plat_desc->force_plat_err != NULL) {
		if (plat_desc->force_plat_err(fid, error_id) != 0) {
			return PFDI_RET_ERROR;
		}
	}

	state->fid = fid;
	state->enabled = true;
	state->error_id = error_id;

	return (pfdi_status_t)0;
}
