/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RCAR_PRIVATE_H
#define RCAR_PRIVATE_H

#include <common/bl_common.h>
#include <lib/bakery_lock.h>
#include <lib/el3_runtime/cpu_data.h>

#include <platform_def.h>

typedef volatile struct mailbox {
	uint64_t value __aligned(CACHE_WRITEBACK_GRANULE);
} mailbox_t;

/*
 * This structure represents the superset of information that is passed to
 * BL31 e.g. while passing control to it from BL2 which is bl31_params
 * and bl31_plat_params and its elements
 */
typedef struct bl2_to_bl31_params_mem {
	image_info_t bl32_image_info;
	image_info_t bl33_image_info;
	entry_point_info_t bl33_ep_info;
	entry_point_info_t bl32_ep_info;
} bl2_to_bl31_params_mem_t;

#define RCAR_INSTANTIATE_LOCK	DEFINE_BAKERY_LOCK(rcar_lock)
/*
 * Constants to specify how many bakery locks this platform implements. These
 * are used if the platform chooses not to use coherent memory for bakery lock
 * data structures.
 */
#define RCAR_MAX_BAKERIES	2

/*
 * Definition of structure which holds platform specific per-cpu data. Currently
 * it holds only the bakery lock information for each cpu. Constants to
 * specify how many bakeries this platform implements and bakery ids are
 * specified in rcar_def.h
 */
typedef struct rcar_cpu_data {
	bakery_info_t pcpu_bakery_info[RCAR_MAX_BAKERIES];
} rcar_cpu_data_t;

/*
 * Helper macros for bakery lock api when using the above rcar_cpu_data_t for
 * bakery lock data structures. It assumes that the bakery_info is at the
 * beginning of the platform specific per-cpu data.
 */
#define rcar_lock_init()	bakery_lock_init(&rcar_lock)
#define rcar_lock_get()		bakery_lock_get(&rcar_lock)
#define rcar_lock_release()	bakery_lock_release(&rcar_lock)

/* lock for SCMI */
#define RCAR_SCMI_INSTANTIATE_LOCK	spinlock_t rcar_scmi_lock
#define RCAR_SCMI_LOCK_GET_INSTANCE	(&rcar_scmi_lock)

/*
 * Function and variable prototypes
 */
void rcar_configure_mmu_el3(uintptr_t total_base,
			    size_t total_size,
			    uintptr_t ro_start,
			    uintptr_t ro_limit);

void plat_invalidate_icache(void);
void plat_cci_disable(void);
void plat_cci_enable(void);
void plat_cci_init(void);

void rcar_console_boot_init(void);
void rcar_console_runtime_init(void);

void __init plat_rcar_scmi_setup(void);
void rcar_scmi_cpuon(u_register_t mpidr);
void rcar_scmi_cpuoff(const struct psci_power_state *target_state);
void rcar_scmi_sys_shutdown(void);
void rcar_scmi_sys_reboot(void);
void rcar_scmi_sys_suspend(void);
const plat_psci_ops_t *plat_rcar_psci_override_pm_ops(plat_psci_ops_t *ops);

int32_t rcar_cluster_pos_by_mpidr(u_register_t mpidr);

/*
 * This mapping array has to be exported by the platform. Each element at
 * a given index maps that core to an SCMI power domain.
 */
extern const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[];

#define SCMI_DOMAIN_ID_MASK		0xFFFFU
#define SCMI_CHANNEL_ID_MASK		0xFFFFU
#define SCMI_CHANNEL_ID_SHIFT		16U

#define SET_SCMI_CHANNEL_ID(n)		\
	(((n) & SCMI_CHANNEL_ID_MASK) << SCMI_CHANNEL_ID_SHIFT)
#define SET_SCMI_DOMAIN_ID(n)		((n) & SCMI_DOMAIN_ID_MASK)
#define GET_SCMI_CHANNEL_ID(n)		\
	(((n) >> SCMI_CHANNEL_ID_SHIFT) & SCMI_CHANNEL_ID_MASK)
#define GET_SCMI_DOMAIN_ID(n)		((n) & SCMI_DOMAIN_ID_MASK)

static inline unsigned int css_system_pwr_state(const psci_power_state_t *state)
{
#if (PLAT_MAX_PWR_LVL == CSS_SYSTEM_PWR_DMN_LVL)
	return state->pwr_domain_state[CSS_SYSTEM_PWR_DMN_LVL];
#else
	return 0;
#endif
}

#endif /* RCAR_PRIVATE_H */
