/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include <arch_features.h>
#include <arch_helpers.h>
#include <bl32/tsp/tsp.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <platform_tsp.h>
#include "tsp_private.h"

#include <platform_def.h>

/*******************************************************************************
 * Per cpu data structure to populate parameters for an SMC in C code and use
 * a pointer to this structure in assembler code to populate x0-x7.
 ******************************************************************************/
static smc_args_t tsp_smc_args[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * Per cpu data structure to keep track of TSP activity
 ******************************************************************************/
work_statistics_t tsp_stats[PLATFORM_CORE_COUNT];

smc_args_t *set_smc_args(uint64_t arg0,
			 uint64_t arg1,
			 uint64_t arg2,
			 uint64_t arg3,
			 uint64_t arg4,
			 uint64_t arg5,
			 uint64_t arg6,
			 uint64_t arg7)
{
	uint32_t linear_id;
	smc_args_t *pcpu_smc_args;

	/*
	 * Return to Secure Monitor by raising an SMC. The results of the
	 * service are passed as an arguments to the SMC.
	 */
	linear_id = plat_my_core_pos();
	pcpu_smc_args = &tsp_smc_args[linear_id];
	write_sp_arg(pcpu_smc_args, SMC_ARG0, arg0);
	write_sp_arg(pcpu_smc_args, SMC_ARG1, arg1);
	write_sp_arg(pcpu_smc_args, SMC_ARG2, arg2);
	write_sp_arg(pcpu_smc_args, SMC_ARG3, arg3);
	write_sp_arg(pcpu_smc_args, SMC_ARG4, arg4);
	write_sp_arg(pcpu_smc_args, SMC_ARG5, arg5);
	write_sp_arg(pcpu_smc_args, SMC_ARG6, arg6);
	write_sp_arg(pcpu_smc_args, SMC_ARG7, arg7);

	return pcpu_smc_args;
}

/*******************************************************************************
 * Setup function for TSP.
 ******************************************************************************/
void tsp_setup(void)
{
	/* Perform early platform-specific setup. */
	tsp_early_platform_setup();

	/* Perform late platform-specific setup. */
	tsp_plat_arch_setup();

#if ENABLE_PAUTH
	/*
	 * Assert that the ARMv8.3-PAuth registers are present or an access
	 * fault will be triggered when they are being saved or restored.
	 */
	assert(is_armv8_3_pauth_present());
#endif /* ENABLE_PAUTH */
}

/*******************************************************************************
 * This function performs any remaining bookkeeping in the test secure payload
 * before the system is switched off (in response to a psci SYSTEM_OFF request).
 ******************************************************************************/
smc_args_t *tsp_system_off_main(uint64_t arg0,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Update this cpu's statistics. */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;

	INFO("TSP: cpu 0x%lx SYSTEM_OFF request\n", read_mpidr());
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets requests\n", read_mpidr(),
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count);

	/* Indicate to the SPD that we have completed this request. */
	return set_smc_args(TSP_SYSTEM_OFF_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any remaining bookkeeping in the test secure payload
 * before the system is reset (in response to a psci SYSTEM_RESET request).
 ******************************************************************************/
smc_args_t *tsp_system_reset_main(uint64_t arg0,
				  uint64_t arg1,
				  uint64_t arg2,
				  uint64_t arg3,
				  uint64_t arg4,
				  uint64_t arg5,
				  uint64_t arg6,
				  uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Update this cpu's statistics. */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;

	INFO("TSP: cpu 0x%lx SYSTEM_RESET request\n", read_mpidr());
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets requests\n", read_mpidr(),
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count);

	/* Indicate to the SPD that we have completed this request. */
	return set_smc_args(TSP_SYSTEM_RESET_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * TSP smc abort handler. This function is called when aborting a preempted
 * yielding SMC request. It should cleanup all resources owned by the SMC
 * handler such as locks or dynamically allocated memory so following SMC
 * request are executed in a clean environment.
 ******************************************************************************/
smc_args_t *tsp_abort_smc_handler(uint64_t func,
				  uint64_t arg1,
				  uint64_t arg2,
				  uint64_t arg3,
				  uint64_t arg4,
				  uint64_t arg5,
				  uint64_t arg6,
				  uint64_t arg7)
{
	return set_smc_args(TSP_ABORT_DONE, 0, 0, 0, 0, 0, 0, 0);
}
