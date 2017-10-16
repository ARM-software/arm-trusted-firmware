/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CSS_SCP_H__
#define __CSS_SCP_H__

#include <cassert.h>
#include <platform_def.h>
#include <types.h>

/* Forward declarations */
struct psci_power_state;

/* API for power management by SCP */
int css_system_reset2(int is_vendor, int reset_type, u_register_t cookie);
void css_scp_suspend(const struct psci_power_state *target_state);
void css_scp_off(const struct psci_power_state *target_state);
void css_scp_on(u_register_t mpidr);
int css_scp_get_power_state(u_register_t mpidr, unsigned int power_level);
void __dead2 css_scp_sys_shutdown(void);
void __dead2 css_scp_sys_reboot(void);
void __dead2 css_scp_system_off(int state);

/* API for SCP Boot Image transfer. Return 0 on success, -1 on error */
int css_scp_boot_image_xfer(void *image, unsigned int image_size);

/*
 * API to wait for SCP to signal till it's ready after booting the transferred
 * image.
 */
int css_scp_boot_ready(void);

#if CSS_LOAD_SCP_IMAGES
/*
 * All CSS platforms load SCP_BL2/SCP_BL2U just below BL rw-data and above
 * BL2/BL2U (this is where BL31 usually resides except when ARM_BL31_IN_DRAM is
 * set. Ensure that SCP_BL2/SCP_BL2U do not overflow into BL1 rw-data nor
 * BL2/BL2U.
 */
CASSERT(SCP_BL2_LIMIT <= BL1_RW_BASE, assert_scp_bl2_limit_overwrite_bl1);
CASSERT(SCP_BL2U_LIMIT <= BL1_RW_BASE, assert_scp_bl2u_limit_overwrite_bl1);

CASSERT(SCP_BL2_BASE >= BL2_LIMIT, assert_scp_bl2_overwrite_bl2);
CASSERT(SCP_BL2U_BASE >= BL2U_LIMIT, assert_scp_bl2u_overwrite_bl2u);
#endif

#endif	/* __CSS_SCP_H__ */
