/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX9_PSCI_H
#define IMX9_PSCI_H

#include <lib/psci/psci.h>

#include <platform_def.h>

#define CORE_PWR_STATE(state)		((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state)	((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state)		((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

extern uintptr_t secure_entrypoint;
extern uint32_t mask_all[IMR_NUM];

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint);
void imx_set_cpu_boot_entry(uint32_t core_id, uint64_t boot_entry, uint32_t flag);
int imx_pwr_domain_on(u_register_t mpidr);
void imx_pwr_domain_on_finish(const psci_power_state_t *target_state);
void imx_pwr_domain_off(const psci_power_state_t *target_state);
void imx_pwr_domain_suspend(const psci_power_state_t *target_state);
void imx_pwr_domain_suspend_finish(const psci_power_state_t *target_state);
void imx_pwr_domain_pwr_down(const psci_power_state_t *target_state);

#endif /* IMX9_PSCI_H */
