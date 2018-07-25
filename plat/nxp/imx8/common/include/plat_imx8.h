/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_IMX8_H__
#define __PLAT_IMX8_H__

#include <gicv3.h>
#include <psci.h>

unsigned int plat_calc_core_pos(uint64_t mpidr);
void imx_mailbox_init(uintptr_t base_addr);
void plat_gic_driver_init(void);
void plat_gic_init(void);
void plat_gic_cpuif_enable(void);
void plat_gic_cpuif_disable(void);
void plat_gic_pcpu_init(void);

void __dead2 imx_system_off(void);
void __dead2 imx_system_reset(void);
int imx_validate_power_state(unsigned int power_state,
			psci_power_state_t *req_state);
void imx_get_sys_suspend_power_state(psci_power_state_t *req_state);
#endif /*__PLAT_IMX8_H__ */
