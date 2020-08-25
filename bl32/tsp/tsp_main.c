/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
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
#include <platform_def.h>
#include <platform_tsp.h>

#include "tsp_private.h"


/*******************************************************************************
 * Lock to control access to the console
 ******************************************************************************/
spinlock_t console_lock;

/*******************************************************************************
 * Per cpu data structure to populate parameters for an SMC in C code and use
 * a pointer to this structure in assembler code to populate x0-x7
 ******************************************************************************/
static tsp_args_t tsp_smc_args[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * Per cpu data structure to keep track of TSP activity
 ******************************************************************************/
work_statistics_t tsp_stats[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * The TSP memory footprint starts at address BL32_BASE and ends with the
 * linker symbol __BL32_END__. Use these addresses to compute the TSP image
 * size.
 ******************************************************************************/
#define BL32_TOTAL_LIMIT BL32_END
#define BL32_TOTAL_SIZE (BL32_TOTAL_LIMIT - (unsigned long) BL32_BASE)

static tsp_args_t *set_smc_args(uint64_t arg0,
			     uint64_t arg1,
			     uint64_t arg2,
			     uint64_t arg3,
			     uint64_t arg4,
			     uint64_t arg5,
			     uint64_t arg6,
			     uint64_t arg7)
{
	uint32_t linear_id;
	tsp_args_t *pcpu_smc_args;

	/*
	 * Return to Secure Monitor by raising an SMC. The results of the
	 * service are passed as an arguments to the SMC
	 */
	linear_id = plat_my_core_pos();
	pcpu_smc_args = &tsp_smc_args[linear_id];
	write_sp_arg(pcpu_smc_args, TSP_ARG0, arg0);
	write_sp_arg(pcpu_smc_args, TSP_ARG1, arg1);
	write_sp_arg(pcpu_smc_args, TSP_ARG2, arg2);
	write_sp_arg(pcpu_smc_args, TSP_ARG3, arg3);
	write_sp_arg(pcpu_smc_args, TSP_ARG4, arg4);
	write_sp_arg(pcpu_smc_args, TSP_ARG5, arg5);
	write_sp_arg(pcpu_smc_args, TSP_ARG6, arg6);
	write_sp_arg(pcpu_smc_args, TSP_ARG7, arg7);

	return pcpu_smc_args;
}

/*******************************************************************************
 * Setup function for TSP.
 ******************************************************************************/
void tsp_setup(void)
{
	/* Perform early platform-specific setup */
	tsp_early_platform_setup();

	/* Perform late platform-specific setup */
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
 * TSP main entry point where it gets the opportunity to initialize its secure
 * state/applications. Once the state is initialized, it must return to the
 * SPD with a pointer to the 'tsp_vector_table' jump table.
 ******************************************************************************/
uint64_t tsp_main(void)
{
	NOTICE("TSP: %s\n", version_string);
	NOTICE("TSP: %s\n", build_message);
	INFO("TSP: Total memory base : 0x%lx\n", (unsigned long) BL32_BASE);
	INFO("TSP: Total memory size : 0x%lx bytes\n", BL32_TOTAL_SIZE);

	uint32_t linear_id = plat_my_core_pos();

	/* Initialize the platform */
	tsp_platform_setup();

	/* Initialize secure/applications state here */
	tsp_generic_timer_start();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_on_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu on requests\n",
	     read_mpidr(),
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count,
	     tsp_stats[linear_id].cpu_on_count);
	spin_unlock(&console_lock);
#endif
	return (uint64_t) &tsp_vector_table;
}

/*******************************************************************************
 * This function performs any remaining book keeping in the test secure payload
 * after this cpu's architectural state has been setup in response to an earlier
 * psci cpu_on request.
 ******************************************************************************/
tsp_args_t *tsp_cpu_on_main(void)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Initialize secure/applications state here */
	tsp_generic_timer_start();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_on_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx turned on\n", read_mpidr());
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu on requests\n",
		read_mpidr(),
		tsp_stats[linear_id].smc_count,
		tsp_stats[linear_id].eret_count,
		tsp_stats[linear_id].cpu_on_count);
	spin_unlock(&console_lock);
#endif
	/* Indicate to the SPD that we have completed turned ourselves on */
	return set_smc_args(TSP_ON_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any remaining book keeping in the test secure payload
 * before this cpu is turned off in response to a psci cpu_off request.
 ******************************************************************************/
tsp_args_t *tsp_cpu_off_main(uint64_t arg0,
			   uint64_t arg1,
			   uint64_t arg2,
			   uint64_t arg3,
			   uint64_t arg4,
			   uint64_t arg5,
			   uint64_t arg6,
			   uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/*
	 * This cpu is being turned off, so disable the timer to prevent the
	 * secure timer interrupt from interfering with power down. A pending
	 * interrupt will be lost but we do not care as we are turning off.
	 */
	tsp_generic_timer_stop();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_off_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx off request\n", read_mpidr());
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu off requests\n",
		read_mpidr(),
		tsp_stats[linear_id].smc_count,
		tsp_stats[linear_id].eret_count,
		tsp_stats[linear_id].cpu_off_count);
	spin_unlock(&console_lock);
#endif

	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_OFF_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any book keeping in the test secure payload before
 * this cpu's architectural state is saved in response to an earlier psci
 * cpu_suspend request.
 ******************************************************************************/
tsp_args_t *tsp_cpu_suspend_main(uint64_t arg0,
			       uint64_t arg1,
			       uint64_t arg2,
			       uint64_t arg3,
			       uint64_t arg4,
			       uint64_t arg5,
			       uint64_t arg6,
			       uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/*
	 * Save the time context and disable it to prevent the secure timer
	 * interrupt from interfering with wakeup from the suspend state.
	 */
	tsp_generic_timer_save();
	tsp_generic_timer_stop();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_suspend_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu suspend requests\n",
		read_mpidr(),
		tsp_stats[linear_id].smc_count,
		tsp_stats[linear_id].eret_count,
		tsp_stats[linear_id].cpu_suspend_count);
	spin_unlock(&console_lock);
#endif

	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_SUSPEND_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any book keeping in the test secure payload after this
 * cpu's architectural state has been restored after wakeup from an earlier psci
 * cpu_suspend request.
 ******************************************************************************/
tsp_args_t *tsp_cpu_resume_main(uint64_t max_off_pwrlvl,
			      uint64_t arg1,
			      uint64_t arg2,
			      uint64_t arg3,
			      uint64_t arg4,
			      uint64_t arg5,
			      uint64_t arg6,
			      uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Restore the generic timer context */
	tsp_generic_timer_restore();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_resume_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx resumed. maximum off power level %" PRId64 "\n",
	     read_mpidr(), max_off_pwrlvl);
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu resume requests\n",
		read_mpidr(),
		tsp_stats[linear_id].smc_count,
		tsp_stats[linear_id].eret_count,
		tsp_stats[linear_id].cpu_resume_count);
	spin_unlock(&console_lock);
#endif
	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_RESUME_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any remaining bookkeeping in the test secure payload
 * before the system is switched off (in response to a psci SYSTEM_OFF request)
 ******************************************************************************/
tsp_args_t *tsp_system_off_main(uint64_t arg0,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx SYSTEM_OFF request\n", read_mpidr());
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets requests\n", read_mpidr(),
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count);
	spin_unlock(&console_lock);
#endif

	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_SYSTEM_OFF_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any remaining bookkeeping in the test secure payload
 * before the system is reset (in response to a psci SYSTEM_RESET request)
 ******************************************************************************/
tsp_args_t *tsp_system_reset_main(uint64_t arg0,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx SYSTEM_RESET request\n", read_mpidr());
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets requests\n", read_mpidr(),
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count);
	spin_unlock(&console_lock);
#endif

	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_SYSTEM_RESET_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * TSP fast smc handler. The secure monitor jumps to this function by
 * doing the ERET after populating X0-X7 registers. The arguments are received
 * in the function arguments in order. Once the service is rendered, this
 * function returns to Secure Monitor by raising SMC.
 ******************************************************************************/
tsp_args_t *tsp_smc_handler(uint64_t func,
			       uint64_t arg1,
			       uint64_t arg2,
			       uint64_t arg3,
			       uint64_t arg4,
			       uint64_t arg5,
			       uint64_t arg6,
			       uint64_t arg7)
{
	uint128_t service_args;
	uint64_t service_arg0;
	uint64_t service_arg1;
	uint64_t results[2];
	uint32_t linear_id = plat_my_core_pos();

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;

#if LOG_LEVEL >= LOG_LEVEL_INFO
	spin_lock(&console_lock);
	INFO("TSP: cpu 0x%lx received %s smc 0x%" PRIx64 "\n", read_mpidr(),
		((func >> 31) & 1) == 1 ? "fast" : "yielding",
		func);
	INFO("TSP: cpu 0x%lx: %d smcs, %d erets\n", read_mpidr(),
		tsp_stats[linear_id].smc_count,
		tsp_stats[linear_id].eret_count);
	spin_unlock(&console_lock);
#endif

	/* Render secure services and obtain results here */
	results[0] = arg1;
	results[1] = arg2;

	/*
	 * Request a service back from dispatcher/secure monitor.
	 * This call returns and thereafter resumes execution.
	 */
	service_args = tsp_get_magic();
	service_arg0 = (uint64_t)service_args;
	service_arg1 = (uint64_t)(service_args >> 64U);

#if CTX_INCLUDE_MTE_REGS
	/*
	 * Write a dummy value to an MTE register, to simulate usage in the
	 * secure world
	 */
	write_gcr_el1(0x99);
#endif

	/* Determine the function to perform based on the function ID */
	switch (TSP_BARE_FID(func)) {
	case TSP_ADD:
		results[0] += service_arg0;
		results[1] += service_arg1;
		break;
	case TSP_SUB:
		results[0] -= service_arg0;
		results[1] -= service_arg1;
		break;
	case TSP_MUL:
		results[0] *= service_arg0;
		results[1] *= service_arg1;
		break;
	case TSP_DIV:
		results[0] /= service_arg0 ? service_arg0 : 1;
		results[1] /= service_arg1 ? service_arg1 : 1;
		break;
	default:
		break;
	}

	return set_smc_args(func, 0,
			    results[0],
			    results[1],
			    0, 0, 0, 0);
}

/*******************************************************************************
 * TSP smc abort handler. This function is called when aborting a preempted
 * yielding SMC request. It should cleanup all resources owned by the SMC
 * handler such as locks or dynamically allocated memory so following SMC
 * request are executed in a clean environment.
 ******************************************************************************/
tsp_args_t *tsp_abort_smc_handler(uint64_t func,
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
