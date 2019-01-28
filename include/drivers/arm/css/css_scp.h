/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CSS_SCP_H
#define CSS_SCP_H

#include <stdint.h>

#include <platform_def.h>

#include <lib/cassert.h>

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
 * All CSS platforms load SCP_BL2/SCP_BL2U just below BL2 (this is where BL31
 * usually resides except when ARM_BL31_IN_DRAM is
 * set). Ensure that SCP_BL2/SCP_BL2U do not overflow into tb_fw_config.
 */
CASSERT(SCP_BL2_LIMIT <= BL2_BASE, assert_scp_bl2_overwrite_bl2);
CASSERT(SCP_BL2U_LIMIT <= BL2_BASE, assert_scp_bl2u_overwrite_bl2);

CASSERT(SCP_BL2_BASE >= ARM_TB_FW_CONFIG_LIMIT, assert_scp_bl2_overflow);
CASSERT(SCP_BL2U_BASE >= ARM_TB_FW_CONFIG_LIMIT, assert_scp_bl2u_overflow);
#endif

#endif /* CSS_SCP_H */
