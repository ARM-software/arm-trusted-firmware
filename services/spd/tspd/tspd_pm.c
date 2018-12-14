/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <bl32/tsp/tsp.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <plat/common/platform.h>

#include "tspd_private.h"

/*******************************************************************************
 * The target cpu is being turned on. Allow the TSPD/TSP to perform any actions
 * needed. Nothing at the moment.
 ******************************************************************************/
static void tspd_cpu_on_handler(u_register_t target_cpu)
{
}

/*******************************************************************************
 * This cpu is being turned off. Allow the TSPD/TSP to perform any actions
 * needed
 ******************************************************************************/
static int32_t tspd_cpu_off_handler(u_register_t unused)
{
	int32_t rc = 0;
	uint32_t linear_id = plat_my_core_pos();
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];

	assert(tsp_vectors);
	assert(get_tsp_pstate(tsp_ctx->state) == TSP_PSTATE_ON);

	/*
	 * Abort any preempted SMC request before overwriting the SECURE
	 * context.
	 */
	tspd_abort_preempted_smc(tsp_ctx);

	/* Program the entry point and enter the TSP */
	cm_set_elr_el3(SECURE, (uint64_t) &tsp_vectors->cpu_off_entry);
	rc = tspd_synchronous_sp_entry(tsp_ctx);

	/*
	 * Read the response from the TSP. A non-zero return means that
	 * something went wrong while communicating with the TSP.
	 */
	if (rc != 0)
		panic();

	/*
	 * Reset TSP's context for a fresh start when this cpu is turned on
	 * subsequently.
	 */
	set_tsp_pstate(tsp_ctx->state, TSP_PSTATE_OFF);

	return 0;
}

/*******************************************************************************
 * This cpu is being suspended. S-EL1 state must have been saved in the
 * resident cpu (mpidr format) if it is a UP/UP migratable TSP.
 ******************************************************************************/
static void tspd_cpu_suspend_handler(u_register_t max_off_pwrlvl)
{
	int32_t rc = 0;
	uint32_t linear_id = plat_my_core_pos();
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];

	assert(tsp_vectors);
	assert(get_tsp_pstate(tsp_ctx->state) == TSP_PSTATE_ON);

	/*
	 * Abort any preempted SMC request before overwriting the SECURE
	 * context.
	 */
	tspd_abort_preempted_smc(tsp_ctx);

	/* Program the entry point and enter the TSP */
	cm_set_elr_el3(SECURE, (uint64_t) &tsp_vectors->cpu_suspend_entry);
	rc = tspd_synchronous_sp_entry(tsp_ctx);

	/*
	 * Read the response from the TSP. A non-zero return means that
	 * something went wrong while communicating with the TSP.
	 */
	if (rc)
		panic();

	/* Update its context to reflect the state the TSP is in */
	set_tsp_pstate(tsp_ctx->state, TSP_PSTATE_SUSPEND);
}

/*******************************************************************************
 * This cpu has been turned on. Enter the TSP to initialise S-EL1 and other bits
 * before passing control back to the Secure Monitor. Entry in S-EL1 is done
 * after initialising minimal architectural state that guarantees safe
 * execution.
 ******************************************************************************/
static void tspd_cpu_on_finish_handler(u_register_t unused)
{
	int32_t rc = 0;
	uint32_t linear_id = plat_my_core_pos();
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];
	entry_point_info_t tsp_on_entrypoint;

	assert(tsp_vectors);
	assert(get_tsp_pstate(tsp_ctx->state) == TSP_PSTATE_OFF);

	tspd_init_tsp_ep_state(&tsp_on_entrypoint,
				TSP_AARCH64,
				(uint64_t) &tsp_vectors->cpu_on_entry,
				tsp_ctx);

	/* Initialise this cpu's secure context */
	cm_init_my_context(&tsp_on_entrypoint);

#if TSP_NS_INTR_ASYNC_PREEMPT
	/*
	 * Disable the NS interrupt locally since it will be enabled globally
	 * within cm_init_my_context.
	 */
	disable_intr_rm_local(INTR_TYPE_NS, SECURE);
#endif

	/* Enter the TSP */
	rc = tspd_synchronous_sp_entry(tsp_ctx);

	/*
	 * Read the response from the TSP. A non-zero return means that
	 * something went wrong while communicating with the SP.
	 */
	if (rc != 0)
		panic();

	/* Update its context to reflect the state the SP is in */
	set_tsp_pstate(tsp_ctx->state, TSP_PSTATE_ON);
}

/*******************************************************************************
 * This cpu has resumed from suspend. The SPD saved the TSP context when it
 * completed the preceding suspend call. Use that context to program an entry
 * into the TSP to allow it to do any remaining book keeping
 ******************************************************************************/
static void tspd_cpu_suspend_finish_handler(u_register_t max_off_pwrlvl)
{
	int32_t rc = 0;
	uint32_t linear_id = plat_my_core_pos();
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];

	assert(tsp_vectors);
	assert(get_tsp_pstate(tsp_ctx->state) == TSP_PSTATE_SUSPEND);

	/* Program the entry point, max_off_pwrlvl and enter the SP */
	write_ctx_reg(get_gpregs_ctx(&tsp_ctx->cpu_ctx),
		      CTX_GPREG_X0,
		      max_off_pwrlvl);
	cm_set_elr_el3(SECURE, (uint64_t) &tsp_vectors->cpu_resume_entry);
	rc = tspd_synchronous_sp_entry(tsp_ctx);

	/*
	 * Read the response from the TSP. A non-zero return means that
	 * something went wrong while communicating with the TSP.
	 */
	if (rc != 0)
		panic();

	/* Update its context to reflect the state the SP is in */
	set_tsp_pstate(tsp_ctx->state, TSP_PSTATE_ON);
}

/*******************************************************************************
 * Return the type of TSP the TSPD is dealing with. Report the current resident
 * cpu (mpidr format) if it is a UP/UP migratable TSP.
 ******************************************************************************/
static int32_t tspd_cpu_migrate_info(u_register_t *resident_cpu)
{
	return TSP_MIGRATE_INFO;
}

/*******************************************************************************
 * System is about to be switched off. Allow the TSPD/TSP to perform
 * any actions needed.
 ******************************************************************************/
static void tspd_system_off(void)
{
	uint32_t linear_id = plat_my_core_pos();
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];

	assert(tsp_vectors);
	assert(get_tsp_pstate(tsp_ctx->state) == TSP_PSTATE_ON);

	/*
	 * Abort any preempted SMC request before overwriting the SECURE
	 * context.
	 */
	tspd_abort_preempted_smc(tsp_ctx);

	/* Program the entry point */
	cm_set_elr_el3(SECURE, (uint64_t) &tsp_vectors->system_off_entry);

	/* Enter the TSP. We do not care about the return value because we
	 * must continue the shutdown anyway */
	tspd_synchronous_sp_entry(tsp_ctx);
}

/*******************************************************************************
 * System is about to be reset. Allow the TSPD/TSP to perform
 * any actions needed.
 ******************************************************************************/
static void tspd_system_reset(void)
{
	uint32_t linear_id = plat_my_core_pos();
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];

	assert(tsp_vectors);
	assert(get_tsp_pstate(tsp_ctx->state) == TSP_PSTATE_ON);

	/*
	 * Abort any preempted SMC request before overwriting the SECURE
	 * context.
	 */
	tspd_abort_preempted_smc(tsp_ctx);

	/* Program the entry point */
	cm_set_elr_el3(SECURE, (uint64_t) &tsp_vectors->system_reset_entry);

	/*
	 * Enter the TSP. We do not care about the return value because we
	 * must continue the reset anyway
	 */
	tspd_synchronous_sp_entry(tsp_ctx);
}

/*******************************************************************************
 * Structure populated by the TSP Dispatcher to be given a chance to perform any
 * TSP bookkeeping before PSCI executes a power mgmt.  operation.
 ******************************************************************************/
const spd_pm_ops_t tspd_pm = {
	.svc_on = tspd_cpu_on_handler,
	.svc_off = tspd_cpu_off_handler,
	.svc_suspend = tspd_cpu_suspend_handler,
	.svc_on_finish = tspd_cpu_on_finish_handler,
	.svc_suspend_finish = tspd_cpu_suspend_finish_handler,
	.svc_migrate = NULL,
	.svc_migrate_info = tspd_cpu_migrate_info,
	.svc_system_off = tspd_system_off,
	.svc_system_reset = tspd_system_reset
};
