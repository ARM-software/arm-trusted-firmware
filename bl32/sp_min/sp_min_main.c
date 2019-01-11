/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <context.h>
#include <drivers/console.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/psci/psci.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <platform_sp_min.h>
#include <services/std_svc.h>
#include <smccc_helpers.h>

#include "sp_min_private.h"

/* Pointers to per-core cpu contexts */
static void *sp_min_cpu_ctx_ptr[PLATFORM_CORE_COUNT];

/* SP_MIN only stores the non secure smc context */
static smc_ctx_t sp_min_smc_context[PLATFORM_CORE_COUNT];

/******************************************************************************
 * Define the smccc helper library APIs
 *****************************************************************************/
void *smc_get_ctx(unsigned int security_state)
{
	assert(security_state == NON_SECURE);
	return &sp_min_smc_context[plat_my_core_pos()];
}

void smc_set_next_ctx(unsigned int security_state)
{
	assert(security_state == NON_SECURE);
	/* SP_MIN stores only non secure smc context. Nothing to do here */
}

void *smc_get_next_ctx(void)
{
	return &sp_min_smc_context[plat_my_core_pos()];
}

/*******************************************************************************
 * This function returns a pointer to the most recent 'cpu_context' structure
 * for the calling CPU that was set as the context for the specified security
 * state. NULL is returned if no such structure has been specified.
 ******************************************************************************/
void *cm_get_context(uint32_t security_state)
{
	assert(security_state == NON_SECURE);
	return sp_min_cpu_ctx_ptr[plat_my_core_pos()];
}

/*******************************************************************************
 * This function sets the pointer to the current 'cpu_context' structure for the
 * specified security state for the calling CPU
 ******************************************************************************/
void cm_set_context(void *context, uint32_t security_state)
{
	assert(security_state == NON_SECURE);
	sp_min_cpu_ctx_ptr[plat_my_core_pos()] = context;
}

/*******************************************************************************
 * This function returns a pointer to the most recent 'cpu_context' structure
 * for the CPU identified by `cpu_idx` that was set as the context for the
 * specified security state. NULL is returned if no such structure has been
 * specified.
 ******************************************************************************/
void *cm_get_context_by_index(unsigned int cpu_idx,
				unsigned int security_state)
{
	assert(security_state == NON_SECURE);
	return sp_min_cpu_ctx_ptr[cpu_idx];
}

/*******************************************************************************
 * This function sets the pointer to the current 'cpu_context' structure for the
 * specified security state for the CPU identified by CPU index.
 ******************************************************************************/
void cm_set_context_by_index(unsigned int cpu_idx, void *context,
				unsigned int security_state)
{
	assert(security_state == NON_SECURE);
	sp_min_cpu_ctx_ptr[cpu_idx] = context;
}

static void copy_cpu_ctx_to_smc_stx(const regs_t *cpu_reg_ctx,
				smc_ctx_t *next_smc_ctx)
{
	next_smc_ctx->r0 = read_ctx_reg(cpu_reg_ctx, CTX_GPREG_R0);
	next_smc_ctx->r1 = read_ctx_reg(cpu_reg_ctx, CTX_GPREG_R1);
	next_smc_ctx->r2 = read_ctx_reg(cpu_reg_ctx, CTX_GPREG_R2);
	next_smc_ctx->lr_mon = read_ctx_reg(cpu_reg_ctx, CTX_LR);
	next_smc_ctx->spsr_mon = read_ctx_reg(cpu_reg_ctx, CTX_SPSR);
	next_smc_ctx->scr = read_ctx_reg(cpu_reg_ctx, CTX_SCR);
}

/*******************************************************************************
 * This function invokes the PSCI library interface to initialize the
 * non secure cpu context and copies the relevant cpu context register values
 * to smc context. These registers will get programmed during `smc_exit`.
 ******************************************************************************/
static void sp_min_prepare_next_image_entry(void)
{
	entry_point_info_t *next_image_info;
	cpu_context_t *ctx = cm_get_context(NON_SECURE);
	u_register_t ns_sctlr;

	/* Program system registers to proceed to non-secure */
	next_image_info = sp_min_plat_get_bl33_ep_info();
	assert(next_image_info);
	assert(NON_SECURE == GET_SECURITY_STATE(next_image_info->h.attr));

	INFO("SP_MIN: Preparing exit to normal world\n");

	psci_prepare_next_non_secure_ctx(next_image_info);
	smc_set_next_ctx(NON_SECURE);

	/* Copy r0, lr and spsr from cpu context to SMC context */
	copy_cpu_ctx_to_smc_stx(get_regs_ctx(cm_get_context(NON_SECURE)),
			smc_get_next_ctx());

	/* Temporarily set the NS bit to access NS SCTLR */
	write_scr(read_scr() | SCR_NS_BIT);
	isb();
	ns_sctlr = read_ctx_reg(get_regs_ctx(ctx), CTX_NS_SCTLR);
	write_sctlr(ns_sctlr);
	isb();

	write_scr(read_scr() & ~SCR_NS_BIT);
	isb();
}

/******************************************************************************
 * Implement the ARM Standard Service function to get arguments for a
 * particular service.
 *****************************************************************************/
uintptr_t get_arm_std_svc_args(unsigned int svc_mask)
{
	/* Setup the arguments for PSCI Library */
	DEFINE_STATIC_PSCI_LIB_ARGS_V1(psci_args, sp_min_warm_entrypoint);

	/* PSCI is the only ARM Standard Service implemented */
	assert(svc_mask == PSCI_FID_MASK);

	return (uintptr_t)&psci_args;
}

/******************************************************************************
 * The SP_MIN main function. Do the platform and PSCI Library setup. Also
 * initialize the runtime service framework.
 *****************************************************************************/
void sp_min_main(void)
{
	NOTICE("SP_MIN: %s\n", version_string);
	NOTICE("SP_MIN: %s\n", build_message);

	/* Perform the SP_MIN platform setup */
	sp_min_platform_setup();

	/* Initialize the runtime services e.g. psci */
	INFO("SP_MIN: Initializing runtime services\n");
	runtime_svc_init();

	/*
	 * We are ready to enter the next EL. Prepare entry into the image
	 * corresponding to the desired security state after the next ERET.
	 */
	sp_min_prepare_next_image_entry();

	/*
	 * Perform any platform specific runtime setup prior to cold boot exit
	 * from SP_MIN.
	 */
	sp_min_plat_runtime_setup();

	console_flush();
}

/******************************************************************************
 * This function is invoked during warm boot. Invoke the PSCI library
 * warm boot entry point which takes care of Architectural and platform setup/
 * restore. Copy the relevant cpu_context register values to smc context which
 * will get programmed during `smc_exit`.
 *****************************************************************************/
void sp_min_warm_boot(void)
{
	smc_ctx_t *next_smc_ctx;
	cpu_context_t *ctx = cm_get_context(NON_SECURE);
	u_register_t ns_sctlr;

	psci_warmboot_entrypoint();

	smc_set_next_ctx(NON_SECURE);

	next_smc_ctx = smc_get_next_ctx();
	zeromem(next_smc_ctx, sizeof(smc_ctx_t));

	copy_cpu_ctx_to_smc_stx(get_regs_ctx(cm_get_context(NON_SECURE)),
			next_smc_ctx);

	/* Temporarily set the NS bit to access NS SCTLR */
	write_scr(read_scr() | SCR_NS_BIT);
	isb();
	ns_sctlr = read_ctx_reg(get_regs_ctx(ctx), CTX_NS_SCTLR);
	write_sctlr(ns_sctlr);
	isb();

	write_scr(read_scr() & ~SCR_NS_BIT);
	isb();
}

#if SP_MIN_WITH_SECURE_FIQ
/******************************************************************************
 * This function is invoked on secure interrupts. By construction of the
 * SP_MIN, secure interrupts can only be handled when core executes in non
 * secure state.
 *****************************************************************************/
void sp_min_fiq(void)
{
	uint32_t id;

	id = plat_ic_acknowledge_interrupt();
	sp_min_plat_fiq_handler(id);
	plat_ic_end_of_interrupt(id);
}
#endif /* SP_MIN_WITH_SECURE_FIQ */
