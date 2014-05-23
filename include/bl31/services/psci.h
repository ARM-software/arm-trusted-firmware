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

#ifndef __PSCI_H__
#define __PSCI_H__


/*******************************************************************************
 * Defines for runtime services func ids
 ******************************************************************************/
#define PSCI_VERSION			0x84000000
#define PSCI_CPU_SUSPEND_AARCH32	0x84000001
#define PSCI_CPU_SUSPEND_AARCH64	0xc4000001
#define PSCI_CPU_OFF			0x84000002
#define PSCI_CPU_ON_AARCH32		0x84000003
#define PSCI_CPU_ON_AARCH64		0xc4000003
#define PSCI_AFFINITY_INFO_AARCH32	0x84000004
#define PSCI_AFFINITY_INFO_AARCH64	0xc4000004
#define PSCI_MIG_AARCH32		0x84000005
#define PSCI_MIG_AARCH64		0xc4000005
#define PSCI_MIG_INFO_TYPE		0x84000006
#define PSCI_MIG_INFO_UP_CPU_AARCH32	0x84000007
#define PSCI_MIG_INFO_UP_CPU_AARCH64	0xc4000007
#define PSCI_SYSTEM_OFF			0x84000008
#define PSCI_SYSTEM_RESET		0x84000009

/*
 * Number of PSCI calls (above) implemented. System off and reset aren't
 * implemented as yet
 */
#define PSCI_NUM_CALLS			13

/*******************************************************************************
 * PSCI Migrate and friends
 ******************************************************************************/
#define PSCI_TOS_UP_MIG_CAP	0
#define PSCI_TOS_NOT_UP_MIG_CAP	1
#define PSCI_TOS_NOT_PRESENT_MP	2

/*******************************************************************************
 * PSCI CPU_SUSPEND 'power_state' parameter specific defines
 ******************************************************************************/
#define PSTATE_ID_SHIFT		0
#define PSTATE_TYPE_SHIFT	16
#define PSTATE_AFF_LVL_SHIFT	24

#define PSTATE_ID_MASK		0xffff
#define PSTATE_TYPE_MASK	0x1
#define PSTATE_AFF_LVL_MASK	0x3
#define PSTATE_VALID_MASK     0xFCFE0000

#define PSTATE_TYPE_STANDBY	0x0
#define PSTATE_TYPE_POWERDOWN	0x1

#define psci_get_pstate_id(pstate)	(pstate >> PSTATE_ID_SHIFT) & \
					PSTATE_ID_MASK
#define psci_get_pstate_type(pstate)	(pstate >> PSTATE_TYPE_SHIFT) & \
					PSTATE_TYPE_MASK
#define psci_get_pstate_afflvl(pstate)	(pstate >> PSTATE_AFF_LVL_SHIFT) & \
					PSTATE_AFF_LVL_MASK

/*******************************************************************************
 * PSCI version
 ******************************************************************************/
#define PSCI_MAJOR_VER		(0 << 16)
#define PSCI_MINOR_VER		0x2

/*******************************************************************************
 * PSCI error codes
 ******************************************************************************/
#define PSCI_E_SUCCESS		0
#define PSCI_E_NOT_SUPPORTED	-1
#define PSCI_E_INVALID_PARAMS	-2
#define PSCI_E_DENIED		-3
#define PSCI_E_ALREADY_ON	-4
#define PSCI_E_ON_PENDING	-5
#define PSCI_E_INTERN_FAIL	-6
#define PSCI_E_NOT_PRESENT	-7
#define PSCI_E_DISABLED		-8

/*******************************************************************************
 * PSCI affinity state related constants. An affinity instance could be present
 * or absent physically to cater for asymmetric topologies. If present then it
 * could in one of the 4 further defined states.
 ******************************************************************************/
#define PSCI_STATE_SHIFT	1
#define PSCI_STATE_MASK		0xff

#define PSCI_AFF_ABSENT		0x0
#define PSCI_AFF_PRESENT	0x1
#define PSCI_STATE_ON		0x0
#define PSCI_STATE_OFF		0x1
#define PSCI_STATE_ON_PENDING	0x2
#define PSCI_STATE_SUSPEND	0x3

#define PSCI_INVALID_DATA -1

#define get_phys_state(x)	(x != PSCI_STATE_ON ? \
				 PSCI_STATE_OFF : PSCI_STATE_ON)

#define psci_validate_power_state(pstate) (pstate & PSTATE_VALID_MASK)


/* Number of affinity instances whose state this psci imp. can track */
#define PSCI_NUM_AFFS		32ull

#ifndef __ASSEMBLY__

#include <stdint.h>


/*******************************************************************************
 * Structure populated by platform specific code to export routines which
 * perform common low level pm functions
 ******************************************************************************/
typedef struct plat_pm_ops {
	int (*affinst_standby)(unsigned int);
	int (*affinst_on)(unsigned long,
			  unsigned long,
			  unsigned long,
			  unsigned int,
			  unsigned int);
	int (*affinst_off)(unsigned long, unsigned int, unsigned int);
	int (*affinst_suspend)(unsigned long,
			       unsigned long,
			       unsigned long,
			       unsigned int,
			       unsigned int);
	int (*affinst_on_finish)(unsigned long, unsigned int, unsigned int);
	int (*affinst_suspend_finish)(unsigned long,
				      unsigned int,
				      unsigned int);
} plat_pm_ops_t;

/*******************************************************************************
 * Optional structure populated by the Secure Payload Dispatcher to be given a
 * chance to perform any bookkeeping before PSCI executes a power mgmt.
 * operation. It also allows PSCI to determine certain properties of the SP e.g.
 * migrate capability etc.
 ******************************************************************************/
typedef struct spd_pm_ops {
	void (*svc_on)(uint64_t target_cpu);
	int32_t (*svc_off)(uint64_t __unused);
	void (*svc_suspend)(uint64_t power_state);
	void (*svc_on_finish)(uint64_t __unused);
	void (*svc_suspend_finish)(uint64_t suspend_level);
	void (*svc_migrate)(uint64_t __unused1, uint64_t __unused2);
	int32_t (*svc_migrate_info)(uint64_t *__unused);
} spd_pm_ops_t;

/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
unsigned int psci_version(void);
int __psci_cpu_suspend(unsigned int, unsigned long, unsigned long);
int __psci_cpu_off(void);
int psci_affinity_info(unsigned long, unsigned int);
int psci_migrate(unsigned int);
unsigned int psci_migrate_info_type(void);
unsigned long psci_migrate_info_up_cpu(void);
void psci_system_off(void);
void psci_system_reset(void);
int psci_cpu_on(unsigned long,
		unsigned long,
		unsigned long);
void __dead2 psci_power_down_wfi(void);
void psci_aff_on_finish_entry(void);
void psci_aff_suspend_finish_entry(void);
void psci_register_spd_pm_hook(const spd_pm_ops_t *);
int psci_get_suspend_stateid(unsigned long mpidr);
int psci_get_suspend_afflvl(unsigned long mpidr);

uint64_t psci_smc_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags);

/* PSCI setup function */
int32_t psci_setup(void);


#endif /*__ASSEMBLY__*/


#endif /* __PSCI_H__ */
