/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <services/el3_spmd_logical_sp.h>
#include <services/ffa_svc.h>

#if ENABLE_SPMD_LP
static bool is_spmd_lp_inited;
static bool is_spmc_inited;

/*
 * Helper function to obtain the array storing the EL3
 * SPMD Logical Partition descriptors.
 */
static struct spmd_lp_desc *get_spmd_el3_lp_array(void)
{
	return (struct spmd_lp_desc *) SPMD_LP_DESCS_START;
}

/*******************************************************************************
 * Validate any logical partition descriptors before we initialize.
 * Initialization of said partitions will be taken care of during SPMD boot.
 ******************************************************************************/
static int el3_spmd_sp_desc_validate(struct spmd_lp_desc *lp_array)
{
	/* Check the array bounds are valid. */
	assert(SPMD_LP_DESCS_END > SPMD_LP_DESCS_START);

	/* If no SPMD logical partitions are implemented then simply bail out. */
	if (SPMD_LP_DESCS_COUNT == 0U) {
		return -1;
	}

	for (uint32_t index = 0U; index < SPMD_LP_DESCS_COUNT; index++) {
		struct spmd_lp_desc *lp_desc = &lp_array[index];

		/* Validate our logical partition descriptors. */
		if (lp_desc == NULL) {
			ERROR("Invalid SPMD Logical SP Descriptor\n");
			return -EINVAL;
		}

		/*
		 * Ensure the ID follows the convention to indicate it resides
		 * in the secure world.
		 */
		if (!ffa_is_secure_world_id(lp_desc->sp_id)) {
			ERROR("Invalid SPMD Logical SP ID (0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure SPMD logical partition is in valid range. */
		if (!is_spmd_lp_id(lp_desc->sp_id)) {
			ERROR("Invalid SPMD Logical Partition ID (0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure the UUID is not the NULL UUID. */
		if (lp_desc->uuid[0] == 0 && lp_desc->uuid[1] == 0 &&
		    lp_desc->uuid[2] == 0 && lp_desc->uuid[3] == 0) {
			ERROR("Invalid UUID for SPMD Logical SP (0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure init function callback is registered. */
		if (lp_desc->init == NULL) {
			ERROR("Missing init function for Logical SP(0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure that SPMD LP only supports sending direct requests. */
		if (lp_desc->properties != FFA_PARTITION_DIRECT_REQ_SEND) {
			ERROR("Invalid SPMD logical partition properties (0x%x)\n",
			      lp_desc->properties);
			return -EINVAL;
		}

		/* Ensure that all partition IDs are unique. */
		for (uint32_t inner_idx = index + 1;
		     inner_idx < SPMD_LP_DESCS_COUNT; inner_idx++) {
			if (lp_desc->sp_id == lp_array[inner_idx].sp_id) {
				ERROR("Duplicate SPMD logical SP ID Detected (0x%x)\n",
				      lp_desc->sp_id);
				return -EINVAL;
			}
		}
	}
	return 0;
}
#endif

/*
 * Initialize SPMD logical partitions. This function assumes that it is called
 * only after the SPMC has successfully initialized.
 */
int32_t spmd_logical_sp_init(void)
{
#if ENABLE_SPMD_LP
	int32_t rc = 0;
	struct spmd_lp_desc *spmd_lp_descs;

	if (is_spmd_lp_inited == true) {
		return 0;
	}

	if (is_spmc_inited == false) {
		return -1;
	}

	spmd_lp_descs = get_spmd_el3_lp_array();

	/* Perform initial validation of the SPMD Logical Partitions. */
	rc = el3_spmd_sp_desc_validate(spmd_lp_descs);
	if (rc != 0) {
		ERROR("Logical SPMD Partition validation failed!\n");
		return rc;
	}

	VERBOSE("SPMD Logical Secure Partition init start.\n");
	for (unsigned int i = 0U; i < SPMD_LP_DESCS_COUNT; i++) {
		rc = spmd_lp_descs[i].init();
		if (rc != 0) {
			ERROR("SPMD Logical SP (0x%x) failed to initialize\n",
			      spmd_lp_descs[i].sp_id);
			return rc;
		}
		VERBOSE("SPMD Logical SP (0x%x) Initialized\n",
			spmd_lp_descs[i].sp_id);
	}

	INFO("SPMD Logical Secure Partition init completed.\n");
	if (rc == 0) {
		is_spmd_lp_inited = true;
	}
	return rc;
#else
	return 0;
#endif
}

void spmd_logical_sp_set_spmc_initialized(void)
{
#if ENABLE_SPMD_LP
	is_spmc_inited = true;
#endif
}

void spmd_logical_sp_set_spmc_failure(void)
{
#if ENABLE_SPMD_LP
	is_spmc_inited = false;
#endif
}
