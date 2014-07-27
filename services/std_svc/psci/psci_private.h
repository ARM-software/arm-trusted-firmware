/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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
#include <platform_def.h>	/* for PLATFORM_NUM_AFFS */
#include <psci.h>

/* Number of affinity instances whose state this psci imp. can track */
#ifdef PLATFORM_NUM_AFFS
#define PSCI_NUM_AFFS		PLATFORM_NUM_AFFS
#else
#define PSCI_NUM_AFFS		(2 * PLATFORM_CORE_COUNT)
#endif

/*******************************************************************************
 * The following two data structures hold the topology tree which in turn tracks
 * the state of the all the affinity instances supported by the platform.
 ******************************************************************************/
typedef struct aff_map_node {
	unsigned long mpidr;
	unsigned short ref_count;
	unsigned char state;
	unsigned char level;
	bakery_lock_t lock;
} aff_map_node_t;

typedef struct aff_limits_node {
	int min;
	int max;
} aff_limits_node_t;

typedef aff_map_node_t (*mpidr_aff_map_nodes_t[MPIDR_MAX_AFFLVL]);
typedef unsigned int (*afflvl_power_on_finisher_t)(aff_map_node_t *);

/*******************************************************************************
 * Data prototypes
 ******************************************************************************/
extern const plat_pm_ops_t *psci_plat_pm_ops;
extern aff_map_node_t psci_aff_map[PSCI_NUM_AFFS];

/*******************************************************************************
 * SPD's power management hooks registered with PSCI
 ******************************************************************************/
extern const spd_pm_ops_t *psci_spd_pm;

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
/* Private exported functions from psci_common.c */
int get_max_afflvl(void);
unsigned short psci_get_state(aff_map_node_t *node);
unsigned short psci_get_phys_state(aff_map_node_t *node);
void psci_set_state(aff_map_node_t *node, unsigned short state);
unsigned long mpidr_set_aff_inst(unsigned long, unsigned char, int);
int psci_validate_mpidr(unsigned long, int);
int get_power_on_target_afflvl(void);
void psci_afflvl_power_on_finish(int,
				int,
				afflvl_power_on_finisher_t *);
int psci_save_ns_entry(uint64_t mpidr,
		       uint64_t entrypoint, uint64_t context_id,
		       uint32_t caller_scr_el3, uint32_t caller_sctlr_el1);
int psci_check_afflvl_range(int start_afflvl, int end_afflvl);
void psci_do_afflvl_state_mgmt(uint32_t start_afflvl,
			       uint32_t end_afflvl,
			       aff_map_node_t *mpidr_nodes[],
			       uint32_t state);
void psci_acquire_afflvl_locks(int start_afflvl,
			       int end_afflvl,
			       aff_map_node_t *mpidr_nodes[]);
void psci_release_afflvl_locks(int start_afflvl,
				int end_afflvl,
				mpidr_aff_map_nodes_t mpidr_nodes);
void psci_print_affinity_map(void);
void psci_set_max_phys_off_afflvl(uint32_t afflvl);
uint32_t psci_find_max_phys_off_afflvl(uint32_t start_afflvl,
				       uint32_t end_afflvl,
				       aff_map_node_t *mpidr_nodes[]);

/* Private exported functions from psci_setup.c */
int psci_get_aff_map_nodes(unsigned long mpidr,
				int start_afflvl,
				int end_afflvl,
				aff_map_node_t *mpidr_nodes[]);
aff_map_node_t *psci_get_aff_map_node(unsigned long, int);

/* Private exported functions from psci_affinity_on.c */
int psci_afflvl_on(unsigned long,
			unsigned long,
			unsigned long,
			int,
			int);

/* Private exported functions from psci_affinity_off.c */
int psci_afflvl_off(int, int);

/* Private exported functions from psci_affinity_suspend.c */
int psci_afflvl_suspend(unsigned long,
			unsigned long,
			unsigned int,
			int,
			int);
unsigned int psci_afflvl_suspend_finish(int, int);
void psci_set_suspend_power_state(unsigned int power_state);

/* Private exported functions from psci_helpers.S */
void psci_do_pwrdown_cache_maintenance(uint32_t affinity_level);
void psci_do_pwrup_cache_maintenance(void);

/* Private exported functions from psci_system_off.c */
void __dead2 psci_system_off(void);
void __dead2 psci_system_reset(void);

#endif /* __PSCI_PRIVATE_H__ */
