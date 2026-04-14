/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>

#include <common/debug.h>
#include <services/pfdi.h>
#include <services/pfdi_svc.h>

static struct plat_pfdi_func_desc plat_pfdi_func_desc;
static const struct plat_pfdi_func_desc *plat_pfdi_func_desc_ptr =
	&plat_pfdi_func_desc;

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

void pfdi_init(void)
{
	assert(pfdi_func_desc.name != NULL);
	assert(pfdi_func_desc.run != NULL);
	assert(pfdi_func_desc.count != NULL);
	assert(pfdi_func_desc.result != NULL);

	NOTICE("PFDI: Initializing Platform Fault Detection Interface.\n");
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
