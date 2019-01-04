/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PSCI_PRIVATE_H
#define PSCI_PRIVATE_H

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <lib/bakery_lock.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>

/*
 * The PSCI capability which are provided by the generic code but does not
 * depend on the platform or spd capabilities.
 */
#define PSCI_GENERIC_CAP	\
			(define_psci_cap(PSCI_VERSION) |		\
			define_psci_cap(PSCI_AFFINITY_INFO_AARCH64) |	\
			define_psci_cap(PSCI_FEATURES))

/*
 * The PSCI capabilities mask for 64 bit functions.
 */
#define PSCI_CAP_64BIT_MASK	\
			(define_psci_cap(PSCI_CPU_SUSPEND_AARCH64) |	\
			define_psci_cap(PSCI_CPU_ON_AARCH64) |		\
			define_psci_cap(PSCI_AFFINITY_INFO_AARCH64) |	\
			define_psci_cap(PSCI_MIG_AARCH64) |		\
			define_psci_cap(PSCI_MIG_INFO_UP_CPU_AARCH64) |	\
			define_psci_cap(PSCI_NODE_HW_STATE_AARCH64) |	\
			define_psci_cap(PSCI_SYSTEM_SUSPEND_AARCH64) |	\
			define_psci_cap(PSCI_STAT_RESIDENCY_AARCH64) |	\
			define_psci_cap(PSCI_STAT_COUNT_AARCH64) |	\
			define_psci_cap(PSCI_SYSTEM_RESET2_AARCH64) |	\
			define_psci_cap(PSCI_MEM_CHK_RANGE_AARCH64))

/*
 * Helper functions to get/set the fields of PSCI per-cpu data.
 */
static inline void psci_set_aff_info_state(aff_info_state_t aff_state)
{
	set_cpu_data(psci_svc_cpu_data.aff_info_state, aff_state);
}

static inline aff_info_state_t psci_get_aff_info_state(void)
{
	return get_cpu_data(psci_svc_cpu_data.aff_info_state);
}

static inline aff_info_state_t psci_get_aff_info_state_by_idx(int idx)
{
	return get_cpu_data_by_index((unsigned int)idx,
				     psci_svc_cpu_data.aff_info_state);
}

static inline void psci_set_aff_info_state_by_idx(int idx,
						  aff_info_state_t aff_state)
{
	set_cpu_data_by_index((unsigned int)idx,
			      psci_svc_cpu_data.aff_info_state, aff_state);
}

static inline unsigned int psci_get_suspend_pwrlvl(void)
{
	return get_cpu_data(psci_svc_cpu_data.target_pwrlvl);
}

static inline void psci_set_suspend_pwrlvl(unsigned int target_lvl)
{
	set_cpu_data(psci_svc_cpu_data.target_pwrlvl, target_lvl);
}

static inline void psci_set_cpu_local_state(plat_local_state_t state)
{
	set_cpu_data(psci_svc_cpu_data.local_state, state);
}

static inline plat_local_state_t psci_get_cpu_local_state(void)
{
	return get_cpu_data(psci_svc_cpu_data.local_state);
}

static inline plat_local_state_t psci_get_cpu_local_state_by_idx(int idx)
{
	return get_cpu_data_by_index((unsigned int)idx,
				     psci_svc_cpu_data.local_state);
}

/* Helper function to identify a CPU standby request in PSCI Suspend call */
static inline bool is_cpu_standby_req(unsigned int is_power_down_state,
				      unsigned int retn_lvl)
{
	return (is_power_down_state == 0U) && (retn_lvl == 0U);
}

/*******************************************************************************
 * The following two data structures implement the power domain tree. The tree
 * is used to track the state of all the nodes i.e. power domain instances
 * described by the platform. The tree consists of nodes that describe CPU power
 * domains i.e. leaf nodes and all other power domains which are parents of a
 * CPU power domain i.e. non-leaf nodes.
 ******************************************************************************/
typedef struct non_cpu_pwr_domain_node {
	/*
	 * Index of the first CPU power domain node level 0 which has this node
	 * as its parent.
	 */
	int cpu_start_idx;

	/*
	 * Number of CPU power domains which are siblings of the domain indexed
	 * by 'cpu_start_idx' i.e. all the domains in the range 'cpu_start_idx
	 * -> cpu_start_idx + ncpus' have this node as their parent.
	 */
	unsigned int ncpus;

	/*
	 * Index of the parent power domain node.
	 * TODO: Figure out whether to whether using pointer is more efficient.
	 */
	unsigned int parent_node;

	plat_local_state_t local_state;

	unsigned char level;

	/* For indexing the psci_lock array*/
	unsigned char lock_index;
} non_cpu_pd_node_t;

typedef struct cpu_pwr_domain_node {
	u_register_t mpidr;

	/*
	 * Index of the parent power domain node.
	 * TODO: Figure out whether to whether using pointer is more efficient.
	 */
	unsigned int parent_node;

	/*
	 * A CPU power domain does not require state coordination like its
	 * parent power domains. Hence this node does not include a bakery
	 * lock. A spinlock is required by the CPU_ON handler to prevent a race
	 * when multiple CPUs try to turn ON the same target CPU.
	 */
	spinlock_t cpu_lock;
} cpu_pd_node_t;

/*******************************************************************************
 * The following are helpers and declarations of locks.
 ******************************************************************************/
#if HW_ASSISTED_COHERENCY
/*
 * On systems where participant CPUs are cache-coherent, we can use spinlocks
 * instead of bakery locks.
 */
#define DEFINE_PSCI_LOCK(_name)		spinlock_t _name
#define DECLARE_PSCI_LOCK(_name)	extern DEFINE_PSCI_LOCK(_name)

/* One lock is required per non-CPU power domain node */
DECLARE_PSCI_LOCK(psci_locks[PSCI_NUM_NON_CPU_PWR_DOMAINS]);

/*
 * On systems with hardware-assisted coherency, make PSCI cache operations NOP,
 * as PSCI participants are cache-coherent, and there's no need for explicit
 * cache maintenance operations or barriers to coordinate their state.
 */
static inline void psci_flush_dcache_range(uintptr_t __unused addr,
					   size_t __unused size)
{
	/* Empty */
}

#define psci_flush_cpu_data(member)
#define psci_inv_cpu_data(member)

static inline void psci_dsbish(void)
{
	/* Empty */
}

static inline void psci_lock_get(non_cpu_pd_node_t *non_cpu_pd_node)
{
	spin_lock(&psci_locks[non_cpu_pd_node->lock_index]);
}

static inline void psci_lock_release(non_cpu_pd_node_t *non_cpu_pd_node)
{
	spin_unlock(&psci_locks[non_cpu_pd_node->lock_index]);
}

#else /* if HW_ASSISTED_COHERENCY == 0 */
/*
 * Use bakery locks for state coordination as not all PSCI participants are
 * cache coherent.
 */
#define DEFINE_PSCI_LOCK(_name)		DEFINE_BAKERY_LOCK(_name)
#define DECLARE_PSCI_LOCK(_name)	DECLARE_BAKERY_LOCK(_name)

/* One lock is required per non-CPU power domain node */
DECLARE_PSCI_LOCK(psci_locks[PSCI_NUM_NON_CPU_PWR_DOMAINS]);

/*
 * If not all PSCI participants are cache-coherent, perform cache maintenance
 * and issue barriers wherever required to coordinate state.
 */
static inline void psci_flush_dcache_range(uintptr_t addr, size_t size)
{
	flush_dcache_range(addr, size);
}

#define psci_flush_cpu_data(member)		flush_cpu_data(member)
#define psci_inv_cpu_data(member)		inv_cpu_data(member)

static inline void psci_dsbish(void)
{
	dsbish();
}

static inline void psci_lock_get(non_cpu_pd_node_t *non_cpu_pd_node)
{
	bakery_lock_get(&psci_locks[non_cpu_pd_node->lock_index]);
}

static inline void psci_lock_release(non_cpu_pd_node_t *non_cpu_pd_node)
{
	bakery_lock_release(&psci_locks[non_cpu_pd_node->lock_index]);
}

#endif /* HW_ASSISTED_COHERENCY */

static inline void psci_lock_init(non_cpu_pd_node_t *non_cpu_pd_node,
				  unsigned char idx)
{
	non_cpu_pd_node[idx].lock_index = idx;
}

/*******************************************************************************
 * Data prototypes
 ******************************************************************************/
extern const plat_psci_ops_t *psci_plat_pm_ops;
extern non_cpu_pd_node_t psci_non_cpu_pd_nodes[PSCI_NUM_NON_CPU_PWR_DOMAINS];
extern cpu_pd_node_t psci_cpu_pd_nodes[PLATFORM_CORE_COUNT];
extern unsigned int psci_caps;

/*******************************************************************************
 * SPD's power management hooks registered with PSCI
 ******************************************************************************/
extern const spd_pm_ops_t *psci_spd_pm;

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
/* Private exported functions from psci_common.c */
int psci_validate_power_state(unsigned int power_state,
			      psci_power_state_t *state_info);
void psci_query_sys_suspend_pwrstate(psci_power_state_t *state_info);
int psci_validate_mpidr(u_register_t mpidr);
void psci_init_req_local_pwr_states(void);
void psci_get_target_local_pwr_states(unsigned int end_pwrlvl,
				      psci_power_state_t *target_state);
int psci_validate_entry_point(entry_point_info_t *ep,
			uintptr_t entrypoint, u_register_t context_id);
void psci_get_parent_pwr_domain_nodes(int cpu_idx,
				      unsigned int end_lvl,
				      unsigned int *node_index);
void psci_do_state_coordination(unsigned int end_pwrlvl,
				psci_power_state_t *state_info);
void psci_acquire_pwr_domain_locks(unsigned int end_pwrlvl, int cpu_idx);
void psci_release_pwr_domain_locks(unsigned int end_pwrlvl, int cpu_idx);
int psci_validate_suspend_req(const psci_power_state_t *state_info,
			      unsigned int is_power_down_state);
unsigned int psci_find_max_off_lvl(const psci_power_state_t *state_info);
unsigned int psci_find_target_suspend_lvl(const psci_power_state_t *state_info);
void psci_set_pwr_domains_to_run(unsigned int end_pwrlvl);
void psci_print_power_domain_map(void);
unsigned int psci_is_last_on_cpu(void);
int psci_spd_migrate_info(u_register_t *mpidr);
void psci_do_pwrdown_sequence(unsigned int power_level);

/*
 * CPU power down is directly called only when HW_ASSISTED_COHERENCY is
 * available. Otherwise, this needs post-call stack maintenance, which is
 * handled in assembly.
 */
void prepare_cpu_pwr_dwn(unsigned int power_level);

/* Private exported functions from psci_on.c */
int psci_cpu_on_start(u_register_t target_cpu,
		      const entry_point_info_t *ep);

void psci_cpu_on_finish(int cpu_idx, const psci_power_state_t *state_info);

/* Private exported functions from psci_off.c */
int psci_do_cpu_off(unsigned int end_pwrlvl);

/* Private exported functions from psci_suspend.c */
void psci_cpu_suspend_start(const entry_point_info_t *ep,
			unsigned int end_pwrlvl,
			psci_power_state_t *state_info,
			unsigned int is_power_down_state);

void psci_cpu_suspend_finish(int cpu_idx, const psci_power_state_t *state_info);

/* Private exported functions from psci_helpers.S */
void psci_do_pwrdown_cache_maintenance(unsigned int pwr_level);
void psci_do_pwrup_cache_maintenance(void);

/* Private exported functions from psci_system_off.c */
void __dead2 psci_system_off(void);
void __dead2 psci_system_reset(void);
u_register_t psci_system_reset2(uint32_t reset_type, u_register_t cookie);

/* Private exported functions from psci_stat.c */
void psci_stats_update_pwr_down(unsigned int end_pwrlvl,
			const psci_power_state_t *state_info);
void psci_stats_update_pwr_up(unsigned int end_pwrlvl,
			const psci_power_state_t *state_info);
u_register_t psci_stat_residency(u_register_t target_cpu,
			unsigned int power_state);
u_register_t psci_stat_count(u_register_t target_cpu,
			unsigned int power_state);

/* Private exported functions from psci_mem_protect.c */
u_register_t psci_mem_protect(unsigned int enable);
u_register_t psci_mem_chk_range(uintptr_t base, u_register_t length);

#endif /* PSCI_PRIVATE_H */
