/*
 * Copyright (c) 2013-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PSCI_H
#define PSCI_H

#include <platform_def.h>	/* for PLAT_NUM_PWR_DOMAINS */

#include <common/bl_common.h>
#include <lib/bakery_lock.h>
#include <lib/psci/psci_lib.h>	/* To maintain compatibility for SPDs */
#include <lib/utils_def.h>

/*******************************************************************************
 * Number of power domains whose state this PSCI implementation can track
 ******************************************************************************/
#ifdef PLAT_NUM_PWR_DOMAINS
#define PSCI_NUM_PWR_DOMAINS	PLAT_NUM_PWR_DOMAINS
#else
#define PSCI_NUM_PWR_DOMAINS	(U(2) * PLATFORM_CORE_COUNT)
#endif

#define PSCI_NUM_NON_CPU_PWR_DOMAINS	(PSCI_NUM_PWR_DOMAINS - \
					 PLATFORM_CORE_COUNT)

/* This is the power level corresponding to a CPU */
#define PSCI_CPU_PWR_LVL	U(0)

/*
 * The maximum power level supported by PSCI. Since PSCI CPU_SUSPEND
 * uses the old power_state parameter format which has 2 bits to specify the
 * power level, this constant is defined to be 3.
 */
#define PSCI_MAX_PWR_LVL	U(3)

/*******************************************************************************
 * Defines for runtime services function ids
 ******************************************************************************/
#define PSCI_VERSION			U(0x84000000)
#define PSCI_CPU_SUSPEND_AARCH32	U(0x84000001)
#define PSCI_CPU_SUSPEND_AARCH64	U(0xc4000001)
#define PSCI_CPU_OFF			U(0x84000002)
#define PSCI_CPU_ON_AARCH32		U(0x84000003)
#define PSCI_CPU_ON_AARCH64		U(0xc4000003)
#define PSCI_AFFINITY_INFO_AARCH32	U(0x84000004)
#define PSCI_AFFINITY_INFO_AARCH64	U(0xc4000004)
#define PSCI_MIG_AARCH32		U(0x84000005)
#define PSCI_MIG_AARCH64		U(0xc4000005)
#define PSCI_MIG_INFO_TYPE		U(0x84000006)
#define PSCI_MIG_INFO_UP_CPU_AARCH32	U(0x84000007)
#define PSCI_MIG_INFO_UP_CPU_AARCH64	U(0xc4000007)
#define PSCI_SYSTEM_OFF			U(0x84000008)
#define PSCI_SYSTEM_RESET		U(0x84000009)
#define PSCI_FEATURES			U(0x8400000A)
#define PSCI_NODE_HW_STATE_AARCH32	U(0x8400000d)
#define PSCI_NODE_HW_STATE_AARCH64	U(0xc400000d)
#define PSCI_SYSTEM_SUSPEND_AARCH32	U(0x8400000E)
#define PSCI_SYSTEM_SUSPEND_AARCH64	U(0xc400000E)
#define PSCI_SET_SUSPEND_MODE		U(0x8400000F)
#define PSCI_STAT_RESIDENCY_AARCH32	U(0x84000010)
#define PSCI_STAT_RESIDENCY_AARCH64	U(0xc4000010)
#define PSCI_STAT_COUNT_AARCH32		U(0x84000011)
#define PSCI_STAT_COUNT_AARCH64		U(0xc4000011)
#define PSCI_SYSTEM_RESET2_AARCH32	U(0x84000012)
#define PSCI_SYSTEM_RESET2_AARCH64	U(0xc4000012)
#define PSCI_MEM_PROTECT		U(0x84000013)
#define PSCI_MEM_CHK_RANGE_AARCH32	U(0x84000014)
#define PSCI_MEM_CHK_RANGE_AARCH64	U(0xc4000014)

/*
 * Number of PSCI calls (above) implemented
 */
#if ENABLE_PSCI_STAT
#if PSCI_OS_INIT_MODE
#define PSCI_NUM_CALLS			U(30)
#else
#define PSCI_NUM_CALLS			U(29)
#endif
#else
#if PSCI_OS_INIT_MODE
#define PSCI_NUM_CALLS			U(26)
#else
#define PSCI_NUM_CALLS			U(25)
#endif
#endif

/* The macros below are used to identify PSCI calls from the SMC function ID */
#define PSCI_FID_MASK			U(0xffe0)
#define PSCI_FID_VALUE			U(0)
#define is_psci_fid(_fid) \
	(((_fid) & PSCI_FID_MASK) == PSCI_FID_VALUE)

/*******************************************************************************
 * PSCI Migrate and friends
 ******************************************************************************/
#define PSCI_TOS_UP_MIG_CAP	0
#define PSCI_TOS_NOT_UP_MIG_CAP	1
#define PSCI_TOS_NOT_PRESENT_MP	2

/*******************************************************************************
 * PSCI CPU_SUSPEND 'power_state' parameter specific defines
 ******************************************************************************/
#define PSTATE_ID_SHIFT		U(0)

#if PSCI_EXTENDED_STATE_ID
#define PSTATE_VALID_MASK	U(0xB0000000)
#define PSTATE_TYPE_SHIFT	U(30)
#define PSTATE_ID_MASK		U(0xfffffff)
#else
#define PSTATE_VALID_MASK	U(0xFCFE0000)
#define PSTATE_TYPE_SHIFT	U(16)
#define PSTATE_PWR_LVL_SHIFT	U(24)
#define PSTATE_ID_MASK		U(0xffff)
#define PSTATE_PWR_LVL_MASK	U(0x3)

#define psci_get_pstate_pwrlvl(pstate)	(((pstate) >> PSTATE_PWR_LVL_SHIFT) & \
					PSTATE_PWR_LVL_MASK)
#define psci_make_powerstate(state_id, type, pwrlvl) \
			(((state_id) & PSTATE_ID_MASK) << PSTATE_ID_SHIFT) |\
			(((type) & PSTATE_TYPE_MASK) << PSTATE_TYPE_SHIFT) |\
			(((pwrlvl) & PSTATE_PWR_LVL_MASK) << PSTATE_PWR_LVL_SHIFT)
#endif /* __PSCI_EXTENDED_STATE_ID__ */

#define PSTATE_TYPE_STANDBY	U(0x0)
#define PSTATE_TYPE_POWERDOWN	U(0x1)
#define PSTATE_TYPE_MASK	U(0x1)

/*******************************************************************************
 * PSCI CPU_FEATURES feature flag specific defines
 ******************************************************************************/
/* Features flags for CPU SUSPEND power state parameter format. Bits [1:1] */
#define FF_PSTATE_SHIFT		U(1)
#define FF_PSTATE_ORIG		U(0)
#define FF_PSTATE_EXTENDED	U(1)
#if PSCI_EXTENDED_STATE_ID
#define FF_PSTATE		FF_PSTATE_EXTENDED
#else
#define FF_PSTATE		FF_PSTATE_ORIG
#endif

/* Features flags for CPU SUSPEND OS Initiated mode support. Bits [0:0] */
#define FF_MODE_SUPPORT_SHIFT		U(0)
#if PSCI_OS_INIT_MODE
#define FF_SUPPORTS_OS_INIT_MODE	U(1)
#else
#define FF_SUPPORTS_OS_INIT_MODE	U(0)
#endif

/*******************************************************************************
 * PSCI version
 ******************************************************************************/
#define PSCI_MAJOR_VER		(U(1) << 16)
#define PSCI_MINOR_VER		U(0x1)

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
#define PSCI_E_INVALID_ADDRESS	-9

#define PSCI_INVALID_MPIDR	~((u_register_t)0)

/*
 * SYSTEM_RESET2 macros
 */
#define PSCI_RESET2_TYPE_VENDOR_SHIFT	U(31)
#define PSCI_RESET2_TYPE_VENDOR		(U(1) << PSCI_RESET2_TYPE_VENDOR_SHIFT)
#define PSCI_RESET2_TYPE_ARCH		(U(0) << PSCI_RESET2_TYPE_VENDOR_SHIFT)
#define PSCI_RESET2_SYSTEM_WARM_RESET	(PSCI_RESET2_TYPE_ARCH | U(0))

#ifndef __ASSEMBLER__

#include <stdint.h>

/* Function to help build the psci capabilities bitfield */

static inline unsigned int define_psci_cap(unsigned int x)
{
	return U(1) << (x & U(0x1f));
}


/* Power state helper functions */

static inline unsigned int psci_get_pstate_id(unsigned int power_state)
{
	return ((power_state) >> PSTATE_ID_SHIFT) & PSTATE_ID_MASK;
}

static inline unsigned int psci_get_pstate_type(unsigned int power_state)
{
	return ((power_state) >> PSTATE_TYPE_SHIFT) & PSTATE_TYPE_MASK;
}

static inline unsigned int psci_check_power_state(unsigned int power_state)
{
	return ((power_state) & PSTATE_VALID_MASK);
}

/*
 * These are the states reported by the PSCI_AFFINITY_INFO API for the specified
 * CPU. The definitions of these states can be found in Section 5.7.1 in the
 * PSCI specification (ARM DEN 0022C).
 */
typedef enum {
	AFF_STATE_ON = U(0),
	AFF_STATE_OFF = U(1),
	AFF_STATE_ON_PENDING = U(2)
} aff_info_state_t;

/*
 * These are the power states reported by PSCI_NODE_HW_STATE API for the
 * specified CPU. The definitions of these states can be found in Section 5.15.3
 * of PSCI specification (ARM DEN 0022C).
 */
#define HW_ON		0
#define HW_OFF		1
#define HW_STANDBY	2

/*
 * Macro to represent invalid affinity level within PSCI.
 */
#define PSCI_INVALID_PWR_LVL	(PLAT_MAX_PWR_LVL + U(1))

/*
 * Type for representing the local power state at a particular level.
 */
typedef uint8_t plat_local_state_t;

/* The local state macro used to represent RUN state. */
#define PSCI_LOCAL_STATE_RUN	U(0)

/*
 * Function to test whether the plat_local_state is RUN state
 */
static inline int is_local_state_run(unsigned int plat_local_state)
{
	return (plat_local_state == PSCI_LOCAL_STATE_RUN) ? 1 : 0;
}

/*
 * Function to test whether the plat_local_state is RETENTION state
 */
static inline int is_local_state_retn(unsigned int plat_local_state)
{
	return ((plat_local_state > PSCI_LOCAL_STATE_RUN) &&
		(plat_local_state <= PLAT_MAX_RET_STATE)) ? 1 : 0;
}

/*
 * Function to test whether the plat_local_state is OFF state
 */
static inline int is_local_state_off(unsigned int plat_local_state)
{
	return ((plat_local_state > PLAT_MAX_RET_STATE) &&
		(plat_local_state <= PLAT_MAX_OFF_STATE)) ? 1 : 0;
}

/*****************************************************************************
 * This data structure defines the representation of the power state parameter
 * for its exchange between the generic PSCI code and the platform port. For
 * example, it is used by the platform port to specify the requested power
 * states during a power management operation. It is used by the generic code to
 * inform the platform about the target power states that each level should
 * enter.
 ****************************************************************************/
typedef struct psci_power_state {
	/*
	 * The pwr_domain_state[] stores the local power state at each level
	 * for the CPU.
	 */
	plat_local_state_t pwr_domain_state[PLAT_MAX_PWR_LVL + U(1)];
#if PSCI_OS_INIT_MODE
	/*
	 * The highest power level at which the current CPU is the last running
	 * CPU.
	 */
	unsigned int last_at_pwrlvl;
#endif
} psci_power_state_t;

/*******************************************************************************
 * Structure used to store per-cpu information relevant to the PSCI service.
 * It is populated in the per-cpu data array. In return we get a guarantee that
 * this information will not reside on a cache line shared with another cpu.
 ******************************************************************************/
typedef struct psci_cpu_data {
	/* State as seen by PSCI Affinity Info API */
	aff_info_state_t aff_info_state;

	/*
	 * Highest power level which takes part in a power management
	 * operation. May be lower while the core is in suspend state.
	 */
	unsigned int target_pwrlvl;

	/* The local power state of this CPU */
	plat_local_state_t local_state;
} psci_cpu_data_t;

/*******************************************************************************
 * Structure populated by platform specific code to export routines which
 * perform common low level power management functions
 ******************************************************************************/
typedef struct plat_psci_ops {
	void (*cpu_standby)(plat_local_state_t cpu_state);
	int (*pwr_domain_on)(u_register_t mpidr);
	void (*pwr_domain_off)(const psci_power_state_t *target_state);
	int (*pwr_domain_off_early)(const psci_power_state_t *target_state);
#if PSCI_OS_INIT_MODE
	int (*pwr_domain_validate_suspend)(
				const psci_power_state_t *target_state);
#endif
	void (*pwr_domain_suspend_pwrdown_early)(
				const psci_power_state_t *target_state);
	void (*pwr_domain_suspend)(const psci_power_state_t *target_state);
	void (*pwr_domain_on_finish)(const psci_power_state_t *target_state);
	void (*pwr_domain_on_finish_late)(
				const psci_power_state_t *target_state);
	void (*pwr_domain_suspend_finish)(
				const psci_power_state_t *target_state);
	void __dead2 (*pwr_domain_pwr_down_wfi)(
				const psci_power_state_t *target_state);
	void __dead2 (*system_off)(void);
	void __dead2 (*system_reset)(void);
	int (*validate_power_state)(unsigned int power_state,
				    psci_power_state_t *req_state);
	int (*validate_ns_entrypoint)(uintptr_t ns_entrypoint);
	void (*get_sys_suspend_power_state)(
				    psci_power_state_t *req_state);
	int (*get_pwr_lvl_state_idx)(plat_local_state_t pwr_domain_state,
				    int pwrlvl);
	int (*translate_power_state_by_mpidr)(u_register_t mpidr,
				    unsigned int power_state,
				    psci_power_state_t *output_state);
	int (*get_node_hw_state)(u_register_t mpidr, unsigned int power_level);
	int (*mem_protect_chk)(uintptr_t base, u_register_t length);
	int (*read_mem_protect)(int *val);
	int (*write_mem_protect)(int val);
	int (*system_reset2)(int is_vendor,
				int reset_type, u_register_t cookie);
} plat_psci_ops_t;

/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
unsigned int psci_version(void);
int psci_cpu_on(u_register_t target_cpu,
		uintptr_t entrypoint,
		u_register_t context_id);
int psci_cpu_suspend(unsigned int power_state,
		     uintptr_t entrypoint,
		     u_register_t context_id);
int psci_system_suspend(uintptr_t entrypoint, u_register_t context_id);
int psci_cpu_off(void);
int psci_affinity_info(u_register_t target_affinity,
		       unsigned int lowest_affinity_level);
int psci_migrate(u_register_t target_cpu);
int psci_migrate_info_type(void);
u_register_t psci_migrate_info_up_cpu(void);
int psci_node_hw_state(u_register_t target_cpu,
		       unsigned int power_level);
int psci_features(unsigned int psci_fid);
#if PSCI_OS_INIT_MODE
int psci_set_suspend_mode(unsigned int mode);
#endif
void __dead2 psci_power_down_wfi(void);
void psci_arch_setup(void);

#endif /*__ASSEMBLER__*/

#endif /* PSCI_H */
