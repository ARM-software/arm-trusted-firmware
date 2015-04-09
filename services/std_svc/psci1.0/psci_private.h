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
#include <psci.h>

/*
 * The following helper macros abstract the interface to the Bakery
 * Lock API.
 */
#if USE_COHERENT_MEM
#define psci_lock_init(pwr_map, idx)	bakery_lock_init(&(pwr_map)[(idx)].lock)
#define psci_lock_get(node)		bakery_lock_get(&((node)->lock))
#define psci_lock_release(node)		bakery_lock_release(&((node)->lock))
#else
#define psci_lock_init(pwr_map, idx)	\
				((pwr_map)[(idx)].pwr_domain_index = (idx))
#define psci_lock_get(node)		\
				bakery_lock_get((node)->pwr_domain_index,\
						CPU_DATA_PSCI_LOCK_OFFSET)
#define psci_lock_release(node)		\
				bakery_lock_release((node)->pwr_domain_index,\
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


/*******************************************************************************
 * The following two data structures hold the topology tree which in turn tracks
 * the state of the all the power domain instances supported by the platform.
 ******************************************************************************/
typedef struct pwr_map_node {
	unsigned long mpidr;
	unsigned char ref_count;
	unsigned char state;
	unsigned char level;
#if USE_COHERENT_MEM
	bakery_lock_t lock;
#else
	/* For indexing the bakery_info array in per CPU data */
	unsigned char pwr_domain_index;
#endif
} pwr_map_node_t;

typedef struct pwr_lvl_limits_node {
	int min;
	int max;
} pwr_lvl_limits_node_t;

typedef pwr_map_node_t (*mpidr_pwr_map_nodes_t[MPIDR_MAX_AFFLVL + 1]);
typedef void (*pwrlvl_power_on_finisher_t)(pwr_map_node_t *mpidr_nodes[],
					int pwrlvl);

/*******************************************************************************
 * Data prototypes
 ******************************************************************************/
extern const plat_pm_ops_t *psci_plat_pm_ops;
extern pwr_map_node_t psci_pwr_domain_map[PSCI_NUM_PWR_DOMAINS];
extern pwr_lvl_limits_node_t psci_pwr_domain_map[MPIDR_MAX_AFFLVL + 1];
extern uint32_t psci_caps;

/*******************************************************************************
 * SPD's power management hooks registered with PSCI
 ******************************************************************************/
extern const spd_pm_ops_t *psci_spd_pm;

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
/* Private exported functions from psci_common.c */
unsigned short psci_get_state(pwr_map_node_t *node);
unsigned short psci_get_phys_state(pwr_map_node_t *node);
void psci_set_state(pwr_map_node_t *node, unsigned short state);
unsigned long mpidr_set_pwr_domain_inst(unsigned long, unsigned char, int);
int psci_validate_mpidr(unsigned long mpidr);
int get_power_on_target_pwrlvl(void);
void psci_power_up_finish(int end_pwrlvl,
				 pwrlvl_power_on_finisher_t pon_handler);
int psci_get_ns_ep_info(entry_point_info_t *ep,
		       uint64_t entrypoint, uint64_t context_id);
int psci_check_pwrlvl_range(int start_pwrlvl, int end_pwrlvl);
void psci_do_state_coordination(uint32_t start_pwrlvl,
			       uint32_t end_pwrlvl,
			       pwr_map_node_t *mpidr_nodes[],
			       uint32_t state);
void psci_acquire_pwr_domain_locks(int start_pwrlvl,
			       int end_pwrlvl,
			       pwr_map_node_t *mpidr_nodes[]);
void psci_release_pwr_domain_locks(int start_pwrlvl,
				int end_pwrlvl,
				mpidr_pwr_map_nodes_t mpidr_nodes);
void psci_print_power_domain_map(void);
uint32_t psci_find_max_phys_off_pwrlvl(uint32_t start_pwrlvl,
				       uint32_t end_pwrlvl,
				       pwr_map_node_t *mpidr_nodes[]);
unsigned int psci_is_last_on_cpu(void);
int psci_spd_migrate_info(uint64_t *mpidr);

/* Private exported functions from psci_setup.c */
int psci_get_pwr_map_nodes(unsigned long mpidr,
				int start_pwrlvl,
				int end_pwrlvl,
				pwr_map_node_t *mpidr_nodes[]);
pwr_map_node_t *psci_get_pwr_map_node(unsigned long, int);

/* Private exported functions from psci_cpu_on.c */
int psci_cpu_on_start(unsigned long target_cpu,
		   entry_point_info_t *ep,
		   int end_pwrlvl);

void psci_cpu_on_finish(pwr_map_node_t *node[], int pwrlvl);

/* Private exported functions from psci_cpu_off.c */
int psci_do_cpu_off(int end_pwrlvl);

/* Private exported functions from psci_cpu_suspend.c */
void psci_cpu_suspend_start(entry_point_info_t *ep,
			int end_pwrlvl);

void psci_cpu_suspend_finish(pwr_map_node_t *node[], int pwrlvl);

void psci_set_suspend_power_state(unsigned int power_state);

/* Private exported functions from psci_helpers.S */
void psci_do_pwrdown_cache_maintenance(uint32_t pwr_level);
void psci_do_pwrup_cache_maintenance(void);

/* Private exported functions from psci_system_off.c */
void __dead2 psci_system_off(void);
void __dead2 psci_system_reset(void);

#endif /* __PSCI_PRIVATE_H__ */
