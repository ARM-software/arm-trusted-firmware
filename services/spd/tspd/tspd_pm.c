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

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <debug.h>
#include <platform.h>
#include <tsp.h>
#include "tspd_private.h"

/*******************************************************************************
 * The target cpu is being turned on. Allow the TSPD/TSP to perform any actions
 * needed. Nothing at the moment.
 ******************************************************************************/
static void tspd_cpu_on_handler(uint64_t target_cpu)
{
}

/*******************************************************************************
 * This cpu is being turned off. Allow the TSPD/TSP to perform any actions
 * needed
 ******************************************************************************/
static int32_t tspd_cpu_off_handler(uint64_t cookie)
{
	int32_t rc = 0;
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];

	assert(tsp_vectors);
	assert(get_tsp_pstate(tsp_ctx->state) == TSP_PSTATE_ON);

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
static void tspd_cpu_suspend_handler(uint64_t power_state)
{
	int32_t rc = 0;
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];

	assert(tsp_vectors);
	assert(get_tsp_pstate(tsp_ctx->state) == TSP_PSTATE_ON);

	/* Program the entry point, power_state parameter and enter the TSP */
	write_ctx_reg(get_gpregs_ctx(&tsp_ctx->cpu_ctx),
		      CTX_GPREG_X0,
		      power_state);
	cm_set_elr_el3(SECURE, (uint64_t) &tsp_vectors->cpu_suspend_entry);
	rc = tspd_synchronous_sp_entry(tsp_ctx);

	/*
	 * Read the response from the TSP. A non-zero return means that
	 * something went wrong while communicating with the TSP.
	 */
	if (rc != 0)
		panic();

	/* Update its context to reflect the state the TSP is in */
	set_tsp_pstate(tsp_ctx->state, TSP_PSTATE_SUSPEND);
}

/*******************************************************************************
 * This cpu has been turned on. Enter the TSP to initialise S-EL1 and other bits
 * before passing control back to the Secure Monitor. Entry in S-El1 is done
 * after initialising minimal architectural state that guarantees safe
 * execution.
 ******************************************************************************/
static void tspd_cpu_on_finish_handler(uint64_t cookie)
{
	int32_t rc = 0;
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];
	entry_point_info_t tsp_on_entrypoint;

	assert(tsp_vectors);
	assert(get_tsp_pstate(tsp_ctx->state) == TSP_PSTATE_OFF);

	tspd_init_tsp_ep_state(&tsp_on_entrypoint,
				TSP_AARCH64,
				(uint64_t) &tsp_vectors->cpu_on_entry,
				tsp_ctx);

	/* Initialise this cpu's secure context */
	cm_init_context(mpidr, &tsp_on_entrypoint);

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
static void tspd_cpu_suspend_finish_handler(uint64_t suspend_level)
{
	int32_t rc = 0;
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];

	assert(tsp_vectors);
	assert(get_tsp_pstate(tsp_ctx->state) == TSP_PSTATE_SUSPEND);

	/* Program the entry point, suspend_level and enter the SP */
	write_ctx_reg(get_gpregs_ctx(&tsp_ctx->cpu_ctx),
		      CTX_GPREG_X0,
		      suspend_level);
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
static int32_t tspd_cpu_migrate_info(uint64_t *resident_cpu)
{
	return TSP_MIGRATE_INFO;
}

/*******************************************************************************
 * System is about to be switched off. Allow the TSPD/TSP to perform
 * any actions needed.
 ******************************************************************************/
static void tspd_system_off(void)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];

	assert(tsp_vectors);
	assert(get_tsp_pstate(tsp_ctx->state) == TSP_PSTATE_ON);

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
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];

	assert(tsp_vectors);
	assert(get_tsp_pstate(tsp_ctx->state) == TSP_PSTATE_ON);

	/* Program the entry point */
	cm_set_elr_el3(SECURE, (uint64_t) &tsp_vectors->system_reset_entry);

	/* Enter the TSP. We do not care about the return value because we
	 * must continue the reset anyway */
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
