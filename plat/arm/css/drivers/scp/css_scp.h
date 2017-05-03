/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CSS_SCP_H__
#define __CSS_SCP_H__

#include <psci.h>

void css_scp_suspend(const psci_power_state_t *target_state);
void css_scp_off(const psci_power_state_t *target_state);
void css_scp_on(u_register_t mpidr);
int css_scp_get_power_state(u_register_t mpidr, unsigned int power_level);
void __dead2 css_scp_sys_shutdown(void);
void __dead2 css_scp_sys_reboot(void);

#endif	/* __CSS_SCP_H__ */
