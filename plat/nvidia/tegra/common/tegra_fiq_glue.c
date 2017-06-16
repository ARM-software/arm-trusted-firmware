/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bakery_lock.h>
#include <bl_common.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <denver.h>
#include <gic_v2.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <tegra_def.h>
#include <tegra_private.h>

static DEFINE_BAKERY_LOCK(tegra_fiq_lock);

/*******************************************************************************
 * Static variables
 ******************************************************************************/
static uint64_t ns_fiq_handler_addr;
static uint32_t fiq_handler_active;
static pcpu_fiq_state_t fiq_state[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * Handler for FIQ interrupts
 ******************************************************************************/
static uint64_t tegra_fiq_interrupt_handler(uint32_t id,
					  uint32_t flags,
					  void *handle,
					  void *cookie)
{
	cpu_context_t *ctx = cm_get_context(NON_SECURE);
	el3_state_t *el3state_ctx = get_el3state_ctx(ctx);
	uint32_t cpu = plat_my_core_pos();
	uint32_t irq;

	bakery_lock_get(&tegra_fiq_lock);

	/*
	 * The FIQ was generated when the execution was in the non-secure
	 * world. Save the context registers to start with.
	 */
	cm_el1_sysregs_context_save(NON_SECURE);

	/*
	 * Save elr_el3 and spsr_el3 from the saved context, and overwrite
	 * the context with the NS fiq_handler_addr and SPSR value.
	 */
	fiq_state[cpu].elr_el3 = read_ctx_reg((el3state_ctx), (uint32_t)(CTX_ELR_EL3));
	fiq_state[cpu].spsr_el3 = read_ctx_reg((el3state_ctx), (uint32_t)(CTX_SPSR_EL3));

	/*
	 * Set the new ELR to continue execution in the NS world using the
	 * FIQ handler registered earlier.
	 */
	assert(ns_fiq_handler_addr);
	write_ctx_reg((el3state_ctx), (uint32_t)(CTX_ELR_EL3), (ns_fiq_handler_addr));

	/*
	 * Mark this interrupt as complete to avoid a FIQ storm.
	 */
	irq = plat_ic_acknowledge_interrupt();
	if (irq < 1022U) {
		plat_ic_end_of_interrupt(irq);
	}

	bakery_lock_release(&tegra_fiq_lock);

	return 0;
}

/*******************************************************************************
 * Setup handler for FIQ interrupts
 ******************************************************************************/
void tegra_fiq_handler_setup(void)
{
	uint32_t flags;
	int32_t rc;

	/* return if already registered */
	if (fiq_handler_active == 0U) {
		/*
		 * Register an interrupt handler for FIQ interrupts generated for
		 * NS interrupt sources
		 */
		flags = 0U;
		set_interrupt_rm_flag((flags), (NON_SECURE));
		rc = register_interrupt_type_handler(INTR_TYPE_EL3,
					tegra_fiq_interrupt_handler,
					flags);
		if (rc != 0) {
			panic();
		}

		/* handler is now active */
		fiq_handler_active = 1;
	}
}

/*******************************************************************************
 * Validate and store NS world's entrypoint for FIQ interrupts
 ******************************************************************************/
void tegra_fiq_set_ns_entrypoint(uint64_t entrypoint)
{
	ns_fiq_handler_addr = entrypoint;
}

/*******************************************************************************
 * Handler to return the NS EL1/EL0 CPU context
 ******************************************************************************/
int32_t tegra_fiq_get_intr_context(void)
{
	cpu_context_t *ctx = cm_get_context(NON_SECURE);
	gp_regs_t *gpregs_ctx = get_gpregs_ctx(ctx);
	const el1_sys_regs_t *el1state_ctx = get_sysregs_ctx(ctx);
	uint32_t cpu = plat_my_core_pos();
	uint64_t val;

	/*
	 * We store the ELR_EL3, SPSR_EL3, SP_EL0 and SP_EL1 registers so
	 * that el3_exit() sends these values back to the NS world.
	 */
	write_ctx_reg((gpregs_ctx), (uint32_t)(CTX_GPREG_X0), (fiq_state[cpu].elr_el3));
	write_ctx_reg((gpregs_ctx), (uint32_t)(CTX_GPREG_X1), (fiq_state[cpu].spsr_el3));

	val = read_ctx_reg((gpregs_ctx), (uint32_t)(CTX_GPREG_SP_EL0));
	write_ctx_reg((gpregs_ctx), (uint32_t)(CTX_GPREG_X2), (val));

	val = read_ctx_reg((el1state_ctx), (uint32_t)(CTX_SP_EL1));
	write_ctx_reg((gpregs_ctx), (uint32_t)(CTX_GPREG_X3), (val));

	return 0;
}
