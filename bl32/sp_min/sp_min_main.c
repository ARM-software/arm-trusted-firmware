/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <platform.h>
#include <platform_def.h>
#include <platform_sp_min.h>
#include <psci.h>
#include <runtime_svc.h>
#include <smcc_helpers.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <types.h>
#include "sp_min_private.h"

/* Pointers to per-core cpu contexts */
static void *sp_min_cpu_ctx_ptr[PLATFORM_CORE_COUNT];

/* SP_MIN only stores the non secure smc context */
static smc_ctx_t sp_min_smc_context[PLATFORM_CORE_COUNT];

/******************************************************************************
 * Define the smcc helper library API's
 *****************************************************************************/
void *smc_get_ctx(int security_state)
{
	assert(security_state == NON_SECURE);
	return &sp_min_smc_context[plat_my_core_pos()];
}

void smc_set_next_ctx(int security_state)
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
	next_smc_ctx->lr_mon = read_ctx_reg(cpu_reg_ctx, CTX_LR);
	next_smc_ctx->spsr_mon = read_ctx_reg(cpu_reg_ctx, CTX_SPSR);
}

/*******************************************************************************
 * This function invokes the PSCI library interface to initialize the
 * non secure cpu context and copies the relevant cpu context register values
 * to smc context. These registers will get programmed during `smc_exit`.
 ******************************************************************************/
static void sp_min_prepare_next_image_entry(void)
{
	entry_point_info_t *next_image_info;

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

	psci_warmboot_entrypoint();

	smc_set_next_ctx(NON_SECURE);

	next_smc_ctx = smc_get_next_ctx();
	memset(next_smc_ctx, 0, sizeof(smc_ctx_t));

	copy_cpu_ctx_to_smc_stx(get_regs_ctx(cm_get_context(NON_SECURE)),
			next_smc_ctx);
}
