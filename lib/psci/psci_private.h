/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef __PSCI_PRIVATE_H__
#define __PSCI_PRIVATE_H__

#include <arch.h>
#include <bakery_lock.h>
#include <bl_common.h>
#include <cpu_data.h>
#include <pmf.h>
#include <psci.h>
#include <spinlock.h>

/*
 * The following helper macros abstract the interface to the Bakery
 * Lock API.
 */
#define psci_lock_init(non_cpu_pd_node, idx)			\
	((non_cpu_pd_node)[(idx)].lock_index = (idx))
#define psci_lock_get(non_cpu_pd_node)				\
	bakery_lock_get(&psci_locks[(non_cpu_pd_node)->lock_index])
#define psci_lock_release(non_cpu_pd_node)			\
	bakery_lock_release(&psci_locks[(non_cpu_pd_node)->lock_index])

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
			define_psci_cap(PSCI_STAT_COUNT_AARCH64))

/*
 * Helper macros to get/set the fields of PSCI per-cpu data.
 */
#define psci_set_aff_info_state(aff_state) \
		set_cpu_data(psci_svc_cpu_data.aff_info_state, aff_state)
#define psci_get_aff_info_state() \
		get_cpu_data(psci_svc_cpu_data.aff_info_state)
#define psci_get_aff_info_state_by_idx(idx) \
		get_cpu_data_by_index(idx, psci_svc_cpu_data.aff_info_state)
#define psci_set_aff_info_state_by_idx(idx, aff_state) \
		set_cpu_data_by_index(idx, psci_svc_cpu_data.aff_info_state,\
					aff_state)
#define psci_get_suspend_pwrlvl() \
		get_cpu_data(psci_svc_cpu_data.target_pwrlvl)
#define psci_set_suspend_pwrlvl(target_lvl) \
		set_cpu_data(psci_svc_cpu_data.target_pwrlvl, target_lvl)
#define psci_set_cpu_local_state(state) \
		set_cpu_data(psci_svc_cpu_data.local_state, state)
#define psci_get_cpu_local_state() \
		get_cpu_data(psci_svc_cpu_data.local_state)
#define psci_get_cpu_local_state_by_idx(idx) \
		get_cpu_data_by_index(idx, psci_svc_cpu_data.local_state)

/*
 * Helper macros for the CPU level spinlocks
 */
#define psci_spin_lock_cpu(idx)	spin_lock(&psci_cpu_pd_nodes[idx].cpu_lock)
#define psci_spin_unlock_cpu(idx) spin_unlock(&psci_cpu_pd_nodes[idx].cpu_lock)

/* Helper macro to identify a CPU standby request in PSCI Suspend call */
#define is_cpu_standby_req(is_power_down_state, retn_lvl) \
		(((!(is_power_down_state)) && ((retn_lvl) == 0)) ? 1 : 0)

/* Following are used as ID's to capture time-stamp */
#define PSCI_STAT_ID_ENTER_LOW_PWR		0
#define PSCI_STAT_ID_EXIT_LOW_PWR		1
#define PSCI_STAT_TOTAL_IDS			2

/* Declare PMF service functions for PSCI */
PMF_DECLARE_CAPTURE_TIMESTAMP(psci_svc)
PMF_DECLARE_GET_TIMESTAMP(psci_svc)

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
	unsigned int cpu_start_idx;

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
 * Data prototypes
 ******************************************************************************/
extern const plat_psci_ops_t *psci_plat_pm_ops;
extern non_cpu_pd_node_t psci_non_cpu_pd_nodes[PSCI_NUM_NON_CPU_PWR_DOMAINS];
extern cpu_pd_node_t psci_cpu_pd_nodes[PLATFORM_CORE_COUNT];
extern unsigned int psci_caps;

/* One bakery lock is required for each non-cpu power domain */
DECLARE_BAKERY_LOCK(psci_locks[PSCI_NUM_NON_CPU_PWR_DOMAINS]);

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
void psci_get_parent_pwr_domain_nodes(unsigned int cpu_idx,
				      unsigned int end_lvl,
				      unsigned int node_index[]);
void psci_do_state_coordination(unsigned int end_pwrlvl,
				psci_power_state_t *state_info);
void psci_acquire_pwr_domain_locks(unsigned int end_pwrlvl,
				   unsigned int cpu_idx);
void psci_release_pwr_domain_locks(unsigned int end_pwrlvl,
				   unsigned int cpu_idx);
int psci_validate_suspend_req(const psci_power_state_t *state_info,
			      unsigned int is_power_down_state_req);
unsigned int psci_find_max_off_lvl(const psci_power_state_t *state_info);
unsigned int psci_find_target_suspend_lvl(const psci_power_state_t *state_info);
void psci_set_pwr_domains_to_run(unsigned int end_pwrlvl);
void psci_print_power_domain_map(void);
unsigned int psci_is_last_on_cpu(void);
int psci_spd_migrate_info(u_register_t *mpidr);

/* Private exported functions from psci_on.c */
int psci_cpu_on_start(u_register_t target_cpu,
		      entry_point_info_t *ep);

void psci_cpu_on_finish(unsigned int cpu_idx,
			psci_power_state_t *state_info);

/* Private exported functions from psci_off.c */
int psci_do_cpu_off(unsigned int end_pwrlvl);

/* Private exported functions from psci_suspend.c */
void psci_cpu_suspend_start(entry_point_info_t *ep,
			unsigned int end_pwrlvl,
			psci_power_state_t *state_info,
			unsigned int is_power_down_state_req);

void psci_cpu_suspend_finish(unsigned int cpu_idx,
			psci_power_state_t *state_info);

/* Private exported functions from psci_helpers.S */
void psci_do_pwrdown_cache_maintenance(unsigned int pwr_level);
void psci_do_pwrup_cache_maintenance(void);

/* Private exported functions from psci_system_off.c */
void __dead2 psci_system_off(void);
void __dead2 psci_system_reset(void);

/* Private exported functions from psci_stat.c */
void psci_stats_update_pwr_down(unsigned int end_pwrlvl,
			const psci_power_state_t *state_info);
void psci_stats_update_pwr_up(unsigned int end_pwrlvl,
			const psci_power_state_t *state_info,
			unsigned int flags);
u_register_t psci_stat_residency(u_register_t target_cpu,
			unsigned int power_state);
u_register_t psci_stat_count(u_register_t target_cpu,
			unsigned int power_state);

#endif /* __PSCI_PRIVATE_H__ */
