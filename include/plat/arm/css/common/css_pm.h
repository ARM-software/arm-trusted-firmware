/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CSS_PM_H
#define CSS_PM_H

#include <cdefs.h>
#include <stdint.h>

#include <lib/psci/psci.h>

/* Macros to read the CSS power domain state */
#define CSS_CORE_PWR_STATE(state)	(state)->pwr_domain_state[ARM_PWR_LVL0]
#define CSS_CLUSTER_PWR_STATE(state)	(state)->pwr_domain_state[ARM_PWR_LVL1]

static inline unsigned int css_system_pwr_state(const psci_power_state_t *state)
{
#if (PLAT_MAX_PWR_LVL == CSS_SYSTEM_PWR_DMN_LVL)
	return state->pwr_domain_state[CSS_SYSTEM_PWR_DMN_LVL];
#else
	return 0;
#endif
}

int css_pwr_domain_on(u_register_t mpidr);
void css_pwr_domain_on_finish(const psci_power_state_t *target_state);
void css_pwr_domain_on_finish_late(const psci_power_state_t *target_state);
void css_pwr_domain_off(const psci_power_state_t *target_state);
void css_pwr_domain_suspend(const psci_power_state_t *target_state);
void css_pwr_domain_suspend_finish(
				const psci_power_state_t *target_state);
void __dead2 css_system_off(void);
void __dead2 css_system_reset(void);
void css_cpu_standby(plat_local_state_t cpu_state);
void css_get_sys_suspend_power_state(psci_power_state_t *req_state);
int css_node_hw_state(u_register_t mpidr, unsigned int power_level);

/*
 * This mapping array has to be exported by the platform. Each element at
 * a given index maps that core to an SCMI power domain.
 */
extern const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[];

#define SCMI_DOMAIN_ID_MASK		U(0xFFFF)
#define SCMI_CHANNEL_ID_MASK		U(0xFFFF)
#define SCMI_CHANNEL_ID_SHIFT		U(16)

#define SET_SCMI_CHANNEL_ID(n)		(((n) & SCMI_CHANNEL_ID_MASK) << \
					 SCMI_CHANNEL_ID_SHIFT)
#define SET_SCMI_DOMAIN_ID(n)		((n) & SCMI_DOMAIN_ID_MASK)
#define GET_SCMI_CHANNEL_ID(n)		(((n) >> SCMI_CHANNEL_ID_SHIFT) & \
					 SCMI_CHANNEL_ID_MASK)
#define GET_SCMI_DOMAIN_ID(n)		((n) & SCMI_DOMAIN_ID_MASK)

#endif /* CSS_PM_H */
