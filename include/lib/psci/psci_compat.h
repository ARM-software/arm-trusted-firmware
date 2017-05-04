/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PSCI_COMPAT_H__
#define __PSCI_COMPAT_H__

#include <arch.h>
#include <platform_def.h>

#ifndef __ASSEMBLY__
/*
 * The below declarations are to enable compatibility for the platform ports
 * using the old platform interface and psci helpers.
 */
#define PLAT_MAX_PWR_LVL	PLATFORM_MAX_AFFLVL
#define PLAT_NUM_PWR_DOMAINS	PLATFORM_NUM_AFFS

/*******************************************************************************
 * PSCI affinity related constants. An affinity instance could
 * be present or absent physically to cater for asymmetric topologies.
 ******************************************************************************/
#define PSCI_AFF_ABSENT		0x0
#define PSCI_AFF_PRESENT	0x1

#define PSCI_STATE_ON		0x0
#define PSCI_STATE_OFF		0x1
#define PSCI_STATE_ON_PENDING	0x2
#define PSCI_STATE_SUSPEND	0x3

/*
 * Using the compatibility platform interfaces means that the local states
 * used in psci_power_state_t need to only convey whether its power down
 * or standby state. The onus is on the platform port to do the right thing
 * including the state coordination in case multiple power down states are
 * involved. Hence if we assume 3 generic states viz, run, standby and
 * power down, we can assign 1 and 2 to standby and power down respectively.
 */
#define PLAT_MAX_RET_STATE	1
#define PLAT_MAX_OFF_STATE	2

/*
 * Macro to represent invalid affinity level within PSCI.
 */
#define PSCI_INVALID_DATA -1

#define psci_get_pstate_afflvl(pstate)		psci_get_pstate_pwrlvl(pstate)

/*
 * This array stores the 'power_state' requests of each CPU during
 * CPU_SUSPEND and SYSTEM_SUSPEND which will be populated by the
 * compatibility layer when appropriate platform hooks are invoked.
 */
extern unsigned int psci_power_state_compat[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * Structure populated by platform specific code to export routines which
 * perform common low level pm functions
 ******************************************************************************/
typedef struct plat_pm_ops {
	void (*affinst_standby)(unsigned int power_state);
	int (*affinst_on)(unsigned long mpidr,
			  unsigned long sec_entrypoint,
			  unsigned int afflvl,
			  unsigned int state);
	void (*affinst_off)(unsigned int afflvl, unsigned int state);
	void (*affinst_suspend)(unsigned long sec_entrypoint,
			       unsigned int afflvl,
			       unsigned int state);
	void (*affinst_on_finish)(unsigned int afflvl, unsigned int state);
	void (*affinst_suspend_finish)(unsigned int afflvl,
				      unsigned int state);
	void (*system_off)(void) __dead2;
	void (*system_reset)(void) __dead2;
	int (*validate_power_state)(unsigned int power_state);
	int (*validate_ns_entrypoint)(unsigned long ns_entrypoint);
	unsigned int (*get_sys_suspend_power_state)(void);
} plat_pm_ops_t;

/*******************************************************************************
 * Function & Data prototypes to enable compatibility for older platform ports
 ******************************************************************************/
int psci_get_suspend_stateid_by_mpidr(unsigned long);
int psci_get_suspend_stateid(void);
int psci_get_suspend_powerstate(void);
unsigned int psci_get_max_phys_off_afflvl(void);
int psci_get_suspend_afflvl(void);

#endif /* ____ASSEMBLY__ */
#endif /* __PSCI_COMPAT_H__ */
