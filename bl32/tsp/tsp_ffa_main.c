/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include "../../services/std_svc/spm/el3_spmc/spmc.h"
#include "../../services/std_svc/spm/el3_spmc/spmc_shared_mem.h"
#include <arch_features.h>
#include <arch_helpers.h>
#include <bl32/tsp/tsp.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <platform_tsp.h>
#include <services/ffa_svc.h>
#include "tsp_private.h"

#include <platform_def.h>

extern void tsp_cpu_on_entry(void);

static ffa_endpoint_id16_t tsp_id, spmc_id;

static smc_args_t *send_ffa_pm_success(void)
{
	return set_smc_args(FFA_MSG_SEND_DIRECT_RESP_SMC32,
			    tsp_id << FFA_DIRECT_MSG_SOURCE_SHIFT |
			    spmc_id,
			    FFA_FWK_MSG_BIT |
			    (FFA_PM_MSG_PM_RESP & FFA_FWK_MSG_MASK),
			    0, 0, 0, 0, 0);
}

/*******************************************************************************
 * This function performs any remaining book keeping in the test secure payload
 * before this cpu is turned off in response to a psci cpu_off request.
 ******************************************************************************/
smc_args_t *tsp_cpu_off_main(uint64_t arg0,
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

	/* Update this cpu's statistics. */
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

	return send_ffa_pm_success();
}

/*******************************************************************************
 * This function performs any book keeping in the test secure payload before
 * this cpu's architectural state is saved in response to an earlier psci
 * cpu_suspend request.
 ******************************************************************************/
smc_args_t *tsp_cpu_suspend_main(uint64_t arg0,
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

	/* Update this cpu's statistics. */
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

	return send_ffa_pm_success();
}

/*******************************************************************************
 * This function performs any bookkeeping in the test secure payload after this
 * cpu's architectural state has been restored after wakeup from an earlier psci
 * cpu_suspend request.
 ******************************************************************************/
smc_args_t *tsp_cpu_resume_main(uint64_t max_off_pwrlvl,
				uint64_t arg1,
				uint64_t arg2,
				uint64_t arg3,
				uint64_t arg4,
				uint64_t arg5,
				uint64_t arg6,
				uint64_t arg7)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Restore the generic timer context. */
	tsp_generic_timer_restore();

	/* Update this cpu's statistics. */
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

	return send_ffa_pm_success();
}

/*******************************************************************************
 * This function handles framework messages. Currently only PM.
 ******************************************************************************/
static smc_args_t *handle_framework_message(uint64_t arg0,
					    uint64_t arg1,
					    uint64_t arg2,
					    uint64_t arg3,
					    uint64_t arg4,
					    uint64_t arg5,
					    uint64_t arg6,
					    uint64_t arg7)
{
	/* Check if it is a power management message from the SPMC. */
	if (ffa_endpoint_source(arg1) != spmc_id) {
		goto err;
	}

	/* Check if it is a PM request message. */
	if ((arg2 & FFA_FWK_MSG_MASK) == FFA_FWK_MSG_PSCI) {
		/* Check if it is a PSCI CPU_OFF request. */
		if (arg3 == PSCI_CPU_OFF) {
			return tsp_cpu_off_main(arg0, arg1, arg2, arg3,
						arg4, arg5, arg6, arg7);
		} else if (arg3 == PSCI_CPU_SUSPEND_AARCH64) {
			return tsp_cpu_suspend_main(arg0, arg1, arg2, arg3,
						arg4, arg5, arg6, arg7);
		}
	} else if ((arg2 & FFA_FWK_MSG_MASK) == FFA_PM_MSG_WB_REQ) {
		/* Check it is a PSCI Warm Boot request. */
		if (arg3 == FFA_WB_TYPE_NOTS2RAM) {
			return tsp_cpu_resume_main(arg0, arg1, arg2, arg3,
						arg4, arg5, arg6, arg7);
		}
	}

err:
	ERROR("%s: Unknown framework message!\n", __func__);
	panic();
}

/*******************************************************************************
 * This function implements the event loop for handling FF-A ABI invocations.
 ******************************************************************************/
static smc_args_t *tsp_event_loop(uint64_t smc_fid,
				  uint64_t arg1,
				  uint64_t arg2,
				  uint64_t arg3,
				  uint64_t arg4,
				  uint64_t arg5,
				  uint64_t arg6,
				  uint64_t arg7)
{
	/* Panic if the SPMC did not forward an FF-A call. */
	if (!is_ffa_fid(smc_fid)) {
		ERROR("%s: Unknown SMC FID (0x%lx)\n", __func__, smc_fid);
		panic();
	}

	switch (smc_fid) {
	case FFA_INTERRUPT:
		/*
		 * IRQs were enabled upon re-entry into the TSP. The interrupt
		 * must have been handled by now. Return to the SPMC indicating
		 * the same.
		 */
		return set_smc_args(FFA_MSG_WAIT, 0, 0, 0, 0, 0, 0, 0);

	case FFA_MSG_SEND_DIRECT_REQ_SMC32:
		/* Check if a framework message, handle accordingly. */
		if ((arg2 & FFA_FWK_MSG_BIT)) {
			return handle_framework_message(smc_fid, arg1, arg2, arg3,
							arg4, arg5, arg6, arg7);
		}
	default:
		break;
	}

	ERROR("%s: Unsupported FF-A FID (0x%lx)\n", __func__, smc_fid);
	panic();
}

static smc_args_t *tsp_loop(smc_args_t *args)
{
	smc_args_t ret;

	do {
		/* --------------------------------------------
		 * Mask FIQ interrupts to avoid preemption
		 * in case EL3 SPMC delegates an IRQ next or a
		 * managed exit. Lastly, unmask IRQs so that
		 * they can be handled immediately upon re-entry
		 *  ---------------------------------------------
		 */
		write_daifset(DAIF_FIQ_BIT);
		write_daifclr(DAIF_IRQ_BIT);
		ret = smc_helper(args->_regs[0], args->_regs[1], args->_regs[2],
			       args->_regs[3], args->_regs[4], args->_regs[5],
			       args->_regs[6], args->_regs[7]);
		args = tsp_event_loop(ret._regs[0], ret._regs[1], ret._regs[2],
				      ret._regs[3], ret._regs[4], ret._regs[5],
				      ret._regs[6], ret._regs[7]);
	} while (1);

	/* Not Reached. */
	return NULL;
}

/*******************************************************************************
 * TSP main entry point where it gets the opportunity to initialize its secure
 * state/applications. Once the state is initialized, it must return to the
 * SPD with a pointer to the 'tsp_vector_table' jump table.
 ******************************************************************************/
uint64_t tsp_main(void)
{
	smc_args_t smc_args = {0};

	NOTICE("TSP: %s\n", version_string);
	NOTICE("TSP: %s\n", build_message);
	INFO("TSP: Total memory base : 0x%lx\n", (unsigned long) BL32_BASE);
	INFO("TSP: Total memory size : 0x%lx bytes\n", BL32_TOTAL_SIZE);
	uint32_t linear_id = plat_my_core_pos();

	/* Initialize the platform. */
	tsp_platform_setup();

	/* Initialize secure/applications state here. */
	tsp_generic_timer_start();

	/* Register secondary entrypoint with the SPMC. */
	smc_args = smc_helper(FFA_SECONDARY_EP_REGISTER_SMC64,
			(uint64_t) tsp_cpu_on_entry,
			0, 0, 0, 0, 0, 0);
	if (smc_args._regs[SMC_ARG0] != FFA_SUCCESS_SMC32) {
		ERROR("TSP could not register secondary ep (0x%lx)\n",
				smc_args._regs[2]);
		panic();
	}
	/* Get TSP's endpoint id */
	smc_args = smc_helper(FFA_ID_GET, 0, 0, 0, 0, 0, 0, 0);
	if (smc_args._regs[SMC_ARG0] != FFA_SUCCESS_SMC32) {
		ERROR("TSP could not get own ID (0x%lx) on core%d\n",
				smc_args._regs[2], linear_id);
		panic();
	}

	tsp_id = smc_args._regs[2];
	INFO("TSP FF-A endpoint id = 0x%x\n", tsp_id);
	/* Get the SPMC ID */
	smc_args = smc_helper(FFA_SPM_ID_GET, 0, 0, 0, 0, 0, 0, 0);
	if (smc_args._regs[SMC_ARG0] != FFA_SUCCESS_SMC32) {
		ERROR("TSP could not get SPMC ID (0x%lx) on core%d\n",
				smc_args._regs[2], linear_id);
		panic();
	}

	spmc_id = smc_args._regs[2];

	/* Update this cpu's statistics */
	tsp_stats[linear_id].smc_count++;
	tsp_stats[linear_id].eret_count++;
	tsp_stats[linear_id].cpu_on_count++;

	INFO("TSP: cpu 0x%lx: %d smcs, %d erets %d cpu on requests\n",
			read_mpidr(),
			tsp_stats[linear_id].smc_count,
			tsp_stats[linear_id].eret_count,
			tsp_stats[linear_id].cpu_on_count);

	/* Tell SPMD that we are done initialising. */
	tsp_loop(set_smc_args(FFA_MSG_WAIT, 0, 0, 0, 0, 0, 0, 0));

	/* Not reached. */
	return 0;
}

/*******************************************************************************
 * This function performs any remaining book keeping in the test secure payload
 * after this cpu's architectural state has been setup in response to an earlier
 * psci cpu_on request.
 ******************************************************************************/
smc_args_t *tsp_cpu_on_main(void)
{
	uint32_t linear_id = plat_my_core_pos();

	/* Initialize secure/applications state here. */
	tsp_generic_timer_start();

	/* Update this cpu's statistics. */
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
	/* ---------------------------------------------
	 * Jump to the main event loop to return to EL3
	 * and be ready for the next request on this cpu.
	 * ---------------------------------------------
	 */
	return tsp_loop(set_smc_args(FFA_MSG_WAIT, 0, 0, 0, 0, 0, 0, 0));
}
