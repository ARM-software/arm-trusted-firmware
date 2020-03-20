/*
 * Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <context.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <plat/common/platform.h>
#include <smccc_helpers.h>

#include "../bl1_private.h"

/*
 * Following arrays will be used for context management.
 * There are 2 instances, for the Secure and Non-Secure contexts.
 */
static cpu_context_t bl1_cpu_context[2];
static smc_ctx_t bl1_smc_context[2];

/* Following contains the next cpu context pointer. */
static void *bl1_next_cpu_context_ptr;

/* Following contains the next smc context pointer. */
static void *bl1_next_smc_context_ptr;

/* Following functions are used for SMC context handling */
void *smc_get_ctx(unsigned int security_state)
{
	assert(sec_state_is_valid(security_state));
	return &bl1_smc_context[security_state];
}

void smc_set_next_ctx(unsigned int security_state)
{
	assert(sec_state_is_valid(security_state));
	bl1_next_smc_context_ptr = &bl1_smc_context[security_state];
}

void *smc_get_next_ctx(void)
{
	return bl1_next_smc_context_ptr;
}

/* Following functions are used for CPU context handling */
void *cm_get_context(uint32_t security_state)
{
	assert(sec_state_is_valid(security_state));
	return &bl1_cpu_context[security_state];
}

void cm_set_next_context(void *context)
{
	assert(context != NULL);
	bl1_next_cpu_context_ptr = context;
}

void *cm_get_next_context(void)
{
	return bl1_next_cpu_context_ptr;
}

/*******************************************************************************
 * Following function copies GP regs r0-r4, lr and spsr,
 * from the CPU context to the SMC context structures.
 ******************************************************************************/
static void copy_cpu_ctx_to_smc_ctx(const regs_t *cpu_reg_ctx,
		smc_ctx_t *next_smc_ctx)
{
	next_smc_ctx->r0 = read_ctx_reg(cpu_reg_ctx, CTX_GPREG_R0);
	next_smc_ctx->r1 = read_ctx_reg(cpu_reg_ctx, CTX_GPREG_R1);
	next_smc_ctx->r2 = read_ctx_reg(cpu_reg_ctx, CTX_GPREG_R2);
	next_smc_ctx->r3 = read_ctx_reg(cpu_reg_ctx, CTX_GPREG_R3);
	next_smc_ctx->lr_mon = read_ctx_reg(cpu_reg_ctx, CTX_LR);
	next_smc_ctx->spsr_mon = read_ctx_reg(cpu_reg_ctx, CTX_SPSR);
	next_smc_ctx->scr = read_ctx_reg(cpu_reg_ctx, CTX_SCR);
}

/*******************************************************************************
 * Following function flushes the SMC & CPU context pointer and its data.
 ******************************************************************************/
static void flush_smc_and_cpu_ctx(void)
{
	flush_dcache_range((uintptr_t)&bl1_next_smc_context_ptr,
		sizeof(bl1_next_smc_context_ptr));
	flush_dcache_range((uintptr_t)bl1_next_smc_context_ptr,
		sizeof(smc_ctx_t));

	flush_dcache_range((uintptr_t)&bl1_next_cpu_context_ptr,
		sizeof(bl1_next_cpu_context_ptr));
	flush_dcache_range((uintptr_t)bl1_next_cpu_context_ptr,
		sizeof(cpu_context_t));
}

/*******************************************************************************
 * This function prepares the context for Secure/Normal world images.
 * Normal world images are transitioned to HYP(if supported) else SVC.
 ******************************************************************************/
void bl1_prepare_next_image(unsigned int image_id)
{
	unsigned int security_state, mode = MODE32_svc;
	image_desc_t *desc;
	entry_point_info_t *next_bl_ep;

	/* Get the image descriptor. */
	desc = bl1_plat_get_image_desc(image_id);
	assert(desc != NULL);

	/* Get the entry point info. */
	next_bl_ep = &desc->ep_info;

	/* Get the image security state. */
	security_state = GET_SECURITY_STATE(next_bl_ep->h.attr);

	/* Prepare the SPSR for the next BL image. */
	if ((security_state != SECURE) && (GET_VIRT_EXT(read_id_pfr1()) != 0U)) {
		mode = MODE32_hyp;
	}

	next_bl_ep->spsr = SPSR_MODE32(mode, SPSR_T_ARM,
				SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS);

	/* Allow platform to make change */
	bl1_plat_set_ep_info(image_id, next_bl_ep);

	/* Prepare the cpu context for the next BL image. */
	cm_init_my_context(next_bl_ep);
	cm_prepare_el3_exit(security_state);
	cm_set_next_context(cm_get_context(security_state));

	/* Prepare the smc context for the next BL image. */
	smc_set_next_ctx(security_state);
	copy_cpu_ctx_to_smc_ctx(get_regs_ctx(cm_get_next_context()),
		smc_get_next_ctx());

	/*
	 * If the next image is non-secure, then we need to program the banked
	 * non secure sctlr. This is not required when the next image is secure
	 * because in AArch32, we expect the secure world to have the same
	 * SCTLR settings.
	 */
	if (security_state == NON_SECURE) {
		cpu_context_t *ctx = cm_get_context(security_state);
		u_register_t ns_sctlr;

		/* Temporarily set the NS bit to access NS SCTLR */
		write_scr(read_scr() | SCR_NS_BIT);
		isb();

		ns_sctlr = read_ctx_reg(get_regs_ctx(ctx), CTX_NS_SCTLR);
		write_sctlr(ns_sctlr);
		isb();

		write_scr(read_scr() & ~SCR_NS_BIT);
		isb();
	}

	/*
	 * Flush the SMC & CPU context and the (next)pointers,
	 * to access them after caches are disabled.
	 */
	flush_smc_and_cpu_ctx();

	/* Indicate that image is in execution state. */
	desc->state = IMAGE_STATE_EXECUTED;

	print_entry_point_info(next_bl_ep);
}
