/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SDEI_PRIVATE_H__
#define __SDEI_PRIVATE_H__

#include <arch_helpers.h>
#include <debug.h>
#include <errno.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <sdei.h>
#include <spinlock.h>
#include <stdbool.h>
#include <types.h>
#include <utils_def.h>

#ifdef AARCH32
# error SDEI is implemented only for AArch64 systems
#endif

#ifndef PLAT_SDEI_CRITICAL_PRI
# error Platform must define SDEI critical priority value
#endif

#ifndef PLAT_SDEI_NORMAL_PRI
# error Platform must define SDEI normal priority value
#endif

/* Output SDEI logs as verbose */
#define SDEI_LOG(...)	VERBOSE("SDEI: " __VA_ARGS__)

/* SDEI handler unregistered state. This is the default state. */
#define SDEI_STATE_UNREGISTERED		0

/* SDE event status values in bit position */
#define SDEI_STATF_REGISTERED		0
#define SDEI_STATF_ENABLED		1
#define SDEI_STATF_RUNNING		2

/* SDEI SMC error codes */
#define	SDEI_EINVAL	(-2)
#define	SDEI_EDENY	(-3)
#define	SDEI_EPEND	(-5)
#define	SDEI_ENOMEM	(-10)

/*
 * 'info' parameter to SDEI_EVENT_GET_INFO SMC.
 *
 * Note that the SDEI v1.0 speification mistakenly enumerates the
 * SDEI_INFO_EV_SIGNALED as SDEI_INFO_SIGNALED. This will be corrected in a
 * future version.
 */
#define SDEI_INFO_EV_TYPE		0
#define SDEI_INFO_EV_NOT_SIGNALED	1
#define SDEI_INFO_EV_PRIORITY		2
#define SDEI_INFO_EV_ROUTING_MODE	3
#define SDEI_INFO_EV_ROUTING_AFF	4

#define SDEI_PRIVATE_MAPPING()	(&sdei_global_mappings[_SDEI_MAP_IDX_PRIV])
#define SDEI_SHARED_MAPPING()	(&sdei_global_mappings[_SDEI_MAP_IDX_SHRD])

#define for_each_mapping_type(_i, _mapping) \
	for (_i = 0, _mapping = &sdei_global_mappings[i]; \
			_i < _SDEI_MAP_IDX_MAX; \
			_i++, _mapping = &sdei_global_mappings[i])

#define iterate_mapping(_mapping, _i, _map) \
	for (_map = (_mapping)->map, _i = 0; \
			_i < (_mapping)->num_maps; \
			_i++, _map++)

#define for_each_private_map(_i, _map) \
	iterate_mapping(SDEI_PRIVATE_MAPPING(), _i, _map)

#define for_each_shared_map(_i, _map) \
	iterate_mapping(SDEI_SHARED_MAPPING(), _i, _map)

/* SDEI_FEATURES */
#define SDEI_FEATURE_BIND_SLOTS		0
#define BIND_SLOTS_MASK			0xffff
#define FEATURES_SHARED_SLOTS_SHIFT	16
#define FEATURES_PRIVATE_SLOTS_SHIFT	0
#define FEATURE_BIND_SLOTS(_priv, _shrd) \
	((((_priv) & BIND_SLOTS_MASK) << FEATURES_PRIVATE_SLOTS_SHIFT) | \
	 (((_shrd) & BIND_SLOTS_MASK) << FEATURES_SHARED_SLOTS_SHIFT))

#define GET_EV_STATE(_e, _s)	get_ev_state_bit(_e, SDEI_STATF_##_s)
#define SET_EV_STATE(_e, _s)	clr_ev_state_bit(_e->state, SDEI_STATF_##_s)

static inline int is_event_private(sdei_ev_map_t *map)
{
	return ((map->map_flags & BIT(_SDEI_MAPF_PRIVATE_SHIFT)) != 0);
}

static inline int is_event_shared(sdei_ev_map_t *map)
{
	return !is_event_private(map);
}

static inline int is_event_critical(sdei_ev_map_t *map)
{
	return ((map->map_flags & BIT(_SDEI_MAPF_CRITICAL_SHIFT)) != 0);
}

static inline int is_event_normal(sdei_ev_map_t *map)
{
	return !is_event_critical(map);
}

static inline int is_event_signalable(sdei_ev_map_t *map)
{
	return ((map->map_flags & BIT(_SDEI_MAPF_SIGNALABLE_SHIFT)) != 0);
}

static inline int is_map_dynamic(sdei_ev_map_t *map)
{
	return ((map->map_flags & BIT(_SDEI_MAPF_DYNAMIC_SHIFT)) != 0);
}

/*
 * Checks whether an event is associated with an interrupt. Static events always
 * return true, and dynamic events return whether SDEI_INTERRUPT_BIND had been
 * called on them. This can be used on both static or dynamic events to check
 * for an associated interrupt.
 */
static inline int is_map_bound(sdei_ev_map_t *map)
{
	return ((map->map_flags & BIT(_SDEI_MAPF_BOUND_SHIFT)) != 0);
}

static inline void set_map_bound(sdei_ev_map_t *map)
{
	map->map_flags |= BIT(_SDEI_MAPF_BOUND_SHIFT);
}

static inline void clr_map_bound(sdei_ev_map_t *map)
{
	map->map_flags &= ~(BIT(_SDEI_MAPF_BOUND_SHIFT));
}

static inline int is_secure_sgi(unsigned int intr)
{
	return (plat_ic_is_sgi(intr) &&
			(plat_ic_get_interrupt_type(intr) == INTR_TYPE_EL3));
}

/*
 * Determine EL of the client. If EL2 is implemented (hence the enabled HCE
 * bit), deem EL2; otherwise, deem EL1.
 */
static inline unsigned int sdei_client_el(void)
{
	return read_scr_el3() & SCR_HCE_BIT ? MODE_EL2 : MODE_EL1;
}

static inline unsigned int sdei_event_priority(sdei_ev_map_t *map)
{
	return is_event_critical(map) ? PLAT_SDEI_CRITICAL_PRI :
		PLAT_SDEI_NORMAL_PRI;
}

static inline int get_ev_state_bit(sdei_entry_t *se, unsigned int bit_no)
{
	return ((se->state & BIT(bit_no)) != 0);
}

static inline void clr_ev_state_bit(sdei_entry_t *se, unsigned int bit_no)
{
	se->state &= ~BIT(bit_no);
}

/* SDEI actions for state transition */
typedef enum {
	/*
	 * Actions resulting from client requests. These directly map to SMC
	 * calls. Note that the state table columns are listed in this order
	 * too.
	 */
	DO_REGISTER = 0,
	DO_RELEASE = 1,
	DO_ENABLE = 2,
	DO_DISABLE = 3,
	DO_UNREGISTER = 4,
	DO_ROUTING = 5,
	DO_CONTEXT = 6,
	DO_COMPLETE = 7,
	DO_COMPLETE_RESUME = 8,

	/* Action for event dispatch */
	DO_DISPATCH = 9,

	DO_MAX,
} sdei_action_t;

typedef enum {
	SDEI_NORMAL,
	SDEI_CRITICAL
} sdei_class_t;

static inline void sdei_map_lock(sdei_ev_map_t *map)
{
	spin_lock(&map->lock);
}

static inline void sdei_map_unlock(sdei_ev_map_t *map)
{
	spin_unlock(&map->lock);
}

extern const sdei_mapping_t sdei_global_mappings[];
extern sdei_entry_t sdei_private_event_table[];
extern sdei_entry_t sdei_shared_event_table[];

void init_sdei_state(void);

sdei_ev_map_t *find_event_map_by_intr(int intr_num, int shared);
sdei_ev_map_t *find_event_map(int ev_num);
sdei_entry_t *get_event_entry(sdei_ev_map_t *map);

int sdei_event_context(void *handle, unsigned int param);
int sdei_event_complete(int resume, uint64_t arg);

void sdei_pe_unmask(void);
unsigned int sdei_pe_mask(void);

int sdei_intr_handler(uint32_t intr, uint32_t flags, void *handle,
		void *cookie);
bool can_sdei_state_trans(sdei_entry_t *se, sdei_action_t act);

#endif /* __SDEI_PRIVATE_H__ */
