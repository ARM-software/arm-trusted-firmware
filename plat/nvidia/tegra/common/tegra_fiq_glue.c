/*
 * Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <bl31/interrupt_mgmt.h>
#include <bl31/ehf.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <denver.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <plat/common/platform.h>

#if ENABLE_WDT_LEGACY_FIQ_HANDLING
#include <flowctrl.h>
#endif
#include <tegra_def.h>
#include <tegra_private.h>

/* Legacy FIQ used by earlier Tegra platforms */
#define LEGACY_FIQ_PPI_WDT		28U

/*******************************************************************************
 * Static variables
 ******************************************************************************/
static uint64_t ns_fiq_handler_addr;
static uint32_t fiq_handler_active;
static pcpu_fiq_state_t fiq_state[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * Handler for FIQ interrupts
 ******************************************************************************/
static int tegra_fiq_interrupt_handler(unsigned int id, unsigned int flags,
		void *handle, void *cookie)
{
	cpu_context_t *ctx = cm_get_context(NON_SECURE);
	el3_state_t *el3state_ctx = get_el3state_ctx(ctx);
	uint32_t cpu = plat_my_core_pos();

	(void)flags;
	(void)handle;
	(void)cookie;

	/*
	 * Jump to NS world only if the NS world's FIQ handler has
	 * been registered
	 */
	if (ns_fiq_handler_addr != 0U) {

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
		cm_set_elr_el3(NON_SECURE, ns_fiq_handler_addr);
	}

#if ENABLE_WDT_LEGACY_FIQ_HANDLING
	/*
	 * Tegra platforms that use LEGACY_FIQ as the watchdog timer FIQ
	 * need to issue an IPI to other CPUs, to allow them to handle
	 * the "system hung" scenario. This interrupt is passed to the GICD
	 * via the Flow Controller. So, once we receive this interrupt,
	 * disable the routing so that we can mark it as "complete" in the
	 * GIC later.
	 */
	if (id == LEGACY_FIQ_PPI_WDT) {
		tegra_fc_disable_fiq_to_ccplex_routing();
	}
#endif

	/*
	 * Mark this interrupt as complete to avoid a FIQ storm.
	 */
	plat_ic_end_of_interrupt(id);

	return 0;
}

/*******************************************************************************
 * Setup handler for FIQ interrupts
 ******************************************************************************/
void tegra_fiq_handler_setup(void)
{
	/* return if already registered */
	if (fiq_handler_active == 0U) {
		/*
		 * Register an interrupt handler for FIQ interrupts generated for
		 * NS interrupt sources
		 */
		ehf_register_priority_handler(PLAT_TEGRA_WDT_PRIO, tegra_fiq_interrupt_handler);

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
	const el1_sysregs_t *el1state_ctx = get_el1_sysregs_ctx(ctx);
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
