/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CSS_SCP_H__
#define __CSS_SCP_H__

#include <types.h>
#include "../scpi/css_scpi.h"

/* Forward declarations */
struct psci_power_state;

/* API for power management by SCP */
void css_scp_suspend(const struct psci_power_state *target_state);
void css_scp_off(const struct psci_power_state *target_state);
void css_scp_on(u_register_t mpidr);
int css_scp_get_power_state(u_register_t mpidr, unsigned int power_level);
void __dead2 css_scp_sys_shutdown(void);
void __dead2 css_scp_sys_reboot(void);

/* API for SCP Boot Image transfer. Return 0 on success, -1 on error */
int css_scp_boot_image_xfer(void *image, unsigned int image_size);

/*
 * API to wait for SCP to signal till it's ready after booting the transferred
 * image.
 */
static inline int css_scp_boot_ready(void)
{
	VERBOSE("Waiting for SCP to signal it is ready to go on\n");
	return scpi_wait_ready();
}

#endif	/* __CSS_SCP_H__ */
