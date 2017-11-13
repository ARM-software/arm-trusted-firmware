/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Exception handlers at EL3, their priority levels, and management.
 */

#include <assert.h>
#include <cpu_data.h>
#include <debug.h>
#include <ehf.h>
#include <gic_common.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <pubsub_events.h>

/* Output EHF logs as verbose */
#define EHF_LOG(...)	VERBOSE("EHF: " __VA_ARGS__)

#define EHF_INVALID_IDX	(-1)

/* For a valid handler, return the actual function pointer; otherwise, 0. */
#define RAW_HANDLER(h) \
	((ehf_handler_t) ((h & _EHF_PRI_VALID) ? (h & ~_EHF_PRI_VALID) : 0))

#define PRI_BIT(idx)	(((ehf_pri_bits_t) 1) << idx)

/*
 * Convert index into secure priority using the platform-defined priority bits
 * field.
 */
#define IDX_TO_PRI(idx) \
	((idx << (7 - exception_data.pri_bits)) & 0x7f)

/* Check whether a given index is valid */
#define IS_IDX_VALID(idx) \
	((exception_data.ehf_priorities[idx].ehf_handler & _EHF_PRI_VALID) != 0)

/* Returns whether given priority is in secure priority range */
#define IS_PRI_SECURE(pri)	((pri & 0x80) == 0)

/* To be defined by the platform */
extern const ehf_priorities_t exception_data;

/* Translate priority to the index in the priority array */
static int pri_to_idx(unsigned int priority)
{
	int idx;

	idx = EHF_PRI_TO_IDX(priority, exception_data.pri_bits);
	assert((idx >= 0) && (idx < exception_data.num_priorities));
	assert(IS_IDX_VALID(idx));

	return idx;
}

/* Return whether there are outstanding priority activation */
static int has_valid_pri_activations(pe_exc_data_t *pe_data)
{
	return pe_data->active_pri_bits != 0;
}

static pe_exc_data_t *this_cpu_data(void)
{
	return &get_cpu_data(ehf_data);
}

/*
 * Return the current priority index of this CPU. If no priority is active,
 * return EHF_INVALID_IDX.
 */
static int get_pe_highest_active_idx(pe_exc_data_t *pe_data)
{
	if (!has_valid_pri_activations(pe_data))
		return EHF_INVALID_IDX;

	/* Current priority is the right-most bit */
	return __builtin_ctz(pe_data->active_pri_bits);
}

/*
 * Mark priority active by setting the corresponding bit in active_pri_bits and
 * programming the priority mask.
 *
 * This API is to be used as part of delegating to lower ELs other than for
 * interrupts; e.g. while handling synchronous exceptions.
 *
 * This API is expected to be invoked before restoring context (Secure or
 * Non-secure) in preparation for the respective dispatch.
 */
void ehf_activate_priority(unsigned int priority)
{
	int idx, cur_pri_idx;
	unsigned int old_mask, run_pri;
	pe_exc_data_t *pe_data = this_cpu_data();

	/*
	 * Query interrupt controller for the running priority, or idle priority
	 * if no interrupts are being handled. The requested priority must be
	 * less (higher priority) than the active running priority.
	 */
	run_pri = plat_ic_get_running_priority();
	if (priority >= run_pri) {
		ERROR("Running priority higher (0x%x) than requested (0x%x)\n",
				run_pri, priority);
		panic();
	}

	/*
	 * If there were priority activations already, the requested priority
	 * must be less (higher priority) than the current highest priority
	 * activation so far.
	 */
	cur_pri_idx = get_pe_highest_active_idx(pe_data);
	idx = pri_to_idx(priority);
	if ((cur_pri_idx != EHF_INVALID_IDX) && (idx >= cur_pri_idx)) {
		ERROR("Activation priority mismatch: req=0x%x current=0x%x\n",
				priority, IDX_TO_PRI(cur_pri_idx));
		panic();
	}

	/* Set the bit corresponding to the requested priority */
	pe_data->active_pri_bits |= PRI_BIT(idx);

	/*
	 * Program priority mask for the activated level. Check that the new
	 * priority mask is setting a higher priority level than the existing
	 * mask.
	 */
	old_mask = plat_ic_set_priority_mask(priority);
	if (priority >= old_mask) {
		ERROR("Requested priority (0x%x) lower than Priority Mask (0x%x)\n",
				priority, old_mask);
		panic();
	}

	/*
	 * If this is the first activation, save the priority mask. This will be
	 * restored after the last deactivation.
	 */
	if (cur_pri_idx == EHF_INVALID_IDX)
		pe_data->init_pri_mask = old_mask;

	EHF_LOG("activate prio=%d\n", get_pe_highest_active_idx(pe_data));
}

/*
 * Mark priority inactive by clearing the corresponding bit in active_pri_bits,
 * and programming the priority mask.
 *
 * This API is expected to be used as part of delegating to to lower ELs other
 * than for interrupts; e.g. while handling synchronous exceptions.
 *
 * This API is expected to be invoked after saving context (Secure or
 * Non-secure), having concluded the respective dispatch.
 */
void ehf_deactivate_priority(unsigned int priority)
{
	int idx, cur_pri_idx;
	pe_exc_data_t *pe_data = this_cpu_data();
	unsigned int old_mask, run_pri;

	/*
	 * Query interrupt controller for the running priority, or idle priority
	 * if no interrupts are being handled. The requested priority must be
	 * less (higher priority) than the active running priority.
	 */
	run_pri = plat_ic_get_running_priority();
	if (priority >= run_pri) {
		ERROR("Running priority higher (0x%x) than requested (0x%x)\n",
				run_pri, priority);
		panic();
	}

	/*
	 * Deactivation is allowed only when there are priority activations, and
	 * the deactivation priority level must match the current activated
	 * priority.
	 */
	cur_pri_idx = get_pe_highest_active_idx(pe_data);
	idx = pri_to_idx(priority);
	if ((cur_pri_idx == EHF_INVALID_IDX) || (idx != cur_pri_idx)) {
		ERROR("Deactivation priority mismatch: req=0x%x current=0x%x\n",
				priority, IDX_TO_PRI(cur_pri_idx));
		panic();
	}

	/* Clear bit corresponding to highest priority */
	pe_data->active_pri_bits &= (pe_data->active_pri_bits - 1);

	/*
	 * Restore priority mask corresponding to the next priority, or the
	 * one stashed earlier if there are no more to deactivate.
	 */
	idx = get_pe_highest_active_idx(pe_data);
	if (idx == EHF_INVALID_IDX)
		old_mask = plat_ic_set_priority_mask(pe_data->init_pri_mask);
	else
		old_mask = plat_ic_set_priority_mask(priority);

	if (old_mask >= priority) {
		ERROR("Deactivation priority (0x%x) lower than Priority Mask (0x%x)\n",
				priority, old_mask);
		panic();
	}

	EHF_LOG("deactivate prio=%d\n", get_pe_highest_active_idx(pe_data));
}

/*
 * After leaving Non-secure world, stash current Non-secure Priority Mask, and
 * set Priority Mask to the highest Non-secure priority so that Non-secure
 * interrupts cannot preempt Secure execution.
 *
 * If the current running priority is in the secure range, or if there are
 * outstanding priority activations, this function does nothing.
 *
 * This function subscribes to the 'cm_exited_normal_world' event published by
 * the Context Management Library.
 */
static void *ehf_exited_normal_world(const void *arg)
{
	unsigned int run_pri;
	pe_exc_data_t *pe_data = this_cpu_data();

	/* If the running priority is in the secure range, do nothing */
	run_pri = plat_ic_get_running_priority();
	if (IS_PRI_SECURE(run_pri))
		return 0;

	/* Do nothing if there are explicit activations */
	if (has_valid_pri_activations(pe_data))
		return 0;

	assert(pe_data->ns_pri_mask == 0);

	pe_data->ns_pri_mask =
		plat_ic_set_priority_mask(GIC_HIGHEST_NS_PRIORITY);

	/* The previous Priority Mask is not expected to be in secure range */
	if (IS_PRI_SECURE(pe_data->ns_pri_mask)) {
		ERROR("Priority Mask (0x%x) already in secure range\n",
				pe_data->ns_pri_mask);
		panic();
	}

	EHF_LOG("Priority Mask: 0x%x => 0x%x\n", pe_data->ns_pri_mask,
			GIC_HIGHEST_NS_PRIORITY);

	return 0;
}

/*
 * Conclude Secure execution and prepare for return to Non-secure world. Restore
 * the Non-secure Priority Mask previously stashed upon leaving Non-secure
 * world.
 *
 * If there the current running priority is in the secure range, or if there are
 * outstanding priority activations, this function does nothing.
 *
 * This function subscribes to the 'cm_entering_normal_world' event published by
 * the Context Management Library.
 */
static void *ehf_entering_normal_world(const void *arg)
{
	unsigned int old_pmr, run_pri;
	pe_exc_data_t *pe_data = this_cpu_data();

	/* If the running priority is in the secure range, do nothing */
	run_pri = plat_ic_get_running_priority();
	if (IS_PRI_SECURE(run_pri))
		return 0;

	/*
	 * If there are explicit activations, do nothing. The Priority Mask will
	 * be restored upon the last deactivation.
	 */
	if (has_valid_pri_activations(pe_data))
		return 0;

	/* Do nothing if we don't have a valid Priority Mask to restore */
	if (pe_data->ns_pri_mask == 0)
		return 0;

	old_pmr = plat_ic_set_priority_mask(pe_data->ns_pri_mask);

	/*
	 * When exiting secure world, the current Priority Mask must be
	 * GIC_HIGHEST_NS_PRIORITY (as set during entry), or the Non-secure
	 * priority mask set upon calling ehf_allow_ns_preemption()
	 */
	if ((old_pmr != GIC_HIGHEST_NS_PRIORITY) &&
			(old_pmr != pe_data->ns_pri_mask)) {
		ERROR("Invalid Priority Mask (0x%x) restored\n", old_pmr);
		panic();
	}

	EHF_LOG("Priority Mask: 0x%x => 0x%x\n", old_pmr, pe_data->ns_pri_mask);

	pe_data->ns_pri_mask = 0;

	return 0;
}

/*
 * Program Priority Mask to the original Non-secure priority such that
 * Non-secure interrupts may preempt Secure execution, viz. during Yielding SMC
 * calls.
 *
 * This API is expected to be invoked before delegating a yielding SMC to Secure
 * EL1. I.e. within the window of secure execution after Non-secure context is
 * saved (after entry into EL3) and Secure context is restored (before entering
 * Secure EL1).
 */
void ehf_allow_ns_preemption(void)
{
	unsigned int old_pmr __unused;
	pe_exc_data_t *pe_data = this_cpu_data();

	/*
	 * We should have been notified earlier of entering secure world, and
	 * therefore have stashed the Non-secure priority mask.
	 */
	assert(pe_data->ns_pri_mask != 0);

	/* Make sure no priority levels are active when requesting this */
	if (has_valid_pri_activations(pe_data)) {
		ERROR("PE %lx has priority activations: 0x%x\n",
				read_mpidr_el1(), pe_data->active_pri_bits);
		panic();
	}

	old_pmr = plat_ic_set_priority_mask(pe_data->ns_pri_mask);

	EHF_LOG("Priority Mask: 0x%x => 0x%x\n", old_pmr, pe_data->ns_pri_mask);

	pe_data->ns_pri_mask = 0;
}

/*
 * Return whether Secure execution has explicitly allowed Non-secure interrupts
 * to preempt itself, viz. during Yielding SMC calls.
 */
unsigned int ehf_is_ns_preemption_allowed(void)
{
	unsigned int run_pri;
	pe_exc_data_t *pe_data = this_cpu_data();

	/* If running priority is in secure range, return false */
	run_pri = plat_ic_get_running_priority();
	if (IS_PRI_SECURE(run_pri))
		return 0;

	/*
	 * If Non-secure preemption was permitted by calling
	 * ehf_allow_ns_preemption() earlier:
	 *
	 * - There wouldn't have been priority activations;
	 * - We would have cleared the stashed the Non-secure Priority Mask.
	 */
	if (has_valid_pri_activations(pe_data))
		return 0;
	if (pe_data->ns_pri_mask != 0)
		return 0;

	return 1;
}

/*
 * Top-level EL3 interrupt handler.
 */
static uint64_t ehf_el3_interrupt_handler(uint32_t id, uint32_t flags,
		void *handle, void *cookie)
{
	int pri, idx, intr, intr_raw, ret = 0;
	ehf_handler_t handler;

	/*
	 * Top-level interrupt type handler from Interrupt Management Framework
	 * doesn't acknowledge the interrupt; so the interrupt ID must be
	 * invalid.
	 */
	assert(id == INTR_ID_UNAVAILABLE);

	/*
	 * Acknowledge interrupt. Proceed with handling only for valid interrupt
	 * IDs. This situation may arise because of Interrupt Management
	 * Framework identifying an EL3 interrupt, but before it's been
	 * acknowledged here, the interrupt was either deasserted, or there was
	 * a higher-priority interrupt of another type.
	 */
	intr_raw = plat_ic_acknowledge_interrupt();
	intr = plat_ic_get_interrupt_id(intr_raw);
	if (intr == INTR_ID_UNAVAILABLE)
		return 0;

	/* Having acknowledged the interrupt, get the running priority */
	pri = plat_ic_get_running_priority();

	/* Check EL3 interrupt priority is in secure range */
	assert(IS_PRI_SECURE(pri));

	/*
	 * Translate the priority to a descriptor index. We do this by masking
	 * and shifting the running priority value (platform-supplied).
	 */
	idx = pri_to_idx(pri);

	/* Validate priority */
	assert(pri == IDX_TO_PRI(idx));

	handler = RAW_HANDLER(exception_data.ehf_priorities[idx].ehf_handler);
	if (!handler) {
		ERROR("No EL3 exception handler for priority 0x%x\n",
				IDX_TO_PRI(idx));
		panic();
	}

	/*
	 * Call registered handler. Pass the raw interrupt value to registered
	 * handlers.
	 */
	ret = handler(intr_raw, flags, handle, cookie);

	return ret;
}

/*
 * Initialize the EL3 exception handling.
 */
void ehf_init(void)
{
	unsigned int flags = 0;
	int ret __unused;

	/* Ensure EL3 interrupts are supported */
	assert(plat_ic_has_interrupt_type(INTR_TYPE_EL3));

	/*
	 * Make sure that priority water mark has enough bits to represent the
	 * whole priority array.
	 */
	assert(exception_data.num_priorities <= (sizeof(ehf_pri_bits_t) * 8));

	assert(exception_data.ehf_priorities);

	/*
	 * Bit 7 of GIC priority must be 0 for secure interrupts. This means
	 * platforms must use at least 1 of the remaining 7 bits.
	 */
	assert((exception_data.pri_bits >= 1) || (exception_data.pri_bits < 8));

	/* Route EL3 interrupts when in Secure and Non-secure. */
	set_interrupt_rm_flag(flags, NON_SECURE);
	set_interrupt_rm_flag(flags, SECURE);

	/* Register handler for EL3 interrupts */
	ret = register_interrupt_type_handler(INTR_TYPE_EL3,
			ehf_el3_interrupt_handler, flags);
	assert(ret == 0);
}

/*
 * Register a handler at the supplied priority. Registration is allowed only if
 * a handler hasn't been registered before, or one wasn't provided at build
 * time. The priority for which the handler is being registered must also accord
 * with the platform-supplied data.
 */
void ehf_register_priority_handler(unsigned int pri, ehf_handler_t handler)
{
	int idx;

	/* Sanity check for handler */
	assert(handler != NULL);

	/* Handler ought to be 4-byte aligned */
	assert((((uintptr_t) handler) & 3) == 0);

	/* Ensure we register for valid priority */
	idx = pri_to_idx(pri);
	assert(idx < exception_data.num_priorities);
	assert(IDX_TO_PRI(idx) == pri);

	/* Return failure if a handler was already registered */
	if (exception_data.ehf_priorities[idx].ehf_handler != _EHF_NO_HANDLER) {
		ERROR("Handler already registered for priority 0x%x\n", pri);
		panic();
	}

	/*
	 * Install handler, and retain the valid bit. We assume that the handler
	 * is 4-byte aligned, which is usually the case.
	 */
	exception_data.ehf_priorities[idx].ehf_handler =
		(((uintptr_t) handler) | _EHF_PRI_VALID);

	EHF_LOG("register pri=0x%x handler=%p\n", pri, handler);
}

SUBSCRIBE_TO_EVENT(cm_entering_normal_world, ehf_entering_normal_world);
SUBSCRIBE_TO_EVENT(cm_exited_normal_world, ehf_exited_normal_world);
