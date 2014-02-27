/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <bl32.h>
#include <tsp.h>
#include <arch_helpers.h>
#include <stdio.h>
#include <platform.h>
#include <debug.h>
#include <spinlock.h>

/*******************************************************************************
 * Lock to control access to the console
 ******************************************************************************/
spinlock_t console_lock;

/*******************************************************************************
 * Per cpu data structure to populate parameters for an SMC in C code and use
 * a pointer to this structure in assembler code to populate x0-x7
 ******************************************************************************/
static tsp_args tsp_smc_args[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * Per cpu data structure to keep track of TSP activity
 ******************************************************************************/
static work_statistics tsp_stats[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * Single reference to the various entry points exported by the test secure
 * payload.  A single copy should suffice for all cpus as they are not expected
 * to change.
 ******************************************************************************/
static const entry_info tsp_entry_info = {
	tsp_fast_smc_entry,
	tsp_cpu_on_entry,
	tsp_cpu_off_entry,
	tsp_cpu_resume_entry,
	tsp_cpu_suspend_entry,
};

static tsp_args *set_smc_args(uint64_t arg0,
			     uint64_t arg1,
			     uint64_t arg2,
			     uint64_t arg3,
			     uint64_t arg4,
			     uint64_t arg5,
			     uint64_t arg6,
			     uint64_t arg7)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id;
	tsp_args *pcpu_smc_args;

	/*
	 * Return to Secure Monitor by raising an SMC. The results of the
	 * service are passed as an arguments to the SMC
	 */
	linear_id = platform_get_core_pos(mpidr);
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
 * TSP main entry point where it gets the opportunity to initialize its secure
 * state/applications. Once the state is initialized, it must return to the
 * SPD with a pointer to the 'tsp_entry_info' structure.
 ******************************************************************************/
uint64_t tsp_main(void)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);

#if DEBUG
	meminfo *mem_layout = bl32_plat_sec_mem_layout();
#endif

	/* Initialize the platform */
	bl32_platform_setup();

	/* Initialize secure/applications state here */

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_on_count++;

	spin_lock(&console_lock);
	printf("TSP %s\n\r", build_message);
	INFO("Total memory base : 0x%x\n", mem_layout->total_base);
	INFO("Total memory size : 0x%x bytes\n", mem_layout->total_size);
	INFO("Free memory base  : 0x%x\n", mem_layout->free_base);
	INFO("Free memory size  : 0x%x bytes\n", mem_layout->free_size);
	INFO("cpu 0x%x: %d smcs, %d erets %d cpu on requests\n", mpidr,
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count,
	     tsp_stats[linear_id].cpu_on_count);
	spin_unlock(&console_lock);

	/*
	 * TODO: There is a massive assumption that the SPD and SP can see each
	 * other's memory without issues so it is safe to pass pointers to
	 * internal memory. Replace this with a shared communication buffer.
	 */
	return (uint64_t) &tsp_entry_info;
}

/*******************************************************************************
 * This function performs any remaining book keeping in the test secure payload
 * after this cpu's architectural state has been setup in response to an earlier
 * psci cpu_on request.
 ******************************************************************************/
tsp_args *tsp_cpu_on_main(void)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_on_count++;

	spin_lock(&console_lock);
	printf("SP: cpu 0x%x turned on\n\r", mpidr);
	INFO("cpu 0x%x: %d smcs, %d erets %d cpu on requests\n", mpidr,
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count,
	     tsp_stats[linear_id].cpu_on_count);
	spin_unlock(&console_lock);

	/* Indicate to the SPD that we have completed turned ourselves on */
	return set_smc_args(TSP_ON_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any remaining book keeping in the test secure payload
 * before this cpu is turned off in response to a psci cpu_off request.
 ******************************************************************************/
tsp_args *tsp_cpu_off_main(uint64_t arg0,
			   uint64_t arg1,
			   uint64_t arg2,
			   uint64_t arg3,
			   uint64_t arg4,
			   uint64_t arg5,
			   uint64_t arg6,
			   uint64_t arg7)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_off_count++;

	spin_lock(&console_lock);
	printf("SP: cpu 0x%x off request\n\r", mpidr);
	INFO("cpu 0x%x: %d smcs, %d erets %d cpu off requests\n", mpidr,
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count,
	     tsp_stats[linear_id].cpu_off_count);
	spin_unlock(&console_lock);


	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_OFF_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any book keeping in the test secure payload before
 * this cpu's architectural state is saved in response to an earlier psci
 * cpu_suspend request.
 ******************************************************************************/
tsp_args *tsp_cpu_suspend_main(uint64_t power_state,
			       uint64_t arg1,
			       uint64_t arg2,
			       uint64_t arg3,
			       uint64_t arg4,
			       uint64_t arg5,
			       uint64_t arg6,
			       uint64_t arg7)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_suspend_count++;

	spin_lock(&console_lock);
	printf("SP: cpu 0x%x suspend request. power state: 0x%x\n\r",
	       mpidr, power_state);
	INFO("cpu 0x%x: %d smcs, %d erets %d cpu suspend requests\n", mpidr,
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count,
	     tsp_stats[linear_id].cpu_suspend_count);
	spin_unlock(&console_lock);

	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_SUSPEND_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any book keeping in the test secure payload after this
 * cpu's architectural state has been restored after wakeup from an earlier psci
 * cpu_suspend request.
 ******************************************************************************/
tsp_args *tsp_cpu_resume_main(uint64_t suspend_level,
			      uint64_t arg1,
			      uint64_t arg2,
			      uint64_t arg3,
			      uint64_t arg4,
			      uint64_t arg5,
			      uint64_t arg6,
			      uint64_t arg7)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_resume_count++;

	spin_lock(&console_lock);
	printf("SP: cpu 0x%x resumed. suspend level %d \n\r",
	       mpidr, suspend_level);
	INFO("cpu 0x%x: %d smcs, %d erets %d cpu suspend requests\n", mpidr,
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count,
	     tsp_stats[linear_id].cpu_suspend_count);
	spin_unlock(&console_lock);

	/* Indicate to the SPD that we have completed this request */
	return set_smc_args(TSP_RESUME_DONE, 0, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * TSP fast smc handler. The secure monitor jumps to this function by
 * doing the ERET after populating X0-X7 registers. The arguments are received
 * in the function arguments in order. Once the service is rendered, this
 * function returns to Secure Monitor by raising SMC
 ******************************************************************************/
tsp_args *tsp_fast_smc_handler(uint64_t func,
			       uint64_t arg1,
			       uint64_t arg2,
			       uint64_t arg3,
			       uint64_t arg4,
			       uint64_t arg5,
			       uint64_t arg6,
			       uint64_t arg7)
{
	uint64_t results[2];
	uint64_t service_args[2];
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;

	printf("SP: cpu 0x%x received fast smc 0x%x\n", read_mpidr(), func);
	INFO("cpu 0x%x: %d smcs, %d erets\n", mpidr,
	     tsp_stats[linear_id].smc_count,
	     tsp_stats[linear_id].eret_count);

	/* Render secure services and obtain results here */

	results[0] = arg1;
	results[1] = arg2;

	/*
	 * Request a service back from dispatcher/secure monitor. This call
	 * return and thereafter resume exectuion
	 */
	tsp_get_magic(service_args);

	/* Determine the function to perform based on the function ID */
	switch (func) {
	case TSP_FID_ADD:
		results[0] += service_args[0];
		results[1] += service_args[1];
		break;
	case TSP_FID_SUB:
		results[0] -= service_args[0];
		results[1] -= service_args[1];
		break;
	case TSP_FID_MUL:
		results[0] *= service_args[0];
		results[1] *= service_args[1];
		break;
	case TSP_FID_DIV:
		results[0] /= service_args[0] ? service_args[0] : 1;
		results[1] /= service_args[1] ? service_args[1] : 1;
		break;
	default:
		break;
	}

	return set_smc_args(func,
			    results[0],
			    results[1],
			    0, 0, 0, 0, 0);
}

