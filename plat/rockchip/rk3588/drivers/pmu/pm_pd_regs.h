/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PM_PD_REGS_H
#define PM_PD_REGS_H

#include <stdint.h>

void qos_save(void);
void qos_restore(void);
void pd_crypto_save(void);
void pd_crypto_restore(void);
void pd_dsu_core_save(void);
void pd_dsu_core_restore(void);
void pd_php_save(void);
void pd_php_restore(void);

void pm_reg_rgns_init(void);

#endif
