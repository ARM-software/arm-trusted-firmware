/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
 */

#ifndef PM_PD_REGS_H
#define PM_PD_REGS_H

#include <stdint.h>

void qos_save(void);
void qos_restore(void);
void pd_usb2phy_save(void);
void pd_usb2phy_restore(void);
void pd_secure_save(void);
void pd_secure_restore(void);
void pd_bcore_save(void);
void pd_bcore_restore(void);
void pd_core_save(void);
void pd_core_restore(void);
void pd_php_save(void);
void pd_php_restore(void);
void pd_center_save(void);
void pd_center_restore(void);
void pd_bus_save(void);
void pd_bus_restore(void);
void pd_pmu1_save(void);
void pd_pmu1_restore_early(void);
void pd_pmu1_restore(void);
void pd_pmu0_save(void);
void pd_pmu0_restore(void);

void pm_reg_rgns_init(void);
void pm_regs_rgn_dump(void);

#endif
