/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2026, Rockchip Electronics Co., Ltd.
 */

#ifndef PM_PD_REGS_H
#define PM_PD_REGS_H

#include <stdint.h>

void pd_aiisp_save(void);
void pd_aiisp_restore(void);
void pd_vdo_save(void);
void pd_vdo_restore(void);
void vd_npu_save(void);
void vd_npu_restore(void);
void vd_core_save(void);
void vd_core_restore(void);

void pm_reg_rgns_init(void);

#endif
