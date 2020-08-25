/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <arch_helpers.h>
#include <arch_features.h>
#include <bl31/ehf.h>
#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/cassert.h>
#include <services/sdei.h>

#include "sdei_private.h"

/* x0-x17 GPREGS context */
#define SDEI_SAVED_GPREGS	18U

/* Maximum preemption nesting levels: Critical priority and Normal priority */
#define MAX_EVENT_NESTING	2U

/* Per-CPU SDEI state access macro */
#define sdei_get_this_pe_state()	(&cpu_state[plat_my_core_pos()])

/* Structure to store information about an outstanding dispatch */
typedef struct sdei_dispatch_context {
	sdei_ev_map_t *map;
	uint64_t x[SDEI_SAVED_GPREGS];
	jmp_buf *dispatch_jmp;

	/* Exception state registers */
	uint64_t elr_el3;
	uint64_t spsr_el3;

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	/* CVE-2018-3639 mitigation state */
	uint64_t disable_cve_2018_3639;
#endif
} sdei_dispatch_context_t;

/* Per-CPU SDEI state data */
typedef struct sdei_cpu_state {
	sdei_dispatch_context_t dispatch_stack[MAX_EVENT_NESTING];
	unsigned short stack_top; /* Empty ascending */
	bool pe_masked;
	bool pending_enables;
} sdei_cpu_state_t;

/* SDEI states for all cores in the system */
static sdei_cpu_state_t cpu_state[PLATFORM_CORE_COUNT];

int64_t sdei_pe_mask(void)
{
	int64_t ret = 0;
	sdei_cpu_state_t *state = sdei_get_this_pe_state();

	/*
	 * Return value indicates whether this call had any effect in the mask
	 * status of this PE.
	 */
	if (!state->pe_masked) {
		state->pe_masked = true;
		ret = 1;
	}

	return ret;
}

void sdei_pe_unmask(void)
{
	unsigned int i;
	sdei_ev_map_t *map;
	sdei_entry_t *se;
	sdei_cpu_state_t *state = sdei_get_this_pe_state();
	uint64_t my_mpidr = read_mpidr_el1() & MPIDR_AFFINITY_MASK;

	/*
	 * If there are pending enables, iterate through the private mappings
	 * and enable those bound maps that are in enabled state. Also, iterate
	 * through shared mappings and enable interrupts of events that are
	 * targeted to this PE.
	 */
	if (state->pending_enables) {
		for_each_private_map(i, map) {
			se = get_event_entry(map);
			if (is_map_bound(map) && GET_EV_STATE(se, ENABLED))
				plat_ic_enable_interrupt(map->intr);
		}

		for_each_shared_map(i, map) {
			se = get_event_entry(map);

			sdei_map_lock(map);
			if (is_map_bound(map) && GET_EV_STATE(se, ENABLED) &&
					(se->reg_flags == SDEI_REGF_RM_PE) &&
					(se->affinity == my_mpidr)) {
				plat_ic_enable_interrupt(map->intr);
			}
			sdei_map_unlock(map);
		}
	}

	state->pending_enables = false;
	state->pe_masked = false;
}

/* Push a dispatch context to the dispatch stack */
static sdei_dispatch_context_t *push_dispatch(void)
{
	sdei_cpu_state_t *state = sdei_get_this_pe_state();
	sdei_dispatch_context_t *disp_ctx;

	/* Cannot have more than max events */
	assert(state->stack_top < MAX_EVENT_NESTING);

	disp_ctx = &state->dispatch_stack[state->stack_top];
	state->stack_top++;

	return disp_ctx;
}

/* Pop a dispatch context to the dispatch stack */
static sdei_dispatch_context_t *pop_dispatch(void)
{
	sdei_cpu_state_t *state = sdei_get_this_pe_state();

	if (state->stack_top == 0U)
		return NULL;

	assert(state->stack_top <= MAX_EVENT_NESTING);

	state->stack_top--;

	return &state->dispatch_stack[state->stack_top];
}

/* Retrieve the context at the top of dispatch stack */
static sdei_dispatch_context_t *get_outstanding_dispatch(void)
{
	sdei_cpu_state_t *state = sdei_get_this_pe_state();

	if (state->stack_top == 0U)
		return NULL;

	assert(state->stack_top <= MAX_EVENT_NESTING);

	return &state->dispatch_stack[state->stack_top - 1U];
}

static sdei_dispatch_context_t *save_event_ctx(sdei_ev_map_t *map,
		void *tgt_ctx)
{
	sdei_dispatch_context_t *disp_ctx;
	const gp_regs_t *tgt_gpregs;
	const el3_state_t *tgt_el3;

	assert(tgt_ctx != NULL);
	tgt_gpregs = get_gpregs_ctx(tgt_ctx);
	tgt_el3 = get_el3state_ctx(tgt_ctx);

	disp_ctx = push_dispatch();
	assert(disp_ctx != NULL);
	disp_ctx->map = map;

	/* Save general purpose and exception registers */
	memcpy(disp_ctx->x, tgt_gpregs, sizeof(disp_ctx->x));
	disp_ctx->spsr_el3 = read_ctx_reg(tgt_el3, CTX_SPSR_EL3);
	disp_ctx->elr_el3 = read_ctx_reg(tgt_el3, CTX_ELR_EL3);

	return disp_ctx;
}

static void restore_event_ctx(const sdei_dispatch_context_t *disp_ctx, void *tgt_ctx)
{
	gp_regs_t *tgt_gpregs;
	el3_state_t *tgt_el3;

	assert(tgt_ctx != NULL);
	tgt_gpregs = get_gpregs_ctx(tgt_ctx);
	tgt_el3 = get_el3state_ctx(tgt_ctx);

	CASSERT(sizeof(disp_ctx->x) == (SDEI_SAVED_GPREGS * sizeof(uint64_t)),
			foo);

	/* Restore general purpose and exception registers */
	memcpy(tgt_gpregs, disp_ctx->x, sizeof(disp_ctx->x));
	write_ctx_reg(tgt_el3, CTX_SPSR_EL3, disp_ctx->spsr_el3);
	write_ctx_reg(tgt_el3, CTX_ELR_EL3, disp_ctx->elr_el3);

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	cve_2018_3639_t *tgt_cve_2018_3639;
	tgt_cve_2018_3639 = get_cve_2018_3639_ctx(tgt_ctx);

	/* Restore CVE-2018-3639 mitigation state */
	write_ctx_reg(tgt_cve_2018_3639, CTX_CVE_2018_3639_DISABLE,
		disp_ctx->disable_cve_2018_3639);
#endif
}

static void save_secure_context(void)
{
	cm_el1_sysregs_context_save(SECURE);
}

/* Restore Secure context and arrange to resume it at the next ERET */
static void restore_and_resume_secure_context(void)
{
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);
}

/*
 * Restore Non-secure context and arrange to resume it at the next ERET. Return
 * pointer to the Non-secure context.
 */
static cpu_context_t *restore_and_resume_ns_context(void)
{
	cpu_context_t *ns_ctx;

	cm_el1_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);

	ns_ctx = cm_get_context(NON_SECURE);
	assert(ns_ctx != NULL);

	return ns_ctx;
}

/*
 * Prepare for ERET:
 * - Set the ELR to the registered handler address
 * - Set the SPSR register as described in the SDEI documentation and
 *   the AArch64.TakeException() pseudocode function in
 *   ARM DDI 0487F.c page J1-7635
 */

static void sdei_set_elr_spsr(sdei_entry_t *se, sdei_dispatch_context_t *disp_ctx)
{
	unsigned int client_el = sdei_client_el();
	u_register_t sdei_spsr = SPSR_64(client_el, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS);

	u_register_t interrupted_pstate = disp_ctx->spsr_el3;

	/* Check the SPAN bit in the client el SCTLR */
	u_register_t client_el_sctlr;

	if (client_el == MODE_EL2) {
		client_el_sctlr = read_sctlr_el2();
	} else {
		client_el_sctlr = read_sctlr_el1();
	}

	/*
	 * Check whether to force the PAN bit or use the value in the
	 * interrupted EL according to the check described in
	 * TakeException. Since the client can only be Non-Secure
	 * EL2 or El1 some of the conditions in ElIsInHost() we know
	 * will always be True.
	 * When the client_el is EL2 we know that there will be a SPAN
	 * bit in SCTLR_EL2 as we have already checked for the condition
	 * HCR_EL2.E2H = 1 and HCR_EL2.TGE = 1
	 */
	u_register_t hcr_el2 = read_hcr();
	bool el_is_in_host = is_armv8_1_vhe_present() &&
			     (hcr_el2 & HCR_TGE_BIT) &&
			     (hcr_el2 & HCR_E2H_BIT);

	if (is_armv8_1_pan_present() &&
	    ((client_el == MODE_EL1) ||
		(client_el == MODE_EL2 && el_is_in_host)) &&
	    ((client_el_sctlr & SCTLR_SPAN_BIT) == 0U)) {
		sdei_spsr |=  SPSR_PAN_BIT;
	} else {
		sdei_spsr |= (interrupted_pstate & SPSR_PAN_BIT);
	}

	/* If SSBS is implemented, take the value from the client el SCTLR */
	u_register_t ssbs_enabled = (read_id_aa64pfr1_el1()
					>> ID_AA64PFR1_EL1_SSBS_SHIFT)
					& ID_AA64PFR1_EL1_SSBS_MASK;
	if (ssbs_enabled != SSBS_UNAVAILABLE) {
		u_register_t  ssbs_bit = ((client_el_sctlr & SCTLR_DSSBS_BIT)
						>> SCTLR_DSSBS_SHIFT)
						<< SPSR_SSBS_SHIFT_AARCH64;
		sdei_spsr |= ssbs_bit;
	}

	/* If MTE is implemented in the client el set the TCO bit */
	if (get_armv8_5_mte_support() >= MTE_IMPLEMENTED_ELX) {
		sdei_spsr |= SPSR_TCO_BIT_AARCH64;
	}

	/* Take the DIT field from the pstate of the interrupted el */
	sdei_spsr |= (interrupted_pstate & SPSR_DIT_BIT);

	cm_set_elr_spsr_el3(NON_SECURE, (uintptr_t) se->ep, sdei_spsr);
}

/*
 * Populate the Non-secure context so that the next ERET will dispatch to the
 * SDEI client.
 */
static void setup_ns_dispatch(sdei_ev_map_t *map, sdei_entry_t *se,
		cpu_context_t *ctx, jmp_buf *dispatch_jmp)
{
	sdei_dispatch_context_t *disp_ctx;

	/* Push the event and context */
	disp_ctx = save_event_ctx(map, ctx);

	/*
	 * Setup handler arguments:
	 *
	 * - x0: Event number
	 * - x1: Handler argument supplied at the time of event registration
	 * - x2: Interrupted PC
	 * - x3: Interrupted SPSR
	 */
	SMC_SET_GP(ctx, CTX_GPREG_X0, (uint64_t) map->ev_num);
	SMC_SET_GP(ctx, CTX_GPREG_X1, se->arg);
	SMC_SET_GP(ctx, CTX_GPREG_X2, disp_ctx->elr_el3);
	SMC_SET_GP(ctx, CTX_GPREG_X3, disp_ctx->spsr_el3);

	/* Setup the elr and spsr register to prepare for ERET */
	sdei_set_elr_spsr(se, disp_ctx);

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	cve_2018_3639_t *tgt_cve_2018_3639;
	tgt_cve_2018_3639 = get_cve_2018_3639_ctx(ctx);

	/* Save CVE-2018-3639 mitigation state */
	disp_ctx->disable_cve_2018_3639 = read_ctx_reg(tgt_cve_2018_3639,
		CTX_CVE_2018_3639_DISABLE);

	/* Force SDEI handler to execute with mitigation enabled by default */
	write_ctx_reg(tgt_cve_2018_3639, CTX_CVE_2018_3639_DISABLE, 0);
#endif

	disp_ctx->dispatch_jmp = dispatch_jmp;
}

/* Handle a triggered SDEI interrupt while events were masked on this PE */
static void handle_masked_trigger(sdei_ev_map_t *map, sdei_entry_t *se,
		sdei_cpu_state_t *state, unsigned int intr_raw)
{
	uint64_t my_mpidr __unused = (read_mpidr_el1() & MPIDR_AFFINITY_MASK);
	bool disable = false;

	/* Nothing to do for event 0 */
	if (map->ev_num == SDEI_EVENT_0)
		return;

	/*
	 * For a private event, or for a shared event specifically routed to
	 * this CPU, we disable interrupt, leave the interrupt pending, and do
	 * EOI.
	 */
	if (is_event_private(map) || (se->reg_flags == SDEI_REGF_RM_PE))
		disable = true;

	if (se->reg_flags == SDEI_REGF_RM_PE)
		assert(se->affinity == my_mpidr);

	if (disable) {
		plat_ic_disable_interrupt(map->intr);
		plat_ic_set_interrupt_pending(map->intr);
		plat_ic_end_of_interrupt(intr_raw);
		state->pending_enables = true;

		return;
	}

	/*
	 * We just received a shared event with routing set to ANY PE. The
	 * interrupt can't be delegated on this PE as SDEI events are masked.
	 * However, because its routing mode is ANY, it is possible that the
	 * event can be delegated on any other PE that hasn't masked events.
	 * Therefore, we set the interrupt back pending so as to give other
	 * suitable PEs a chance of handling it.
	 */
	assert(plat_ic_is_spi(map->intr) != 0);
	plat_ic_set_interrupt_pending(map->intr);

	/*
	 * Leaving the same interrupt pending also means that the same interrupt
	 * can target this PE again as soon as this PE leaves EL3. Whether and
	 * how often that happens depends on the implementation of GIC.
	 *
	 * We therefore call a platform handler to resolve this situation.
	 */
	plat_sdei_handle_masked_trigger(my_mpidr, map->intr);

	/* This PE is masked. We EOI the interrupt, as it can't be delegated */
	plat_ic_end_of_interrupt(intr_raw);
}

/* SDEI main interrupt handler */
int sdei_intr_handler(uint32_t intr_raw, uint32_t flags, void *handle,
		void *cookie)
{
	sdei_entry_t *se;
	cpu_context_t *ctx;
	sdei_ev_map_t *map;
	const sdei_dispatch_context_t *disp_ctx;
	unsigned int sec_state;
	sdei_cpu_state_t *state;
	uint32_t intr;
	jmp_buf dispatch_jmp;
	const uint64_t mpidr = read_mpidr_el1();

	/*
	 * To handle an event, the following conditions must be true:
	 *
	 * 1. Event must be signalled
	 * 2. Event must be enabled
	 * 3. This PE must be a target PE for the event
	 * 4. PE must be unmasked for SDEI
	 * 5. If this is a normal event, no event must be running
	 * 6. If this is a critical event, no critical event must be running
	 *
	 * (1) and (2) are true when this function is running
	 * (3) is enforced in GIC by selecting the appropriate routing option
	 * (4) is satisfied by client calling PE_UNMASK
	 * (5) and (6) is enforced using interrupt priority, the RPR, in GIC:
	 *   - Normal SDEI events belong to Normal SDE priority class
	 *   - Critical SDEI events belong to Critical CSDE priority class
	 *
	 * The interrupt has already been acknowledged, and therefore is active,
	 * so no other PE can handle this event while we are at it.
	 *
	 * Find if this is an SDEI interrupt. There must be an event mapped to
	 * this interrupt
	 */
	intr = plat_ic_get_interrupt_id(intr_raw);
	map = find_event_map_by_intr(intr, (plat_ic_is_spi(intr) != 0));
	if (map == NULL) {
		ERROR("No SDEI map for interrupt %u\n", intr);
		panic();
	}

	/*
	 * Received interrupt number must either correspond to event 0, or must
	 * be bound interrupt.
	 */
	assert((map->ev_num == SDEI_EVENT_0) || is_map_bound(map));

	se = get_event_entry(map);
	state = sdei_get_this_pe_state();

	if (state->pe_masked) {
		/*
		 * Interrupts received while this PE was masked can't be
		 * dispatched.
		 */
		SDEI_LOG("interrupt %u on %" PRIx64 " while PE masked\n",
			 map->intr, mpidr);
		if (is_event_shared(map))
			sdei_map_lock(map);

		handle_masked_trigger(map, se, state, intr_raw);

		if (is_event_shared(map))
			sdei_map_unlock(map);

		return 0;
	}

	/* Insert load barrier for signalled SDEI event */
	if (map->ev_num == SDEI_EVENT_0)
		dmbld();

	if (is_event_shared(map))
		sdei_map_lock(map);

	/* Assert shared event routed to this PE had been configured so */
	if (is_event_shared(map) && (se->reg_flags == SDEI_REGF_RM_PE)) {
		assert(se->affinity == (mpidr & MPIDR_AFFINITY_MASK));
	}

	if (!can_sdei_state_trans(se, DO_DISPATCH)) {
		SDEI_LOG("SDEI event 0x%x can't be dispatched; state=0x%x\n",
				map->ev_num, se->state);

		/*
		 * If the event is registered, leave the interrupt pending so
		 * that it's delivered when the event is enabled.
		 */
		if (GET_EV_STATE(se, REGISTERED))
			plat_ic_set_interrupt_pending(map->intr);

		/*
		 * The interrupt was disabled or unregistered after the handler
		 * started to execute, which means now the interrupt is already
		 * disabled and we just need to EOI the interrupt.
		 */
		plat_ic_end_of_interrupt(intr_raw);

		if (is_event_shared(map))
			sdei_map_unlock(map);

		return 0;
	}

	disp_ctx = get_outstanding_dispatch();
	if (is_event_critical(map)) {
		/*
		 * If this event is Critical, and if there's an outstanding
		 * dispatch, assert the latter is a Normal dispatch. Critical
		 * events can preempt an outstanding Normal event dispatch.
		 */
		if (disp_ctx != NULL)
			assert(is_event_normal(disp_ctx->map));
	} else {
		/*
		 * If this event is Normal, assert that there are no outstanding
		 * dispatches. Normal events can't preempt any outstanding event
		 * dispatches.
		 */
		assert(disp_ctx == NULL);
	}

	sec_state = get_interrupt_src_ss(flags);

	if (is_event_shared(map))
		sdei_map_unlock(map);

	SDEI_LOG("ACK %" PRIx64 ", ev:0x%x ss:%d spsr:%lx ELR:%lx\n",
		 mpidr, map->ev_num, sec_state, read_spsr_el3(), read_elr_el3());

	ctx = handle;

	/*
	 * Check if we interrupted secure state. Perform a context switch so
	 * that we can delegate to NS.
	 */
	if (sec_state == SECURE) {
		save_secure_context();
		ctx = restore_and_resume_ns_context();
	}

	/* Synchronously dispatch event */
	setup_ns_dispatch(map, se, ctx, &dispatch_jmp);
	begin_sdei_synchronous_dispatch(&dispatch_jmp);

	/*
	 * We reach here when client completes the event.
	 *
	 * If the cause of dispatch originally interrupted the Secure world,
	 * resume Secure.
	 *
	 * No need to save the Non-secure context ahead of a world switch: the
	 * Non-secure context was fully saved before dispatch, and has been
	 * returned to its pre-dispatch state.
	 */
	if (sec_state == SECURE)
		restore_and_resume_secure_context();

	/*
	 * The event was dispatched after receiving SDEI interrupt. With
	 * the event handling completed, EOI the corresponding
	 * interrupt.
	 */
	if ((map->ev_num != SDEI_EVENT_0) && !is_map_bound(map)) {
		ERROR("Invalid SDEI mapping: ev=0x%x\n", map->ev_num);
		panic();
	}
	plat_ic_end_of_interrupt(intr_raw);

	return 0;
}

/*
 * Explicitly dispatch the given SDEI event.
 *
 * When calling this API, the caller must be prepared for the SDEI dispatcher to
 * restore and make Non-secure context as active. This call returns only after
 * the client has completed the dispatch. Then, the Non-secure context will be
 * active, and the following ERET will return to Non-secure.
 *
 * Should the caller require re-entry to Secure, it must restore the Secure
 * context and program registers for ERET.
 */
int sdei_dispatch_event(int ev_num)
{
	sdei_entry_t *se;
	sdei_ev_map_t *map;
	cpu_context_t *ns_ctx;
	sdei_dispatch_context_t *disp_ctx;
	sdei_cpu_state_t *state;
	jmp_buf dispatch_jmp;

	/* Can't dispatch if events are masked on this PE */
	state = sdei_get_this_pe_state();
	if (state->pe_masked)
		return -1;

	/* Event 0 can't be dispatched */
	if (ev_num == SDEI_EVENT_0)
		return -1;

	/* Locate mapping corresponding to this event */
	map = find_event_map(ev_num);
	if (map == NULL)
		return -1;

	/* Only explicit events can be dispatched */
	if (!is_map_explicit(map))
		return -1;

	/* Examine state of dispatch stack */
	disp_ctx = get_outstanding_dispatch();
	if (disp_ctx != NULL) {
		/*
		 * There's an outstanding dispatch. If the outstanding dispatch
		 * is critical, no more dispatches are possible.
		 */
		if (is_event_critical(disp_ctx->map))
			return -1;

		/*
		 * If the outstanding dispatch is Normal, only critical events
		 * can be dispatched.
		 */
		if (is_event_normal(map))
			return -1;
	}

	se = get_event_entry(map);
	if (!can_sdei_state_trans(se, DO_DISPATCH))
		return -1;

	/*
	 * Prepare for NS dispatch by restoring the Non-secure context and
	 * marking that as active.
	 */
	ns_ctx = restore_and_resume_ns_context();

	/* Activate the priority corresponding to the event being dispatched */
	ehf_activate_priority(sdei_event_priority(map));

	/* Dispatch event synchronously */
	setup_ns_dispatch(map, se, ns_ctx, &dispatch_jmp);
	begin_sdei_synchronous_dispatch(&dispatch_jmp);

	/*
	 * We reach here when client completes the event.
	 *
	 * Deactivate the priority level that was activated at the time of
	 * explicit dispatch.
	 */
	ehf_deactivate_priority(sdei_event_priority(map));

	return 0;
}

static void end_sdei_synchronous_dispatch(jmp_buf *buffer)
{
	longjmp(*buffer, 1);
}

int sdei_event_complete(bool resume, uint64_t pc)
{
	sdei_dispatch_context_t *disp_ctx;
	sdei_entry_t *se;
	sdei_ev_map_t *map;
	cpu_context_t *ctx;
	sdei_action_t act;
	unsigned int client_el = sdei_client_el();

	/* Return error if called without an active event */
	disp_ctx = get_outstanding_dispatch();
	if (disp_ctx == NULL)
		return SDEI_EDENY;

	/* Validate resumption point */
	if (resume && (plat_sdei_validate_entry_point(pc, client_el) != 0))
		return SDEI_EDENY;

	map = disp_ctx->map;
	assert(map != NULL);
	se = get_event_entry(map);

	if (is_event_shared(map))
		sdei_map_lock(map);

	act = resume ? DO_COMPLETE_RESUME : DO_COMPLETE;
	if (!can_sdei_state_trans(se, act)) {
		if (is_event_shared(map))
			sdei_map_unlock(map);
		return SDEI_EDENY;
	}

	if (is_event_shared(map))
		sdei_map_unlock(map);

	/* Having done sanity checks, pop dispatch */
	(void) pop_dispatch();

	SDEI_LOG("EOI:%lx, %d spsr:%lx elr:%lx\n", read_mpidr_el1(),
			map->ev_num, read_spsr_el3(), read_elr_el3());

	/*
	 * Restore Non-secure to how it was originally interrupted. Once done,
	 * it's up-to-date with the saved copy.
	 */
	ctx = cm_get_context(NON_SECURE);
	restore_event_ctx(disp_ctx, ctx);

	if (resume) {
		/*
		 * Complete-and-resume call. Prepare the Non-secure context
		 * (currently active) for complete and resume.
		 */
		cm_set_elr_spsr_el3(NON_SECURE, pc, SPSR_64(client_el,
					MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS));

		/*
		 * Make it look as if a synchronous exception were taken at the
		 * supplied Non-secure resumption point. Populate SPSR and
		 * ELR_ELx so that an ERET from there works as expected.
		 *
		 * The assumption is that the client, if necessary, would have
		 * saved any live content in these registers before making this
		 * call.
		 */
		if (client_el == MODE_EL2) {
			write_elr_el2(disp_ctx->elr_el3);
			write_spsr_el2(disp_ctx->spsr_el3);
		} else {
			/* EL1 */
			write_elr_el1(disp_ctx->elr_el3);
			write_spsr_el1(disp_ctx->spsr_el3);
		}
	}

	/* End the outstanding dispatch */
	end_sdei_synchronous_dispatch(disp_ctx->dispatch_jmp);

	return 0;
}

int64_t sdei_event_context(void *handle, unsigned int param)
{
	sdei_dispatch_context_t *disp_ctx;

	if (param >= SDEI_SAVED_GPREGS)
		return SDEI_EINVAL;

	/* Get outstanding dispatch on this CPU */
	disp_ctx = get_outstanding_dispatch();
	if (disp_ctx == NULL)
		return SDEI_EDENY;

	assert(disp_ctx->map != NULL);

	if (!can_sdei_state_trans(get_event_entry(disp_ctx->map), DO_CONTEXT))
		return SDEI_EDENY;

	/*
	 * No locking is required for the Running status as this is the only CPU
	 * which can complete the event
	 */

	return (int64_t) disp_ctx->x[param];
}
