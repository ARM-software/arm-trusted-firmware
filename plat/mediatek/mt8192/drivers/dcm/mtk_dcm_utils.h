/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_DCM_UTILS_H
#define MTK_DCM_UTILS_H

#include <stdbool.h>

#include <mtk_dcm.h>
#include <platform_def.h>

/* Base */
#define MP_CPUSYS_TOP_BASE	(MCUCFG_BASE + 0x8000)
#define CPCCFG_REG_BASE		(MCUCFG_BASE + 0xA800)

/* Register Definition */
#define CPU_PLLDIV_CFG0		(MP_CPUSYS_TOP_BASE + 0x22a0)
#define CPU_PLLDIV_CFG1		(MP_CPUSYS_TOP_BASE + 0x22a4)
#define CPU_PLLDIV_CFG2		(MP_CPUSYS_TOP_BASE + 0x22a8)
#define CPU_PLLDIV_CFG3		(MP_CPUSYS_TOP_BASE + 0x22ac)
#define CPU_PLLDIV_CFG4		(MP_CPUSYS_TOP_BASE + 0x22b0)
#define BUS_PLLDIV_CFG		(MP_CPUSYS_TOP_BASE + 0x22e0)
#define MCSI_DCM0		(MP_CPUSYS_TOP_BASE + 0x2440)
#define MP_ADB_DCM_CFG0		(MP_CPUSYS_TOP_BASE + 0x2500)
#define MP_ADB_DCM_CFG4		(MP_CPUSYS_TOP_BASE + 0x2510)
#define MP_MISC_DCM_CFG0	(MP_CPUSYS_TOP_BASE + 0x2518)
#define MCUSYS_DCM_CFG0		(MP_CPUSYS_TOP_BASE + 0x25c0)
#define EMI_WFIFO		(CPCCFG_REG_BASE + 0x100)
#define MP0_DCM_CFG0		(MP_CPUSYS_TOP_BASE + 0x4880)
#define MP0_DCM_CFG7		(MP_CPUSYS_TOP_BASE + 0x489c)

/* MP_CPUSYS_TOP */
bool dcm_mp_cpusys_top_adb_dcm_is_on(void);
void dcm_mp_cpusys_top_adb_dcm(bool on);
bool dcm_mp_cpusys_top_apb_dcm_is_on(void);
void dcm_mp_cpusys_top_apb_dcm(bool on);
bool dcm_mp_cpusys_top_bus_pll_div_dcm_is_on(void);
void dcm_mp_cpusys_top_bus_pll_div_dcm(bool on);
bool dcm_mp_cpusys_top_core_stall_dcm_is_on(void);
void dcm_mp_cpusys_top_core_stall_dcm(bool on);
bool dcm_mp_cpusys_top_cpubiu_dcm_is_on(void);
void dcm_mp_cpusys_top_cpubiu_dcm(bool on);
bool dcm_mp_cpusys_top_cpu_pll_div_0_dcm_is_on(void);
void dcm_mp_cpusys_top_cpu_pll_div_0_dcm(bool on);
bool dcm_mp_cpusys_top_cpu_pll_div_1_dcm_is_on(void);
void dcm_mp_cpusys_top_cpu_pll_div_1_dcm(bool on);
bool dcm_mp_cpusys_top_cpu_pll_div_2_dcm_is_on(void);
void dcm_mp_cpusys_top_cpu_pll_div_2_dcm(bool on);
bool dcm_mp_cpusys_top_cpu_pll_div_3_dcm_is_on(void);
void dcm_mp_cpusys_top_cpu_pll_div_3_dcm(bool on);
bool dcm_mp_cpusys_top_cpu_pll_div_4_dcm_is_on(void);
void dcm_mp_cpusys_top_cpu_pll_div_4_dcm(bool on);
bool dcm_mp_cpusys_top_fcm_stall_dcm_is_on(void);
void dcm_mp_cpusys_top_fcm_stall_dcm(bool on);
bool dcm_mp_cpusys_top_last_cor_idle_dcm_is_on(void);
void dcm_mp_cpusys_top_last_cor_idle_dcm(bool on);
bool dcm_mp_cpusys_top_misc_dcm_is_on(void);
void dcm_mp_cpusys_top_misc_dcm(bool on);
bool dcm_mp_cpusys_top_mp0_qdcm_is_on(void);
void dcm_mp_cpusys_top_mp0_qdcm(bool on);
/* CPCCFG_REG */
bool dcm_cpccfg_reg_emi_wfifo_is_on(void);
void dcm_cpccfg_reg_emi_wfifo(bool on);

#endif
