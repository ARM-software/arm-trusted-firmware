/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <bl31/bl31.h>
#include <bl31/ehf.h>
#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <context.h>
#include <lib/cassert.h>
#include <lib/el3_runtime/pubsub.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <services/sdei.h>

#include "sdei_private.h"

#define MAJOR_VERSION	1ULL
#define MINOR_VERSION	0ULL
#define VENDOR_VERSION	0ULL

#define MAKE_SDEI_VERSION(_major, _minor, _vendor) \
	((((_major)) << 48ULL) | (((_minor)) << 32ULL) | (_vendor))

#define LOWEST_INTR_PRIORITY		0xff

#define is_valid_affinity(_mpidr)	(plat_core_pos_by_mpidr(_mpidr) >= 0)

CASSERT(PLAT_SDEI_CRITICAL_PRI < PLAT_SDEI_NORMAL_PRI,
		sdei_critical_must_have_higher_priority);

static unsigned int num_dyn_priv_slots, num_dyn_shrd_slots;

/* Initialise SDEI map entries */
static void init_map(sdei_ev_map_t *map)
{
	map->reg_count = 0;
}

/* Convert mapping to SDEI class */
static sdei_class_t map_to_class(sdei_ev_map_t *map)
{
	return is_event_critical(map) ? SDEI_CRITICAL : SDEI_NORMAL;
}

/* Clear SDEI event entries except state */
static void clear_event_entries(sdei_entry_t *se)
{
	se->ep = 0;
	se->arg = 0;
	se->affinity = 0;
	se->reg_flags = 0;
}

/* Perform CPU-specific state initialisation */
static void *sdei_cpu_on_init(const void *arg)
{
	unsigned int i;
	sdei_ev_map_t *map;
	sdei_entry_t *se;

	/* Initialize private mappings on this CPU */
	for_each_private_map(i, map) {
		se = get_event_entry(map);
		clear_event_entries(se);
		se->state = 0;
	}

	SDEI_LOG("Private events initialized on %lx\n", read_mpidr_el1());

	/* All PEs start with SDEI events masked */
	(void) sdei_pe_mask();

	return NULL;
}

/* CPU initialisation after wakeup from suspend */
static void *sdei_cpu_wakeup_init(const void *arg)
{
	SDEI_LOG("Events masked on %lx\n", read_mpidr_el1());

	/* All PEs wake up with SDEI events masked */
	sdei_pe_mask();

	return 0;
}

/* Initialise an SDEI class */
static void sdei_class_init(sdei_class_t class)
{
	unsigned int i;
	bool zero_found __unused = false;
	int ev_num_so_far __unused;
	sdei_ev_map_t *map;

	/* Sanity check and configuration of shared events */
	ev_num_so_far = -1;
	for_each_shared_map(i, map) {
#if ENABLE_ASSERTIONS
		/* Ensure mappings are sorted */
		assert((ev_num_so_far < 0) || (map->ev_num > ev_num_so_far));

		ev_num_so_far = map->ev_num;

		/* Event 0 must not be shared */
		assert(map->ev_num != SDEI_EVENT_0);

		/* Check for valid event */
		assert(map->ev_num >= 0);

		/* Make sure it's a shared event */
		assert(is_event_shared(map));

		/* No shared mapping should have signalable property */
		assert(!is_event_signalable(map));

		/* Shared mappings can't be explicit */
		assert(!is_map_explicit(map));
#endif

		/* Skip initializing the wrong priority */
		if (map_to_class(map) != class)
			continue;

		/* Platform events are always bound, so set the bound flag */
		if (is_map_dynamic(map)) {
			assert(map->intr == SDEI_DYN_IRQ);
			assert(is_event_normal(map));
			num_dyn_shrd_slots++;
		} else {
			/* Shared mappings must be bound to shared interrupt */
			assert(plat_ic_is_spi(map->intr) != 0);
			set_map_bound(map);
		}

		init_map(map);
	}

	/* Sanity check and configuration of private events for this CPU */
	ev_num_so_far = -1;
	for_each_private_map(i, map) {
#if ENABLE_ASSERTIONS
		/* Ensure mappings are sorted */
		assert((ev_num_so_far < 0) || (map->ev_num > ev_num_so_far));

		ev_num_so_far = map->ev_num;

		if (map->ev_num == SDEI_EVENT_0) {
			zero_found = true;

			/* Event 0 must be a Secure SGI */
			assert(is_secure_sgi(map->intr));

			/*
			 * Event 0 can have only have signalable flag (apart
			 * from being private
			 */
			assert(map->map_flags == (SDEI_MAPF_SIGNALABLE |
						SDEI_MAPF_PRIVATE));
		} else {
			/* No other mapping should have signalable property */
			assert(!is_event_signalable(map));
		}

		/* Check for valid event */
		assert(map->ev_num >= 0);

		/* Make sure it's a private event */
		assert(is_event_private(map));

		/*
		 * Other than priority, explicit events can only have explicit
		 * and private flags set.
		 */
		if (is_map_explicit(map)) {
			assert((map->map_flags | SDEI_MAPF_CRITICAL) ==
					(SDEI_MAPF_EXPLICIT | SDEI_MAPF_PRIVATE
					| SDEI_MAPF_CRITICAL));
		}
#endif

		/* Skip initializing the wrong priority */
		if (map_to_class(map) != class)
			continue;

		/* Platform events are always bound, so set the bound flag */
		if (map->ev_num != SDEI_EVENT_0) {
			if (is_map_dynamic(map)) {
				assert(map->intr == SDEI_DYN_IRQ);
				assert(is_event_normal(map));
				num_dyn_priv_slots++;
			} else if (is_map_explicit(map)) {
				/*
				 * Explicit mappings don't have a backing
				 * SDEI interrupt, but verify that anyway.
				 */
				assert(map->intr == SDEI_DYN_IRQ);
			} else {
				/*
				 * Private mappings must be bound to private
				 * interrupt.
				 */
				assert(plat_ic_is_ppi((unsigned) map->intr) != 0);
				set_map_bound(map);
			}
		}

		init_map(map);
	}

	/* Ensure event 0 is in the mapping */
	assert(zero_found);

	(void) sdei_cpu_on_init(NULL);
}

/* SDEI dispatcher initialisation */
void sdei_init(void)
{
	plat_sdei_setup();
	sdei_class_init(SDEI_CRITICAL);
	sdei_class_init(SDEI_NORMAL);

	/* Register priority level handlers */
	ehf_register_priority_handler(PLAT_SDEI_CRITICAL_PRI,
			sdei_intr_handler);
	ehf_register_priority_handler(PLAT_SDEI_NORMAL_PRI,
			sdei_intr_handler);
}

/* Populate SDEI event entry */
static void set_sdei_entry(sdei_entry_t *se, uint64_t ep, uint64_t arg,
		unsigned int flags, uint64_t affinity)
{
	assert(se != NULL);

	se->ep = ep;
	se->arg = arg;
	se->affinity = (affinity & MPIDR_AFFINITY_MASK);
	se->reg_flags = flags;
}

static uint64_t sdei_version(void)
{
	return MAKE_SDEI_VERSION(MAJOR_VERSION, MINOR_VERSION, VENDOR_VERSION);
}

/* Validate flags and MPIDR values for REGISTER and ROUTING_SET calls */
static int validate_flags(uint64_t flags, uint64_t mpidr)
{
	/* Validate flags */
	switch (flags) {
	case SDEI_REGF_RM_PE:
		if (!is_valid_affinity(mpidr))
			return SDEI_EINVAL;
		break;
	case SDEI_REGF_RM_ANY:
		break;
	default:
		/* Unknown flags */
		return SDEI_EINVAL;
	}

	return 0;
}

/* Set routing of an SDEI event */
static int sdei_event_routing_set(int ev_num, uint64_t flags, uint64_t mpidr)
{
	int ret;
	unsigned int routing;
	sdei_ev_map_t *map;
	sdei_entry_t *se;

	ret = validate_flags(flags, mpidr);
	if (ret != 0)
		return ret;

	/* Check if valid event number */
	map = find_event_map(ev_num);
	if (map == NULL)
		return SDEI_EINVAL;

	/* The event must not be private */
	if (is_event_private(map))
		return SDEI_EINVAL;

	se = get_event_entry(map);

	sdei_map_lock(map);

	if (!is_map_bound(map) || is_event_private(map)) {
		ret = SDEI_EINVAL;
		goto finish;
	}

	if (!can_sdei_state_trans(se, DO_ROUTING)) {
		ret = SDEI_EDENY;
		goto finish;
	}

	/* Choose appropriate routing */
	routing = (unsigned int) ((flags == SDEI_REGF_RM_ANY) ?
		INTR_ROUTING_MODE_ANY : INTR_ROUTING_MODE_PE);

	/* Update event registration flag */
	se->reg_flags = (unsigned int) flags;
	if (flags == SDEI_REGF_RM_PE) {
		se->affinity = (mpidr & MPIDR_AFFINITY_MASK);
	}

	/*
	 * ROUTING_SET is permissible only when event composite state is
	 * 'registered, disabled, and not running'. This means that the
	 * interrupt is currently disabled, and not active.
	 */
	plat_ic_set_spi_routing(map->intr, routing, (u_register_t) mpidr);

finish:
	sdei_map_unlock(map);

	return ret;
}

/* Register handler and argument for an SDEI event */
static int64_t sdei_event_register(int ev_num,
				uint64_t ep,
				uint64_t arg,
				uint64_t flags,
				uint64_t mpidr)
{
	int ret;
	unsigned int routing;
	sdei_entry_t *se;
	sdei_ev_map_t *map;
	sdei_state_t backup_state;

	if ((ep == 0U) || (plat_sdei_validate_entry_point(
					ep, sdei_client_el()) != 0)) {
		return SDEI_EINVAL;
	}

	ret = validate_flags(flags, mpidr);
	if (ret != 0)
		return ret;

	/* Check if valid event number */
	map = find_event_map(ev_num);
	if (map == NULL)
		return SDEI_EINVAL;

	/* Private events always target the PE */
	if (is_event_private(map))
		flags = SDEI_REGF_RM_PE;

	se = get_event_entry(map);

	/*
	 * Even though register operation is per-event (additionally for private
	 * events, registration is required individually), it has to be
	 * serialised with respect to bind/release, which are global operations.
	 * So we hold the lock throughout, unconditionally.
	 */
	sdei_map_lock(map);

	backup_state = se->state;
	if (!can_sdei_state_trans(se, DO_REGISTER))
		goto fallback;

	/*
	 * When registering for dynamic events, make sure it's been bound
	 * already. This has to be the case as, without binding, the client
	 * can't know about the event number to register for.
	 */
	if (is_map_dynamic(map) && !is_map_bound(map))
		goto fallback;

	if (is_event_private(map)) {
		/* Multiple calls to register are possible for private events */
		assert(map->reg_count >= 0);
	} else {
		/* Only single call to register is possible for shared events */
		assert(map->reg_count == 0);
	}

	if (is_map_bound(map)) {
		/* Meanwhile, did any PE ACK the interrupt? */
		if (plat_ic_get_interrupt_active(map->intr) != 0U)
			goto fallback;

		/* The interrupt must currently owned by Non-secure */
		if (plat_ic_get_interrupt_type(map->intr) != INTR_TYPE_NS)
			goto fallback;

		/*
		 * Disable forwarding of new interrupt triggers to CPU
		 * interface.
		 */
		plat_ic_disable_interrupt(map->intr);

		/*
		 * Any events that are triggered after register and before
		 * enable should remain pending. Clear any previous interrupt
		 * triggers which are pending (except for SGIs). This has no
		 * affect on level-triggered interrupts.
		 */
		if (ev_num != SDEI_EVENT_0)
			plat_ic_clear_interrupt_pending(map->intr);

		/* Map interrupt to EL3 and program the correct priority */
		plat_ic_set_interrupt_type(map->intr, INTR_TYPE_EL3);

		/* Program the appropriate interrupt priority */
		plat_ic_set_interrupt_priority(map->intr, sdei_event_priority(map));

		/*
		 * Set the routing mode for shared event as requested. We
		 * already ensure that shared events get bound to SPIs.
		 */
		if (is_event_shared(map)) {
			routing = (unsigned int) ((flags == SDEI_REGF_RM_ANY) ?
				INTR_ROUTING_MODE_ANY : INTR_ROUTING_MODE_PE);
			plat_ic_set_spi_routing(map->intr, routing,
					(u_register_t) mpidr);
		}
	}

	/* Populate event entries */
	set_sdei_entry(se, ep, arg, (unsigned int) flags, mpidr);

	/* Increment register count */
	map->reg_count++;

	sdei_map_unlock(map);

	return 0;

fallback:
	/* Reinstate previous state */
	se->state = backup_state;

	sdei_map_unlock(map);

	return SDEI_EDENY;
}

/* Enable SDEI event */
static int64_t sdei_event_enable(int ev_num)
{
	sdei_ev_map_t *map;
	sdei_entry_t *se;
	int ret;
	bool before, after;

	/* Check if valid event number */
	map = find_event_map(ev_num);
	if (map == NULL)
		return SDEI_EINVAL;

	se = get_event_entry(map);
	ret = SDEI_EDENY;

	if (is_event_shared(map))
		sdei_map_lock(map);

	before = GET_EV_STATE(se, ENABLED);
	if (!can_sdei_state_trans(se, DO_ENABLE))
		goto finish;
	after = GET_EV_STATE(se, ENABLED);

	/*
	 * Enable interrupt for bound events only if there's a change in enabled
	 * state.
	 */
	if (is_map_bound(map) && (!before && after))
		plat_ic_enable_interrupt(map->intr);

	ret = 0;

finish:
	if (is_event_shared(map))
		sdei_map_unlock(map);

	return ret;
}

/* Disable SDEI event */
static int sdei_event_disable(int ev_num)
{
	sdei_ev_map_t *map;
	sdei_entry_t *se;
	int ret;
	bool before, after;

	/* Check if valid event number */
	map = find_event_map(ev_num);
	if (map == NULL)
		return SDEI_EINVAL;

	se = get_event_entry(map);
	ret = SDEI_EDENY;

	if (is_event_shared(map))
		sdei_map_lock(map);

	before = GET_EV_STATE(se, ENABLED);
	if (!can_sdei_state_trans(se, DO_DISABLE))
		goto finish;
	after = GET_EV_STATE(se, ENABLED);

	/*
	 * Disable interrupt for bound events only if there's a change in
	 * enabled state.
	 */
	if (is_map_bound(map) && (before && !after))
		plat_ic_disable_interrupt(map->intr);

	ret = 0;

finish:
	if (is_event_shared(map))
		sdei_map_unlock(map);

	return ret;
}

/* Query SDEI event information */
static int64_t sdei_event_get_info(int ev_num, int info)
{
	sdei_entry_t *se;
	sdei_ev_map_t *map;

	uint64_t flags;
	bool registered;
	uint64_t affinity;

	/* Check if valid event number */
	map = find_event_map(ev_num);
	if (map == NULL)
		return SDEI_EINVAL;

	se = get_event_entry(map);

	if (is_event_shared(map))
		sdei_map_lock(map);

	/* Sample state under lock */
	registered = GET_EV_STATE(se, REGISTERED);
	flags = se->reg_flags;
	affinity = se->affinity;

	if (is_event_shared(map))
		sdei_map_unlock(map);

	switch (info) {
	case SDEI_INFO_EV_TYPE:
		return is_event_shared(map);

	case SDEI_INFO_EV_NOT_SIGNALED:
		return !is_event_signalable(map);

	case SDEI_INFO_EV_PRIORITY:
		return is_event_critical(map);

	case SDEI_INFO_EV_ROUTING_MODE:
		if (!is_event_shared(map))
			return SDEI_EINVAL;
		if (!registered)
			return SDEI_EDENY;
		return (flags == SDEI_REGF_RM_PE);

	case SDEI_INFO_EV_ROUTING_AFF:
		if (!is_event_shared(map))
			return SDEI_EINVAL;
		if (!registered)
			return SDEI_EDENY;
		if (flags != SDEI_REGF_RM_PE)
			return SDEI_EINVAL;
		return affinity;

	default:
		return SDEI_EINVAL;
	}
}

/* Unregister an SDEI event */
static int sdei_event_unregister(int ev_num)
{
	int ret = 0;
	sdei_entry_t *se;
	sdei_ev_map_t *map;

	/* Check if valid event number */
	map = find_event_map(ev_num);
	if (map == NULL)
		return SDEI_EINVAL;

	se = get_event_entry(map);

	/*
	 * Even though unregister operation is per-event (additionally for
	 * private events, unregistration is required individually), it has to
	 * be serialised with respect to bind/release, which are global
	 * operations.  So we hold the lock throughout, unconditionally.
	 */
	sdei_map_lock(map);

	if (!can_sdei_state_trans(se, DO_UNREGISTER)) {
		/*
		 * Even if the call is invalid, and the handler is running (for
		 * example, having unregistered from a running handler earlier),
		 * return pending error code; otherwise, return deny.
		 */
		ret = GET_EV_STATE(se, RUNNING) ? SDEI_EPEND : SDEI_EDENY;

		goto finish;
	}

	map->reg_count--;
	if (is_event_private(map)) {
		/* Multiple calls to register are possible for private events */
		assert(map->reg_count >= 0);
	} else {
		/* Only single call to register is possible for shared events */
		assert(map->reg_count == 0);
	}

	if (is_map_bound(map)) {
		plat_ic_disable_interrupt(map->intr);

		/*
		 * Clear pending interrupt. Skip for SGIs as they may not be
		 * cleared on interrupt controllers.
		 */
		if (ev_num != SDEI_EVENT_0)
			plat_ic_clear_interrupt_pending(map->intr);

		assert(plat_ic_get_interrupt_type(map->intr) == INTR_TYPE_EL3);
		plat_ic_set_interrupt_type(map->intr, INTR_TYPE_NS);
		plat_ic_set_interrupt_priority(map->intr, LOWEST_INTR_PRIORITY);
	}

	clear_event_entries(se);

	/*
	 * If the handler is running at the time of unregister, return the
	 * pending error code.
	 */
	if (GET_EV_STATE(se, RUNNING))
		ret = SDEI_EPEND;

finish:
	sdei_map_unlock(map);

	return ret;
}

/* Query status of an SDEI event */
static int sdei_event_status(int ev_num)
{
	sdei_ev_map_t *map;
	sdei_entry_t *se;
	sdei_state_t state;

	/* Check if valid event number */
	map = find_event_map(ev_num);
	if (map == NULL)
		return SDEI_EINVAL;

	se = get_event_entry(map);

	if (is_event_shared(map))
		sdei_map_lock(map);

	/* State value directly maps to the expected return format */
	state = se->state;

	if (is_event_shared(map))
		sdei_map_unlock(map);

	return (int) state;
}

/* Bind an SDEI event to an interrupt */
static int sdei_interrupt_bind(unsigned int intr_num)
{
	sdei_ev_map_t *map;
	bool retry = true, shared_mapping;

	/* SGIs are not allowed to be bound */
	if (plat_ic_is_sgi(intr_num) != 0)
		return SDEI_EINVAL;

	shared_mapping = (plat_ic_is_spi(intr_num) != 0);
	do {
		/*
		 * Bail out if there is already an event for this interrupt,
		 * either platform-defined or dynamic.
		 */
		map = find_event_map_by_intr(intr_num, shared_mapping);
		if (map != NULL) {
			if (is_map_dynamic(map)) {
				if (is_map_bound(map)) {
					/*
					 * Dynamic event, already bound. Return
					 * event number.
					 */
					return map->ev_num;
				}
			} else {
				/* Binding non-dynamic event */
				return SDEI_EINVAL;
			}
		}

		/*
		 * The interrupt is not bound yet. Try to find a free slot to
		 * bind it. Free dynamic mappings have their interrupt set as
		 * SDEI_DYN_IRQ.
		 */
		map = find_event_map_by_intr(SDEI_DYN_IRQ, shared_mapping);
		if (map == NULL)
			return SDEI_ENOMEM;

		/* The returned mapping must be dynamic */
		assert(is_map_dynamic(map));

		/*
		 * We cannot assert for bound maps here, as we might be racing
		 * with another bind.
		 */

		/* The requested interrupt must already belong to NS */
		if (plat_ic_get_interrupt_type(intr_num) != INTR_TYPE_NS)
			return SDEI_EDENY;

		/*
		 * Interrupt programming and ownership transfer are deferred
		 * until register.
		 */

		sdei_map_lock(map);
		if (!is_map_bound(map)) {
			map->intr = intr_num;
			set_map_bound(map);
			retry = false;
		}
		sdei_map_unlock(map);
	} while (retry);

	return map->ev_num;
}

/* Release a bound SDEI event previously to an interrupt */
static int sdei_interrupt_release(int ev_num)
{
	int ret = 0;
	sdei_ev_map_t *map;
	sdei_entry_t *se;

	/* Check if valid event number */
	map = find_event_map(ev_num);
	if (map == NULL)
		return SDEI_EINVAL;

	if (!is_map_dynamic(map))
		return SDEI_EINVAL;

	se = get_event_entry(map);

	sdei_map_lock(map);

	/* Event must have been unregistered before release */
	if (map->reg_count != 0) {
		ret = SDEI_EDENY;
		goto finish;
	}

	/*
	 * Interrupt release never causes the state to change. We only check
	 * whether it's permissible or not.
	 */
	if (!can_sdei_state_trans(se, DO_RELEASE)) {
		ret = SDEI_EDENY;
		goto finish;
	}

	if (is_map_bound(map)) {
		/*
		 * Deny release if the interrupt is active, which means it's
		 * probably being acknowledged and handled elsewhere.
		 */
		if (plat_ic_get_interrupt_active(map->intr) != 0U) {
			ret = SDEI_EDENY;
			goto finish;
		}

		/*
		 * Interrupt programming and ownership transfer are already done
		 * during unregister.
		 */

		map->intr = SDEI_DYN_IRQ;
		clr_map_bound(map);
	} else {
		SDEI_LOG("Error release bound:%d cnt:%d\n", is_map_bound(map),
				map->reg_count);
		ret = SDEI_EINVAL;
	}

finish:
	sdei_map_unlock(map);

	return ret;
}

/* Perform reset of private SDEI events */
static int sdei_private_reset(void)
{
	sdei_ev_map_t *map;
	int ret = 0, final_ret = 0;
	unsigned int i;

	/* Unregister all private events */
	for_each_private_map(i, map) {
		/*
		 * The unregister can fail if the event is not registered, which
		 * is allowed, and a deny will be returned. But if the event is
		 * running or unregister pending, the call fails.
		 */
		ret = sdei_event_unregister(map->ev_num);
		if ((ret == SDEI_EPEND) && (final_ret == 0))
			final_ret = SDEI_EDENY;
	}

	return final_ret;
}

/* Perform reset of shared SDEI events */
static int sdei_shared_reset(void)
{
	const sdei_mapping_t *mapping;
	sdei_ev_map_t *map;
	int ret = 0, final_ret = 0;
	unsigned int i, j;

	/* Unregister all shared events */
	for_each_shared_map(i, map) {
		/*
		 * The unregister can fail if the event is not registered, which
		 * is allowed, and a deny will be returned. But if the event is
		 * running or unregister pending, the call fails.
		 */
		ret = sdei_event_unregister(map->ev_num);
		if ((ret == SDEI_EPEND) && (final_ret == 0))
			final_ret = SDEI_EDENY;
	}

	if (final_ret != 0)
		return final_ret;

	/*
	 * Loop through both private and shared mappings, and release all
	 * bindings.
	 */
	for_each_mapping_type(i, mapping) {
		iterate_mapping(mapping, j, map) {
			/*
			 * Release bindings for mappings that are dynamic and
			 * bound.
			 */
			if (is_map_dynamic(map) && is_map_bound(map)) {
				/*
				 * Any failure to release would mean there is at
				 * least a PE registered for the event.
				 */
				ret = sdei_interrupt_release(map->ev_num);
				if ((ret != 0) && (final_ret == 0))
					final_ret = ret;
			}
		}
	}

	return final_ret;
}

/* Send a signal to another SDEI client PE */
static int sdei_signal(int ev_num, uint64_t target_pe)
{
	sdei_ev_map_t *map;

	/* Only event 0 can be signalled */
	if (ev_num != SDEI_EVENT_0)
		return SDEI_EINVAL;

	/* Find mapping for event 0 */
	map = find_event_map(SDEI_EVENT_0);
	if (map == NULL)
		return SDEI_EINVAL;

	/* The event must be signalable */
	if (!is_event_signalable(map))
		return SDEI_EINVAL;

	/* Validate target */
	if (plat_core_pos_by_mpidr(target_pe) < 0)
		return SDEI_EINVAL;

	/* Raise SGI. Platform will validate target_pe */
	plat_ic_raise_el3_sgi((int) map->intr, (u_register_t) target_pe);

	return 0;
}

/* Query SDEI dispatcher features */
static uint64_t sdei_features(unsigned int feature)
{
	if (feature == SDEI_FEATURE_BIND_SLOTS) {
		return FEATURE_BIND_SLOTS(num_dyn_priv_slots,
				num_dyn_shrd_slots);
	}

	return (uint64_t) SDEI_EINVAL;
}

/* SDEI top level handler for servicing SMCs */
uint64_t sdei_smc_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{

	uint64_t x5;
	unsigned int ss = (unsigned int) get_interrupt_src_ss(flags);
	int64_t ret;
	bool resume = false;
	cpu_context_t *ctx = handle;
	int ev_num = (int) x1;

	if (ss != NON_SECURE)
		SMC_RET1(ctx, SMC_UNK);

	/* Verify the caller EL */
	if (GET_EL(read_spsr_el3()) != sdei_client_el())
		SMC_RET1(ctx, SMC_UNK);

	switch (smc_fid) {
	case SDEI_VERSION:
		SDEI_LOG("> VER\n");
		ret = (int64_t) sdei_version();
		SDEI_LOG("< VER:%llx\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_EVENT_REGISTER:
		x5 = SMC_GET_GP(ctx, CTX_GPREG_X5);
		SDEI_LOG("> REG(n:%d e:%llx a:%llx f:%x m:%llx)\n", ev_num,
				x2, x3, (int) x4, x5);
		ret = sdei_event_register(ev_num, x2, x3, x4, x5);
		SDEI_LOG("< REG:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_EVENT_ENABLE:
		SDEI_LOG("> ENABLE(n:%d)\n", (int) x1);
		ret = sdei_event_enable(ev_num);
		SDEI_LOG("< ENABLE:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_EVENT_DISABLE:
		SDEI_LOG("> DISABLE(n:%d)\n", ev_num);
		ret = sdei_event_disable(ev_num);
		SDEI_LOG("< DISABLE:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_EVENT_CONTEXT:
		SDEI_LOG("> CTX(p:%d):%lx\n", (int) x1, read_mpidr_el1());
		ret = sdei_event_context(ctx, (unsigned int) x1);
		SDEI_LOG("< CTX:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_EVENT_COMPLETE_AND_RESUME:
		resume = true;
		/* Fallthrough */

	case SDEI_EVENT_COMPLETE:
		SDEI_LOG("> COMPLETE(r:%u sta/ep:%llx):%lx\n",
				(unsigned int) resume, x1, read_mpidr_el1());
		ret = sdei_event_complete(resume, x1);
		SDEI_LOG("< COMPLETE:%llx\n", ret);

		/*
		 * Set error code only if the call failed. If the call
		 * succeeded, we discard the dispatched context, and restore the
		 * interrupted context to a pristine condition, and therefore
		 * shouldn't be modified. We don't return to the caller in this
		 * case anyway.
		 */
		if (ret != 0)
			SMC_RET1(ctx, ret);

		SMC_RET0(ctx);

	case SDEI_EVENT_STATUS:
		SDEI_LOG("> STAT(n:%d)\n", ev_num);
		ret = sdei_event_status(ev_num);
		SDEI_LOG("< STAT:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_EVENT_GET_INFO:
		SDEI_LOG("> INFO(n:%d, %d)\n", ev_num, (int) x2);
		ret = sdei_event_get_info(ev_num, (int) x2);
		SDEI_LOG("< INFO:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_EVENT_UNREGISTER:
		SDEI_LOG("> UNREG(n:%d)\n", ev_num);
		ret = sdei_event_unregister(ev_num);
		SDEI_LOG("< UNREG:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_PE_UNMASK:
		SDEI_LOG("> UNMASK:%lx\n", read_mpidr_el1());
		sdei_pe_unmask();
		SDEI_LOG("< UNMASK:%d\n", 0);
		SMC_RET1(ctx, 0);

	case SDEI_PE_MASK:
		SDEI_LOG("> MASK:%lx\n", read_mpidr_el1());
		ret = sdei_pe_mask();
		SDEI_LOG("< MASK:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_INTERRUPT_BIND:
		SDEI_LOG("> BIND(%d)\n", (int) x1);
		ret = sdei_interrupt_bind((unsigned int) x1);
		SDEI_LOG("< BIND:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_INTERRUPT_RELEASE:
		SDEI_LOG("> REL(%d)\n", ev_num);
		ret = sdei_interrupt_release(ev_num);
		SDEI_LOG("< REL:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_SHARED_RESET:
		SDEI_LOG("> S_RESET():%lx\n", read_mpidr_el1());
		ret = sdei_shared_reset();
		SDEI_LOG("< S_RESET:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_PRIVATE_RESET:
		SDEI_LOG("> P_RESET():%lx\n", read_mpidr_el1());
		ret = sdei_private_reset();
		SDEI_LOG("< P_RESET:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_EVENT_ROUTING_SET:
		SDEI_LOG("> ROUTE_SET(n:%d f:%llx aff:%llx)\n", ev_num, x2, x3);
		ret = sdei_event_routing_set(ev_num, x2, x3);
		SDEI_LOG("< ROUTE_SET:%lld\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_FEATURES:
		SDEI_LOG("> FTRS(f:%llx)\n", x1);
		ret = (int64_t) sdei_features((unsigned int) x1);
		SDEI_LOG("< FTRS:%llx\n", ret);
		SMC_RET1(ctx, ret);

	case SDEI_EVENT_SIGNAL:
		SDEI_LOG("> SIGNAL(e:%d t:%llx)\n", ev_num, x2);
		ret = sdei_signal(ev_num, x2);
		SDEI_LOG("< SIGNAL:%lld\n", ret);
		SMC_RET1(ctx, ret);

	default:
		/* Do nothing in default case */
		break;
	}

	WARN("Unimplemented SDEI Call: 0x%x\n", smc_fid);
	SMC_RET1(ctx, SMC_UNK);
}

/* Subscribe to PSCI CPU on to initialize per-CPU SDEI configuration */
SUBSCRIBE_TO_EVENT(psci_cpu_on_finish, sdei_cpu_on_init);

/* Subscribe to PSCI CPU suspend finisher for per-CPU configuration */
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, sdei_cpu_wakeup_init);
