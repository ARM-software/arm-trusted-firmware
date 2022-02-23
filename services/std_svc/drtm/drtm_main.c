/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 * DRTM service
 *
 * Authors:
 *	Lucian Paul-Trifu <lucian.paultrifu@gmail.com>
 *	Brian Nezvadovitz <brinez@microsoft.com> 2021-02-01
 */

#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include "drtm_main.h"
#include <services/drtm_svc.h>

int drtm_setup(void)
{
	INFO("DRTM service setup\n");

	return 0;
}

uint64_t drtm_smc_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	/* Check that the SMC call is from the Normal World. */
	if (!is_caller_non_secure(flags)) {
		SMC_RET1(handle, NOT_SUPPORTED);
	}

	switch (smc_fid) {
	case ARM_DRTM_SVC_VERSION:
		INFO("DRTM service handler: version\n");
		/* Return the version of current implementation */
		SMC_RET1(handle, ARM_DRTM_VERSION);
		break;	/* not reached */

	case ARM_DRTM_SVC_FEATURES:
		if (((x1 >> ARM_DRTM_FUNC_SHIFT) & ARM_DRTM_FUNC_MASK) ==
		    ARM_DRTM_FUNC_ID) {
			/* Dispatch function-based queries. */
			switch (x1 & FUNCID_MASK) {
			case ARM_DRTM_SVC_VERSION:
				SMC_RET1(handle, SUCCESS);
				break;	/* not reached */

			case ARM_DRTM_SVC_FEATURES:
				SMC_RET1(handle, SUCCESS);
				break;	/* not reached */

			case ARM_DRTM_SVC_UNPROTECT_MEM:
				SMC_RET1(handle, SUCCESS);
				break;	/* not reached */

			case ARM_DRTM_SVC_DYNAMIC_LAUNCH:
				SMC_RET1(handle, SUCCESS);
				break;	/* not reached */

			case ARM_DRTM_SVC_CLOSE_LOCALITY:
				WARN("ARM_DRTM_SVC_CLOSE_LOCALITY feature %s",
				     "is not supported\n");
				SMC_RET1(handle, NOT_SUPPORTED);
				break;	/* not reached */

			case ARM_DRTM_SVC_GET_ERROR:
				SMC_RET1(handle, SUCCESS);
				break;	/* not reached */

			case ARM_DRTM_SVC_SET_ERROR:
				SMC_RET1(handle, SUCCESS);
				break;	/* not reached */

			case ARM_DRTM_SVC_SET_TCB_HASH:
				WARN("ARM_DRTM_SVC_TCB_HASH feature %s",
				     "is not supported\n");
				SMC_RET1(handle, NOT_SUPPORTED);
				break;	/* not reached */

			case ARM_DRTM_SVC_LOCK_TCB_HASH:
				WARN("ARM_DRTM_SVC_LOCK_TCB_HASH feature %s",
				     "is not supported\n");
				SMC_RET1(handle, NOT_SUPPORTED);
				break;	/* not reached */

			default:
				ERROR("Unknown DRTM service function\n");
				SMC_RET1(handle, NOT_SUPPORTED);
				break;	/* not reached */
			}
		}

	case ARM_DRTM_SVC_UNPROTECT_MEM:
		INFO("DRTM service handler: unprotect mem\n");
		SMC_RET1(handle, SMC_OK);
		break;	/* not reached */

	case ARM_DRTM_SVC_DYNAMIC_LAUNCH:
		INFO("DRTM service handler: dynamic launch\n");
		SMC_RET1(handle, SMC_OK);
		break;	/* not reached */

	case ARM_DRTM_SVC_CLOSE_LOCALITY:
		WARN("DRTM service handler: close locality %s\n",
		     "is not supported");
		SMC_RET1(handle, NOT_SUPPORTED);
		break;	/* not reached */

	case ARM_DRTM_SVC_GET_ERROR:
		INFO("DRTM service handler: get error\n");
		SMC_RET2(handle, SMC_OK, 0);
		break;	/* not reached */

	case ARM_DRTM_SVC_SET_ERROR:
		INFO("DRTM service handler: set error\n");
		SMC_RET1(handle, SMC_OK);
		break;	/* not reached */

	case ARM_DRTM_SVC_SET_TCB_HASH:
		WARN("DRTM service handler: set TCB hash %s\n",
		     "is not supported");
		SMC_RET1(handle, NOT_SUPPORTED);
		break;  /* not reached */

	case ARM_DRTM_SVC_LOCK_TCB_HASH:
		WARN("DRTM service handler: lock TCB hash %s\n",
		     "is not supported");
		SMC_RET1(handle, NOT_SUPPORTED);
		break;  /* not reached */

	default:
		ERROR("Unknown DRTM service function: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
		break;	/* not reached */
	}

	/* not reached */
	SMC_RET1(handle, SMC_UNK);
}
