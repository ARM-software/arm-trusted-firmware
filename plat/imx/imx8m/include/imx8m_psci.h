/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX8M_PSCI_H
#define IMX8M_PSCI_H

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

int imx_pwr_domain_on(u_register_t mpidr);
void imx_pwr_domain_on_finish(const psci_power_state_t *target_state);
void imx_pwr_domain_off(const psci_power_state_t *target_state);
int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint);
void imx_cpu_standby(plat_local_state_t cpu_state);
void imx_domain_suspend(const psci_power_state_t *target_state);
void imx_domain_suspend_finish(const psci_power_state_t *target_state);
void __dead2 imx_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state);
int imx_system_reset2(int is_vendor, int reset_type, u_register_t cookie);

#endif /* IMX8M_PSCI_H */
