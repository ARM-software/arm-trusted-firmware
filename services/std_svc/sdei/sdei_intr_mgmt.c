/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <cassert.h>
#include <context_mgmt.h>
#include <debug.h>
#include <ehf.h>
#include <interrupt_mgmt.h>
#include <runtime_svc.h>
#include <sdei.h>
#include <string.h>
#include "sdei_private.h"

#define PE_MASKED	1
#define PE_NOT_MASKED	0

/* x0-x17 GPREGS context */
#define SDEI_SAVED_GPREGS	18

/* Maximum preemption nesting levels: Critical priority and Normal priority */
#define MAX_EVENT_NESTING	2

/* Per-CPU SDEI state access macro */
#define sdei_get_this_pe_state()	(&sdei_cpu_state[plat_my_core_pos()])

/* Structure to store information about an outstanding dispatch */
typedef struct sdei_dispatch_context {
	sdei_ev_map_t *map;
	unsigned int sec_state;
	unsigned int intr_raw;
	uint64_t x[SDEI_SAVED_GPREGS];

	/* Exception state registers */
	uint64_t elr_el3;
	uint64_t spsr_el3;
} sdei_dispatch_context_t;

/* Per-CPU SDEI state data */
typedef struct sdei_cpu_state {
	sdei_dispatch_context_t dispatch_stack[MAX_EVENT_NESTING];
	unsigned short stack_top; /* Empty ascending */
	unsigned int pe_masked:1;
	unsigned int pending_enables:1;
} sdei_cpu_state_t;

/* SDEI states for all cores in the system */
static sdei_cpu_state_t sdei_cpu_state[PLATFORM_CORE_COUNT];

unsigned int sdei_pe_mask(void)
{
	unsigned int ret;
	sdei_cpu_state_t *state = sdei_get_this_pe_state();

	/*
	 * Return value indicates whether this call had any effect in the mask
	 * status of this PE.
	 */
	ret = (state->pe_masked ^ PE_MASKED);
	state->pe_masked = PE_MASKED;

	return ret;
}

void sdei_pe_unmask(void)
{
	int i;
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
			if (is_map_bound(map) &&
					GET_EV_STATE(se, ENABLED) &&
					(se->reg_flags == SDEI_REGF_RM_PE) &&
					(se->affinity == my_mpidr)) {
				plat_ic_enable_interrupt(map->intr);
			}
			sdei_map_unlock(map);
		}
	}

	state->pending_enables = 0;
	state->pe_masked = PE_NOT_MASKED;
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

	if (state->stack_top == 0)
		return NULL;

	assert(state->stack_top <= MAX_EVENT_NESTING);

	state->stack_top--;

	return &state->dispatch_stack[state->stack_top];
}

/* Retrieve the context at the top of dispatch stack */
static sdei_dispatch_context_t *get_outstanding_dispatch(void)
{
	sdei_cpu_state_t *state = sdei_get_this_pe_state();

	if (state->stack_top == 0)
		return NULL;

	assert(state->stack_top <= MAX_EVENT_NESTING);

	return &state->dispatch_stack[state->stack_top - 1];
}

static void save_event_ctx(sdei_ev_map_t *map, void *tgt_ctx, int sec_state,
		unsigned int intr_raw)
{
	sdei_dispatch_context_t *disp_ctx;
	gp_regs_t *tgt_gpregs;
	el3_state_t *tgt_el3;

	assert(tgt_ctx);
	tgt_gpregs = get_gpregs_ctx(tgt_ctx);
	tgt_el3 = get_el3state_ctx(tgt_ctx);

	disp_ctx = push_dispatch();
	assert(disp_ctx);
	disp_ctx->sec_state = sec_state;
	disp_ctx->map = map;
	disp_ctx->intr_raw = intr_raw;

	/* Save general purpose and exception registers */
	memcpy(disp_ctx->x, tgt_gpregs, sizeof(disp_ctx->x));
	disp_ctx->spsr_el3 = read_ctx_reg(tgt_el3, CTX_SPSR_EL3);
	disp_ctx->elr_el3 = read_ctx_reg(tgt_el3, CTX_ELR_EL3);
}

static void restore_event_ctx(sdei_dispatch_context_t *disp_ctx, void *tgt_ctx)
{
	gp_regs_t *tgt_gpregs;
	el3_state_t *tgt_el3;

	assert(tgt_ctx);
	tgt_gpregs = get_gpregs_ctx(tgt_ctx);
	tgt_el3 = get_el3state_ctx(tgt_ctx);

	CASSERT(sizeof(disp_ctx->x) == (SDEI_SAVED_GPREGS * sizeof(uint64_t)),
			foo);

	/* Restore general purpose and exception registers */
	memcpy(tgt_gpregs, disp_ctx->x, sizeof(disp_ctx->x));
	write_ctx_reg(tgt_el3, CTX_SPSR_EL3, disp_ctx->spsr_el3);
	write_ctx_reg(tgt_el3, CTX_ELR_EL3, disp_ctx->elr_el3);
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
	assert(ns_ctx);

	return ns_ctx;
}

/*
 * Populate the Non-secure context so that the next ERET will dispatch to the
 * SDEI client.
 */
static void setup_ns_dispatch(sdei_ev_map_t *map, sdei_entry_t *se,
		cpu_context_t *ctx, int sec_state_to_resume,
		unsigned int intr_raw)
{
	el3_state_t *el3_ctx = get_el3state_ctx(ctx);

	/* Push the event and context */
	save_event_ctx(map, ctx, sec_state_to_resume, intr_raw);

	/*
	 * Setup handler arguments:
	 *
	 * - x0: Event number
	 * - x1: Handler argument supplied at the time of event registration
	 * - x2: Interrupted PC
	 * - x3: Interrupted SPSR
	 */
	SMC_SET_GP(ctx, CTX_GPREG_X0, map->ev_num);
	SMC_SET_GP(ctx, CTX_GPREG_X1, se->arg);
	SMC_SET_GP(ctx, CTX_GPREG_X2, read_ctx_reg(el3_ctx, CTX_ELR_EL3));
	SMC_SET_GP(ctx, CTX_GPREG_X3, read_ctx_reg(el3_ctx, CTX_SPSR_EL3));

	/*
	 * Prepare for ERET:
	 *
	 * - Set PC to the registered handler address
	 * - Set SPSR to jump to client EL with exceptions masked
	 */
	cm_set_elr_spsr_el3(NON_SECURE, (uintptr_t) se->ep,
			SPSR_64(sdei_client_el(), MODE_SP_ELX,
				DISABLE_ALL_EXCEPTIONS));
}

/* Handle a triggered SDEI interrupt while events were masked on this PE */
static void handle_masked_trigger(sdei_ev_map_t *map, sdei_entry_t *se,
		sdei_cpu_state_t *state, unsigned int intr_raw)
{
	uint64_t my_mpidr __unused = (read_mpidr_el1() & MPIDR_AFFINITY_MASK);
	int disable = 0;

	/* Nothing to do for event 0 */
	if (map->ev_num == SDEI_EVENT_0)
		return;

	/*
	 * For a private event, or for a shared event specifically routed to
	 * this CPU, we disable interrupt, leave the interrupt pending, and do
	 * EOI.
	 */
	if (is_event_private(map)) {
		disable = 1;
	} else if (se->reg_flags == SDEI_REGF_RM_PE) {
		assert(se->affinity == my_mpidr);
		disable = 1;
	}

	if (disable) {
		plat_ic_disable_interrupt(map->intr);
		plat_ic_set_interrupt_pending(map->intr);
		plat_ic_end_of_interrupt(intr_raw);
		state->pending_enables = 1;

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
	assert(plat_ic_is_spi(map->intr));
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
	sdei_dispatch_context_t *disp_ctx;
	unsigned int sec_state;
	sdei_cpu_state_t *state;
	uint32_t intr;

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
	map = find_event_map_by_intr(intr, plat_ic_is_spi(intr));
	if (!map) {
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

	if (state->pe_masked == PE_MASKED) {
		/*
		 * Interrupts received while this PE was masked can't be
		 * dispatched.
		 */
		SDEI_LOG("interrupt %u on %lx while PE masked\n", map->intr,
				read_mpidr_el1());
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
		assert(se->affinity ==
				(read_mpidr_el1() & MPIDR_AFFINITY_MASK));
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
		if (disp_ctx)
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

	SDEI_LOG("ACK %lx, ev:%d ss:%d spsr:%lx ELR:%lx\n", read_mpidr_el1(),
			map->ev_num, sec_state, read_spsr_el3(),
			read_elr_el3());

	ctx = handle;

	/*
	 * Check if we interrupted secure state. Perform a context switch so
	 * that we can delegate to NS.
	 */
	if (sec_state == SECURE) {
		save_secure_context();
		ctx = restore_and_resume_ns_context();
	}

	setup_ns_dispatch(map, se, ctx, sec_state, intr_raw);

	/*
	 * End of interrupt is done in sdei_event_complete, when the client
	 * signals completion.
	 */
	return 0;
}

/* Explicitly dispatch the given SDEI event */
int sdei_dispatch_event(int ev_num, unsigned int preempted_sec_state)
{
	sdei_entry_t *se;
	sdei_ev_map_t *map;
	cpu_context_t *ctx;
	sdei_dispatch_context_t *disp_ctx;
	sdei_cpu_state_t *state;

	/* Validate preempted security state */
	if ((preempted_sec_state != SECURE) &&
			(preempted_sec_state != NON_SECURE)) {
		return -1;
	}

	/* Can't dispatch if events are masked on this PE */
	state = sdei_get_this_pe_state();
	if (state->pe_masked == PE_MASKED)
		return -1;

	/* Event 0 can't be dispatched */
	if (ev_num == SDEI_EVENT_0)
		return -1;

	/* Locate mapping corresponding to this event */
	map = find_event_map(ev_num);
	if (!map)
		return -1;

	/*
	 * Statically-bound or dynamic maps are dispatched only as a result of
	 * interrupt, and not upon explicit request.
	 */
	if (is_map_dynamic(map) || is_map_bound(map))
		return -1;

	/* The event must be private */
	if (is_event_shared(map))
		return -1;

	/* Examine state of dispatch stack */
	disp_ctx = get_outstanding_dispatch();
	if (disp_ctx) {
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

	/* Activate the priority corresponding to the event being dispatched */
	ehf_activate_priority(sdei_event_priority(map));

	/*
	 * We assume the current context is SECURE, and that it's already been
	 * saved.
	 */
	ctx = restore_and_resume_ns_context();

	/*
	 * The caller has effectively terminated execution. Record to resume the
	 * preempted context later when the event completes or
	 * complete-and-resumes.
	 */
	setup_ns_dispatch(map, se, ctx, preempted_sec_state, 0);

	return 0;
}

int sdei_event_complete(int resume, uint64_t pc)
{
	sdei_dispatch_context_t *disp_ctx;
	sdei_entry_t *se;
	sdei_ev_map_t *map;
	cpu_context_t *ctx;
	sdei_action_t act;
	unsigned int client_el = sdei_client_el();

	/* Return error if called without an active event */
	disp_ctx = pop_dispatch();
	if (!disp_ctx)
		return SDEI_EDENY;

	/* Validate resumption point */
	if (resume && (plat_sdei_validate_entry_point(pc, client_el) != 0))
		return SDEI_EDENY;

	map = disp_ctx->map;
	assert(map);

	se = get_event_entry(map);

	SDEI_LOG("EOI:%lx, %d spsr:%lx elr:%lx\n", read_mpidr_el1(),
			map->ev_num, read_spsr_el3(), read_elr_el3());

	if (is_event_shared(map))
		sdei_map_lock(map);

	act = resume ? DO_COMPLETE_RESUME : DO_COMPLETE;
	if (!can_sdei_state_trans(se, act)) {
		if (is_event_shared(map))
			sdei_map_unlock(map);
		return SDEI_EDENY;
	}

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

	/*
	 * If the cause of dispatch originally interrupted the Secure world, and
	 * if Non-secure world wasn't allowed to preempt Secure execution,
	 * resume Secure.
	 *
	 * No need to save the Non-secure context ahead of a world switch: the
	 * Non-secure context was fully saved before dispatch, and has been
	 * returned to its pre-dispatch state.
	 */
	if ((disp_ctx->sec_state == SECURE) &&
			(ehf_is_ns_preemption_allowed() == 0)) {
		restore_and_resume_secure_context();
	}

	if ((map->ev_num == SDEI_EVENT_0) || is_map_bound(map)) {
		/*
		 * The event was dispatched after receiving SDEI interrupt. With
		 * the event handling completed, EOI the corresponding
		 * interrupt.
		 */
		plat_ic_end_of_interrupt(disp_ctx->intr_raw);
	} else {
		/*
		 * An unbound event must have been dispatched explicitly.
		 * Deactivate the priority level that was activated at the time
		 * of explicit dispatch.
		 */
		ehf_deactivate_priority(sdei_event_priority(map));
	}

	if (is_event_shared(map))
		sdei_map_unlock(map);

	return 0;
}

int sdei_event_context(void *handle, unsigned int param)
{
	sdei_dispatch_context_t *disp_ctx;

	if (param >= SDEI_SAVED_GPREGS)
		return SDEI_EINVAL;

	/* Get outstanding dispatch on this CPU */
	disp_ctx = get_outstanding_dispatch();
	if (!disp_ctx)
		return SDEI_EDENY;

	assert(disp_ctx->map);

	if (!can_sdei_state_trans(get_event_entry(disp_ctx->map), DO_CONTEXT))
		return SDEI_EDENY;

	/*
	 * No locking is required for the Running status as this is the only CPU
	 * which can complete the event
	 */

	return disp_ctx->x[param];
}
