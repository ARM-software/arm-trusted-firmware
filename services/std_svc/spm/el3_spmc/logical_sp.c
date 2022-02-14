/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <services/el3_spmc_logical_sp.h>
#include <services/ffa_svc.h>
#include "spmc.h"

/*******************************************************************************
 * Validate any logical partition descriptors before we initialise.
 * Initialization of said partitions will be taken care of during SPMC boot.
 ******************************************************************************/
int el3_sp_desc_validate(void)
{
	struct el3_lp_desc *lp_array;

	/*
	 * Assert the number of descriptors is less than maximum allowed.
	 * This constant should be define on a per platform basis.
	 */
	assert(EL3_LP_DESCS_COUNT <= MAX_EL3_LP_DESCS_COUNT);

	/* Check the array bounds are valid. */
	assert(EL3_LP_DESCS_END >= EL3_LP_DESCS_START);

	/* If no logical partitions are implemented then simply bail out. */
	if (EL3_LP_DESCS_COUNT == 0U) {
		return 0;
	}

	lp_array = get_el3_lp_array();

	for (unsigned int index = 0; index < EL3_LP_DESCS_COUNT; index++) {
		struct el3_lp_desc *lp_desc = &lp_array[index];

		/* Validate our logical partition descriptors. */
		if (lp_desc == NULL) {
			ERROR("Invalid Logical SP Descriptor\n");
			return -EINVAL;
		}

		/*
		 * Ensure the ID follows the convention to indidate it resides
		 * in the secure world.
		 */
		if (!ffa_is_secure_world_id(lp_desc->sp_id)) {
			ERROR("Invalid Logical SP ID (0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure we don't conflict with the SPMC partition ID. */
		if (lp_desc->sp_id == FFA_SPMC_ID) {
			ERROR("Logical SP ID clashes with SPMC ID(0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure the UUID is not the NULL UUID. */
		if (lp_desc->uuid[0] == 0 && lp_desc->uuid[1] == 0 &&
		    lp_desc->uuid[2] == 0 && lp_desc->uuid[3] == 0) {
			ERROR("Invalid UUID for Logical SP (0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure init function callback is registered. */
		if (lp_desc->init == NULL) {
			ERROR("Missing init function for Logical SP(0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure that LP only supports receiving direct requests. */
		if (lp_desc->properties &
		    ~(FFA_PARTITION_DIRECT_REQ_RECV)) {
			ERROR("Invalid partition properties (0x%x)\n",
			      lp_desc->properties);
			return -EINVAL;
		}

		/* Ensure direct request function callback is registered. */
		if (lp_desc->direct_req == NULL) {
			ERROR("No Direct Req handler for Logical SP (0x%x)\n",
			      lp_desc->sp_id);
			return -EINVAL;
		}

		/* Ensure that all partition IDs are unique. */
		for (unsigned int inner_idx = index + 1;
		     inner_idx < EL3_LP_DESCS_COUNT; inner_idx++) {
			if (lp_desc->sp_id == lp_array[inner_idx].sp_id) {
				ERROR("Duplicate SP ID Detected (0x%x)\n",
				      lp_desc->sp_id);
				return -EINVAL;
			}
		}
	}
	return 0;
}
