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

#ifndef __ASSEMBLY__
/*******************************************************************************
 * The following two data structures hold the generic information to bringup
 * a suspended/hotplugged out cpu
 ******************************************************************************/
typedef struct {
	unsigned long entrypoint;
	unsigned long spsr;
} eret_params;

typedef struct {
	eret_params eret_info;
	unsigned long context_id;
	unsigned int scr;
	unsigned int sctlr;
} ns_entry_info;

/*******************************************************************************
 * The following two data structures hold the topology tree which in turn tracks
 * the state of the all the affinity instances supported by the platform.
 ******************************************************************************/
typedef struct {
	unsigned long mpidr;
	unsigned short ref_count;
	unsigned char state;
	unsigned char level;
	unsigned int data;
	bakery_lock lock;
} aff_map_node;

typedef struct {
	int min;
	int max;
} aff_limits_node;

/*******************************************************************************
 * This data structure holds secure world context that needs to be preserved
 * across cpu_suspend calls which enter the power down state.
 ******************************************************************************/
typedef struct {
	/* Align the suspend level to allow per-cpu lockless access */
	int suspend_level
	__attribute__((__aligned__(CACHE_WRITEBACK_GRANULE)));
	sysregs_context sec_sysregs;
} suspend_context;

typedef aff_map_node *mpidr_aff_map_nodes[MPIDR_MAX_AFFLVL];
typedef unsigned int (*afflvl_power_on_finisher)(unsigned long,
						 aff_map_node *);

/*******************************************************************************
 * Data prototypes
 ******************************************************************************/
extern suspend_context psci_suspend_context[PSCI_NUM_AFFS];
extern ns_entry_info psci_ns_entry_info[PSCI_NUM_AFFS];
extern unsigned int psci_ns_einfo_idx;
extern aff_limits_node psci_aff_limits[MPIDR_MAX_AFFLVL + 1];
extern plat_pm_ops *psci_plat_pm_ops;
extern aff_map_node psci_aff_map[PSCI_NUM_AFFS];
extern afflvl_power_on_finisher psci_afflvl_off_finish_handlers[];
extern afflvl_power_on_finisher psci_afflvl_sus_finish_handlers[];

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
/* Private exported functions from psci_common.c */
extern int get_max_afflvl(void);
extern unsigned short psci_get_state(aff_map_node *node);
extern unsigned short psci_get_phys_state(aff_map_node *node);
extern void psci_set_state(aff_map_node *node, unsigned short state);
extern void psci_get_ns_entry_info(unsigned int index);
extern unsigned long mpidr_set_aff_inst(unsigned long,unsigned char, int);
extern int psci_validate_mpidr(unsigned long, int);
extern int get_power_on_target_afflvl(unsigned long mpidr);
extern void psci_afflvl_power_on_finish(unsigned long,
						int,
						int,
						afflvl_power_on_finisher *);
extern int psci_set_ns_entry_info(unsigned int index,
				  unsigned long entrypoint,
				  unsigned long context_id);
extern int psci_check_afflvl_range(int start_afflvl, int end_afflvl);
extern void psci_acquire_afflvl_locks(unsigned long mpidr,
				      int start_afflvl,
				      int end_afflvl,
				      mpidr_aff_map_nodes mpidr_nodes);
extern void psci_release_afflvl_locks(unsigned long mpidr,
				      int start_afflvl,
				      int end_afflvl,
				      mpidr_aff_map_nodes mpidr_nodes);

/* Private exported functions from psci_setup.c */
extern int psci_get_aff_map_nodes(unsigned long mpidr,
				  int start_afflvl,
				  int end_afflvl,
				  mpidr_aff_map_nodes mpidr_nodes);
extern aff_map_node *psci_get_aff_map_node(unsigned long, int);

/* Private exported functions from psci_affinity_on.c */
extern int psci_afflvl_on(unsigned long,
			  unsigned long,
			  unsigned long,
			  int,
			  int);

/* Private exported functions from psci_affinity_off.c */
extern int psci_afflvl_off(unsigned long, int, int);

/* Private exported functions from psci_affinity_suspend.c */
extern void psci_set_suspend_afflvl(aff_map_node *node, int afflvl);
extern int psci_get_suspend_afflvl(aff_map_node *node);
extern int psci_afflvl_suspend(unsigned long,
			       unsigned long,
			       unsigned long,
			       unsigned int,
			       int,
			       int);
extern unsigned int psci_afflvl_suspend_finish(unsigned long, int, int);
#endif /*__ASSEMBLY__*/

#endif /* __PSCI_PRIVATE_H__ */
