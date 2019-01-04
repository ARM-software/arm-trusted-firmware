/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <plat/common/platform.h>

#include "opteed_private.h"

/*******************************************************************************
 * The target cpu is being turned on. Allow the OPTEED/OPTEE to perform any
 * actions needed. Nothing at the moment.
 ******************************************************************************/
static void opteed_cpu_on_handler(u_register_t target_cpu)
{
}

/*******************************************************************************
 * This cpu is being turned off. Allow the OPTEED/OPTEE to perform any actions
 * needed
 ******************************************************************************/
static int32_t opteed_cpu_off_handler(u_register_t unused)
{
	int32_t rc = 0;
	uint32_t linear_id = plat_my_core_pos();
	optee_context_t *optee_ctx = &opteed_sp_context[linear_id];

	assert(optee_vector_table);
	assert(get_optee_pstate(optee_ctx->state) == OPTEE_PSTATE_ON);

	/* Program the entry point and enter OPTEE */
	cm_set_elr_el3(SECURE, (uint64_t) &optee_vector_table->cpu_off_entry);
	rc = opteed_synchronous_sp_entry(optee_ctx);

	/*
	 * Read the response from OPTEE. A non-zero return means that
	 * something went wrong while communicating with OPTEE.
	 */
	if (rc != 0)
		panic();

	/*
	 * Reset OPTEE's context for a fresh start when this cpu is turned on
	 * subsequently.
	 */
	set_optee_pstate(optee_ctx->state, OPTEE_PSTATE_OFF);

	 return 0;
}

/*******************************************************************************
 * This cpu is being suspended. S-EL1 state must have been saved in the
 * resident cpu (mpidr format) if it is a UP/UP migratable OPTEE.
 ******************************************************************************/
static void opteed_cpu_suspend_handler(u_register_t max_off_pwrlvl)
{
	int32_t rc = 0;
	uint32_t linear_id = plat_my_core_pos();
	optee_context_t *optee_ctx = &opteed_sp_context[linear_id];

	assert(optee_vector_table);
	assert(get_optee_pstate(optee_ctx->state) == OPTEE_PSTATE_ON);

	write_ctx_reg(get_gpregs_ctx(&optee_ctx->cpu_ctx), CTX_GPREG_X0,
		      max_off_pwrlvl);

	/* Program the entry point and enter OPTEE */
	cm_set_elr_el3(SECURE, (uint64_t) &optee_vector_table->cpu_suspend_entry);
	rc = opteed_synchronous_sp_entry(optee_ctx);

	/*
	 * Read the response from OPTEE. A non-zero return means that
	 * something went wrong while communicating with OPTEE.
	 */
	if (rc != 0)
		panic();

	/* Update its context to reflect the state OPTEE is in */
	set_optee_pstate(optee_ctx->state, OPTEE_PSTATE_SUSPEND);
}

/*******************************************************************************
 * This cpu has been turned on. Enter OPTEE to initialise S-EL1 and other bits
 * before passing control back to the Secure Monitor. Entry in S-El1 is done
 * after initialising minimal architectural state that guarantees safe
 * execution.
 ******************************************************************************/
static void opteed_cpu_on_finish_handler(u_register_t unused)
{
	int32_t rc = 0;
	uint32_t linear_id = plat_my_core_pos();
	optee_context_t *optee_ctx = &opteed_sp_context[linear_id];
	entry_point_info_t optee_on_entrypoint;

	assert(optee_vector_table);
	assert(get_optee_pstate(optee_ctx->state) == OPTEE_PSTATE_OFF);

	opteed_init_optee_ep_state(&optee_on_entrypoint, opteed_rw,
				(uint64_t)&optee_vector_table->cpu_on_entry,
				0, 0, 0, optee_ctx);

	/* Initialise this cpu's secure context */
	cm_init_my_context(&optee_on_entrypoint);

	/* Enter OPTEE */
	rc = opteed_synchronous_sp_entry(optee_ctx);

	/*
	 * Read the response from OPTEE. A non-zero return means that
	 * something went wrong while communicating with OPTEE.
	 */
	if (rc != 0)
		panic();

	/* Update its context to reflect the state OPTEE is in */
	set_optee_pstate(optee_ctx->state, OPTEE_PSTATE_ON);
}

/*******************************************************************************
 * This cpu has resumed from suspend. The OPTEED saved the OPTEE context when it
 * completed the preceding suspend call. Use that context to program an entry
 * into OPTEE to allow it to do any remaining book keeping
 ******************************************************************************/
static void opteed_cpu_suspend_finish_handler(u_register_t max_off_pwrlvl)
{
	int32_t rc = 0;
	uint32_t linear_id = plat_my_core_pos();
	optee_context_t *optee_ctx = &opteed_sp_context[linear_id];

	assert(optee_vector_table);
	assert(get_optee_pstate(optee_ctx->state) == OPTEE_PSTATE_SUSPEND);

	/* Program the entry point, max_off_pwrlvl and enter the SP */
	write_ctx_reg(get_gpregs_ctx(&optee_ctx->cpu_ctx),
		      CTX_GPREG_X0,
		      max_off_pwrlvl);
	cm_set_elr_el3(SECURE, (uint64_t) &optee_vector_table->cpu_resume_entry);
	rc = opteed_synchronous_sp_entry(optee_ctx);

	/*
	 * Read the response from OPTEE. A non-zero return means that
	 * something went wrong while communicating with OPTEE.
	 */
	if (rc != 0)
		panic();

	/* Update its context to reflect the state OPTEE is in */
	set_optee_pstate(optee_ctx->state, OPTEE_PSTATE_ON);
}

/*******************************************************************************
 * Return the type of OPTEE the OPTEED is dealing with. Report the current
 * resident cpu (mpidr format) if it is a UP/UP migratable OPTEE.
 ******************************************************************************/
static int32_t opteed_cpu_migrate_info(u_register_t *resident_cpu)
{
	return OPTEE_MIGRATE_INFO;
}

/*******************************************************************************
 * System is about to be switched off. Allow the OPTEED/OPTEE to perform
 * any actions needed.
 ******************************************************************************/
static void opteed_system_off(void)
{
	uint32_t linear_id = plat_my_core_pos();
	optee_context_t *optee_ctx = &opteed_sp_context[linear_id];

	assert(optee_vector_table);
	assert(get_optee_pstate(optee_ctx->state) == OPTEE_PSTATE_ON);

	/* Program the entry point */
	cm_set_elr_el3(SECURE, (uint64_t) &optee_vector_table->system_off_entry);

	/* Enter OPTEE. We do not care about the return value because we
	 * must continue the shutdown anyway */
	opteed_synchronous_sp_entry(optee_ctx);
}

/*******************************************************************************
 * System is about to be reset. Allow the OPTEED/OPTEE to perform
 * any actions needed.
 ******************************************************************************/
static void opteed_system_reset(void)
{
	uint32_t linear_id = plat_my_core_pos();
	optee_context_t *optee_ctx = &opteed_sp_context[linear_id];

	assert(optee_vector_table);
	assert(get_optee_pstate(optee_ctx->state) == OPTEE_PSTATE_ON);

	/* Program the entry point */
	cm_set_elr_el3(SECURE, (uint64_t) &optee_vector_table->system_reset_entry);

	/* Enter OPTEE. We do not care about the return value because we
	 * must continue the reset anyway */
	opteed_synchronous_sp_entry(optee_ctx);
}


/*******************************************************************************
 * Structure populated by the OPTEE Dispatcher to be given a chance to
 * perform any OPTEE bookkeeping before PSCI executes a power mgmt.
 * operation.
 ******************************************************************************/
const spd_pm_ops_t opteed_pm = {
	.svc_on = opteed_cpu_on_handler,
	.svc_off = opteed_cpu_off_handler,
	.svc_suspend = opteed_cpu_suspend_handler,
	.svc_on_finish = opteed_cpu_on_finish_handler,
	.svc_suspend_finish = opteed_cpu_suspend_finish_handler,
	.svc_migrate = NULL,
	.svc_migrate_info = opteed_cpu_migrate_info,
	.svc_system_off = opteed_system_off,
	.svc_system_reset = opteed_system_reset,
};
