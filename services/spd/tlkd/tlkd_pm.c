/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <debug.h>
#include <psci.h>
#include <tlk.h>

#include "tlkd_private.h"

extern tlk_context_t tlk_ctx;

#define MPIDR_CPU0	0x80000000

/*******************************************************************************
 * Return the type of payload TLKD is dealing with. Report the current
 * resident cpu (mpidr format) if it is a UP/UP migratable payload.
 ******************************************************************************/
static int32_t cpu_migrate_info(uint64_t *resident_cpu)
{
	/* the payload runs only on CPU0 */
	*resident_cpu = MPIDR_CPU0;

	/* Uniprocessor, not migrate capable payload */
	return PSCI_TOS_NOT_UP_MIG_CAP;
}

/*******************************************************************************
 * This cpu is being suspended. Inform TLK of the SYSTEM_SUSPEND event, so
 * that it can pass this information to its Trusted Apps.
 ******************************************************************************/
static void cpu_suspend_handler(uint64_t suspend_level)
{
	gp_regs_t *gp_regs;
	int cpu = read_mpidr() & MPIDR_CPU_MASK;
	int32_t rc = 0;

	/*
	 * TLK runs only on CPU0 and suspends its Trusted Apps during
	 * SYSTEM_SUSPEND. It has no role to play during CPU_SUSPEND.
	 */
	if ((cpu != 0) || (suspend_level != PLAT_MAX_PWR_LVL))
		return;

	/* pass system suspend event to TLK */
	gp_regs = get_gpregs_ctx(&tlk_ctx.cpu_ctx);
	write_ctx_reg(gp_regs, CTX_GPREG_X0, TLK_SYSTEM_SUSPEND);

	/* Program the entry point and enter TLK */
	rc = tlkd_synchronous_sp_entry(&tlk_ctx);

	/*
	 * Read the response from TLK. A non-zero return means that
	 * something went wrong while communicating with it.
	 */
	if (rc != 0)
		panic();
}

/*******************************************************************************
 * This cpu is being resumed. Inform TLK of the SYSTEM_SUSPEND exit, so
 * that it can pass this information to its Trusted Apps.
 ******************************************************************************/
static void cpu_resume_handler(uint64_t suspend_level)
{
	gp_regs_t *gp_regs;
	int cpu = read_mpidr() & MPIDR_CPU_MASK;
	int32_t rc = 0;

	/*
	 * TLK runs only on CPU0 and resumes its Trusted Apps during
	 * SYSTEM_SUSPEND exit. It has no role to play during CPU_SUSPEND
	 * exit.
	 */
	if ((cpu != 0) || (suspend_level != PLAT_MAX_PWR_LVL))
		return;

	/* pass system resume event to TLK */
	gp_regs = get_gpregs_ctx(&tlk_ctx.cpu_ctx);
	write_ctx_reg(gp_regs, CTX_GPREG_X0, TLK_SYSTEM_RESUME);

	/* Program the entry point and enter TLK */
	rc = tlkd_synchronous_sp_entry(&tlk_ctx);

	/*
	 * Read the response from TLK. A non-zero return means that
	 * something went wrong while communicating with it.
	 */
	if (rc != 0)
		panic();
}

/*******************************************************************************
 * System is about to be reset. Inform the SP to allow any book-keeping
 ******************************************************************************/
static void system_off_handler(void)
{
	int cpu = read_mpidr() & MPIDR_CPU_MASK;
	gp_regs_t *gp_regs;

	/* TLK runs only on CPU0 */
	if (cpu != 0)
		return;

	/* pass system off/reset events to TLK */
	gp_regs = get_gpregs_ctx(&tlk_ctx.cpu_ctx);
	write_ctx_reg(gp_regs, CTX_GPREG_X0, TLK_SYSTEM_OFF);

	/*
	 * Enter the SP. We do not care about the return value because we
	 * must continue with the shutdown anyway.
	 */
	(void)tlkd_synchronous_sp_entry(&tlk_ctx);
}

/*******************************************************************************
 * Structure populated by the Dispatcher to be given a chance to perform any
 * bookkeeping before PSCI executes a power mgmt.  operation.
 ******************************************************************************/
const spd_pm_ops_t tlkd_pm_ops = {
	.svc_migrate_info = cpu_migrate_info,
	.svc_suspend = cpu_suspend_handler,
	.svc_suspend_finish = cpu_resume_handler,
	.svc_system_off = system_off_handler,
	.svc_system_reset = system_off_handler
};
