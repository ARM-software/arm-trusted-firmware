/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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
#include <psci.h>
#include <spinlock.h>

/*
 * The following helper macros abstract the interface to the Bakery
 * Lock API.
 */
#if USE_COHERENT_MEM
#define psci_lock_init(non_cpu_pd_node, idx)	\
	bakery_lock_init(&(non_cpu_pd_node)[(idx)].lock)
#define psci_lock_get(non_cpu_pd_node)		\
	bakery_lock_get(&((non_cpu_pd_node)->lock))
#define psci_lock_release(non_cpu_pd_node)	\
	bakery_lock_release(&((non_cpu_pd_node)->lock))
#else
#define psci_lock_init(non_cpu_pd_node, idx)			\
	((non_cpu_pd_node)[(idx)].lock_index = (idx))
#define psci_lock_get(non_cpu_pd_node)				\
	bakery_lock_get((non_cpu_pd_node)->lock_index, 		\
			CPU_DATA_PSCI_LOCK_OFFSET)
#define psci_lock_release(non_cpu_pd_node)			\
	bakery_lock_release((non_cpu_pd_node)->lock_index,	\
			     CPU_DATA_PSCI_LOCK_OFFSET)
#endif

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
			define_psci_cap(PSCI_SYSTEM_SUSPEND_AARCH64))

/*
 * Helper macros for the CPU level spinlocks
 */
#define psci_spin_lock_cpu(idx)	spin_lock(&psci_cpu_pd_nodes[idx].cpu_lock)
#define psci_spin_unlock_cpu(idx) spin_unlock(&psci_cpu_pd_nodes[idx].cpu_lock)

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

	unsigned char ref_count;
	unsigned char level;
#if USE_COHERENT_MEM
	bakery_lock_t lock;
#else
	/* For indexing the bakery_info array in per CPU data */
	unsigned char lock_index;
#endif
} non_cpu_pd_node_t;

typedef struct cpu_pwr_domain_node {
	unsigned long mpidr;

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

typedef void (*pwrlvl_power_on_finisher_t)(unsigned int cpu_idx,
					   int max_off_pwrlvl);

/*******************************************************************************
 * Data prototypes
 ******************************************************************************/
extern const plat_pm_ops_t *psci_plat_pm_ops;
extern non_cpu_pd_node_t psci_non_cpu_pd_nodes[PSCI_NUM_NON_CPU_PWR_DOMAINS];
extern cpu_pd_node_t psci_cpu_pd_nodes[PLATFORM_CORE_COUNT];
extern uint32_t psci_caps;

/*******************************************************************************
 * SPD's power management hooks registered with PSCI
 ******************************************************************************/
extern const spd_pm_ops_t *psci_spd_pm;

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
/* Private exported functions from psci_common.c */
unsigned short psci_get_state(unsigned int idx, int level);
unsigned short psci_get_phys_state(unsigned int idx, int level);
void psci_set_state(unsigned int idx, unsigned short state, int level);
int psci_validate_mpidr(unsigned long mpidr);
int get_power_on_target_pwrlvl(void);
void psci_power_up_finish(int end_pwrlvl,
				 pwrlvl_power_on_finisher_t pon_handler);
int psci_get_ns_ep_info(entry_point_info_t *ep,
		       uint64_t entrypoint, uint64_t context_id);
void psci_get_parent_pwr_domain_nodes(unsigned int cpu_idx,
				      int end_lvl,
				      unsigned int node_index[]);
void psci_do_state_coordination(int end_pwrlvl,
				unsigned int cpu_idx,
				uint32_t state);
void psci_acquire_pwr_domain_locks(int end_pwrlvl,
				   unsigned int cpu_idx);
void psci_release_pwr_domain_locks(int end_pwrlvl,
				   unsigned int cpu_idx);
void psci_print_power_domain_map(void);
uint32_t psci_find_max_phys_off_pwrlvl(uint32_t end_pwrlvl,
			       unsigned int cpu_idx);
unsigned int psci_is_last_on_cpu(void);
int psci_spd_migrate_info(uint64_t *mpidr);

/* Private exported functions from psci_on.c */
int psci_cpu_on_start(unsigned long target_cpu,
		      entry_point_info_t *ep,
		      int end_pwrlvl);

void psci_cpu_on_finish(unsigned int cpu_idx,
			int max_off_pwrlvl);

/* Private exported functions from psci_cpu_off.c */
int psci_do_cpu_off(int end_pwrlvl);

/* Private exported functions from psci_suspend.c */
void psci_cpu_suspend_start(entry_point_info_t *ep,
			int end_pwrlvl);
void psci_cpu_suspend_finish(unsigned int cpu_idx,
			     int max_off_pwrlvl);

void psci_set_suspend_power_state(unsigned int power_state);

/* Private exported functions from psci_helpers.S */
void psci_do_pwrdown_cache_maintenance(uint32_t pwr_level);
void psci_do_pwrup_cache_maintenance(void);

/* Private exported functions from psci_system_off.c */
void __dead2 psci_system_off(void);
void __dead2 psci_system_reset(void);

#endif /* __PSCI_PRIVATE_H__ */
