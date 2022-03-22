/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <common/debug.h>
#include <plat/common/platform.h>
#include <services/rmmd_svc.h>
#include <services/trp/platform_trp.h>

#include <platform_def.h>
#include "trp_private.h"

/*******************************************************************************
 * Per cpu data structure to populate parameters for an SMC in C code and use
 * a pointer to this structure in assembler code to populate x0-x7
 ******************************************************************************/
static trp_args_t trp_smc_args[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * Set the arguments for SMC call
 ******************************************************************************/
static trp_args_t *set_smc_args(uint64_t arg0,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7)
{
	uint32_t linear_id;
	trp_args_t *pcpu_smc_args;

	/*
	 * Return to Secure Monitor by raising an SMC. The results of the
	 * service are passed as an arguments to the SMC
	 */
	linear_id = plat_my_core_pos();
	pcpu_smc_args = &trp_smc_args[linear_id];
	write_trp_arg(pcpu_smc_args, TRP_ARG0, arg0);
	write_trp_arg(pcpu_smc_args, TRP_ARG1, arg1);
	write_trp_arg(pcpu_smc_args, TRP_ARG2, arg2);
	write_trp_arg(pcpu_smc_args, TRP_ARG3, arg3);
	write_trp_arg(pcpu_smc_args, TRP_ARG4, arg4);
	write_trp_arg(pcpu_smc_args, TRP_ARG5, arg5);
	write_trp_arg(pcpu_smc_args, TRP_ARG6, arg6);
	write_trp_arg(pcpu_smc_args, TRP_ARG7, arg7);

	return pcpu_smc_args;
}

/*******************************************************************************
 * Setup function for TRP.
 ******************************************************************************/
void trp_setup(void)
{
	/* Perform early platform-specific setup */
	trp_early_platform_setup();
}

/* Main function for TRP */
void trp_main(void)
{
	NOTICE("TRP: %s\n", version_string);
	NOTICE("TRP: %s\n", build_message);
	INFO("TRP: Memory base : 0x%lx\n", (unsigned long)RMM_BASE);
	INFO("TRP: Total size : 0x%lx bytes\n", (unsigned long)(RMM_END
								- RMM_BASE));
}

/*******************************************************************************
 * Returning RMI version back to Normal World
 ******************************************************************************/
static trp_args_t *trp_ret_rmi_version(void)
{
	VERBOSE("RMM version is %u.%u\n", RMI_ABI_VERSION_MAJOR,
					  RMI_ABI_VERSION_MINOR);
	return set_smc_args(RMMD_RMI_REQ_COMPLETE, RMI_ABI_VERSION,
			    0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * Transitioning granule of NON-SECURE type to REALM type
 ******************************************************************************/
static trp_args_t *trp_asc_mark_realm(unsigned long long x1)
{
	unsigned long long ret;

	VERBOSE("Delegating granule 0x%llx\n", x1);
	ret = trp_smc(set_smc_args(RMMD_GTSI_DELEGATE, x1, 0, 0, 0, 0, 0, 0));

	if (ret != 0ULL) {
		ERROR("Granule transition from NON-SECURE type to REALM type "
			"failed 0x%llx\n", ret);
	}
	return set_smc_args(RMMD_RMI_REQ_COMPLETE, ret, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * Transitioning granule of REALM type to NON-SECURE type
 ******************************************************************************/
static trp_args_t *trp_asc_mark_nonsecure(unsigned long long x1)
{
	unsigned long long ret;

	VERBOSE("Undelegating granule 0x%llx\n", x1);
	ret = trp_smc(set_smc_args(RMMD_GTSI_UNDELEGATE, x1, 0, 0, 0, 0, 0, 0));

	if (ret != 0ULL) {
		ERROR("Granule transition from REALM type to NON-SECURE type "
			"failed 0x%llx\n", ret);
	}
	return set_smc_args(RMMD_RMI_REQ_COMPLETE, ret, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * Main RMI SMC handler function
 ******************************************************************************/
trp_args_t *trp_rmi_handler(unsigned long fid, unsigned long long x1)
{
	switch (fid) {
	case RMI_RMM_REQ_VERSION:
		return trp_ret_rmi_version();
	case RMI_RMM_GRANULE_DELEGATE:
		return trp_asc_mark_realm(x1);
	case RMI_RMM_GRANULE_UNDELEGATE:
		return trp_asc_mark_nonsecure(x1);
	default:
		ERROR("Invalid SMC code to %s, FID %lu\n", __func__, fid);
	}
	return set_smc_args(SMC_UNK, 0, 0, 0, 0, 0, 0, 0);
}
