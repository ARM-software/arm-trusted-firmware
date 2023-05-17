/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include "socfpga_handoff.h"

#define AGX5_PWRMGR_BASE					0x10d14000

/* DSU */
#define AGX5_PWRMGR_DSU_FWENCTL					0x0
#define AGX5_PWRMGR_DSU_PGENCTL					0x4
#define AGX5_PWRMGR_DSU_PGSTAT					0x8
#define AGX5_PWRMGR_DSU_PWRCTLR					0xc
#define AGX5_PWRMGR_DSU_PWRSTAT0				0x10
#define AGX5_PWRMGR_DSU_PWRSTAT1				0x14

/* DSU Macros*/
#define AGX5_PWRMGR_DSU_FWEN(x)					((x) & 0xf)
#define AGX5_PWRMGR_DSU_PGEN(x)					((x) & 0xf)
#define AGX5_PWRMGR_DSU_PGEN_OUT(x)				((x) & 0xf)
#define AGX5_PWRMGR_DSU_SINGLE_PACCEPT(x)			((x) & 0x1)
#define AGX5_PWRMGR_DSU_SINGLE_PDENY(x)				(((x) & 0x1) << 1)
#define AGX5_PWRMGR_DSU_SINGLE_FSM_STATE(x)			(((x) & 0xff) << 8)
#define AGX5_PWRMGR_DSU_SINGLE_PCH_DONE(x)			(((x) & 0x1) << 31)
#define AGX5_PWRMGR_DSU_MULTI_PACTIVE_IN(x)			((x) & 0xff)
#define AGX5_PWRMGR_DSU_MULTI_PACCEPT(x)			(((x) & 0xff) << 8)
#define AGX5_PWRMGR_DSU_MULTI_PDENY(x)				(((x) & 0xff) << 16)
#define AGX5_PWRMGR_DSU_MULTI_PCH_DONE(x)			(((x) & 0x1) << 31)

/* CPU */
#define AGX5_PWRMGR_CPU_PWRCTLR0				0x18
#define AGX5_PWRMGR_CPU_PWRCTLR1				0x20
#define AGX5_PWRMGR_CPU_PWRCTLR2				0x28
#define AGX5_PWRMGR_CPU_PWRCTLR3				0x30
#define AGX5_PWRMGR_CPU_PWRSTAT0				0x1c
#define AGX5_PWRMGR_CPU_PWRSTAT1				0x24
#define AGX5_PWRMGR_CPU_PWRSTAT2				0x2c
#define AGX5_PWRMGR_CPU_PWRSTAT3				0x34

/* APS */
#define AGX5_PWRMGR_APS_FWENCTL					0x38
#define AGX5_PWRMGR_APS_PGENCTL					0x3C
#define AGX5_PWRMGR_APS_PGSTAT					0x40

/* PSS */
#define AGX5_PWRMGR_PSS_FWENCTL					0x44
#define AGX5_PWRMGR_PSS_PGENCTL					0x48
#define AGX5_PWRMGR_PSS_PGSTAT					0x4c

/* PSS Macros*/
#define AGX5_PWRMGR_PSS_FWEN(x)					((x) & 0xff)
#define AGX5_PWRMGR_PSS_PGEN(x)					((x) & 0xff)
#define AGX5_PWRMGR_PSS_PGEN_OUT(x)				((x) & 0xff)

/* MPU */
#define AGX5_PWRMGR_MPU_PCHCTLR					0x50
#define AGX5_PWRMGR_MPU_PCHSTAT					0x54
#define AGX5_PWRMGR_MPU_BOOTCONFIG				0x58
#define AGX5_PWRMGR_CPU_POWER_STATE_MASK			0x1E

/* MPU Macros*/
#define AGX5_PWRMGR_MPU_TRIGGER_PCH_DSU(x)			((x) & 0x1)
#define AGX5_PWRMGR_MPU_TRIGGER_PCH_CPU(x)			(((x) & 0xf) << 1)
#define AGX5_PWRMGR_MPU_STATUS_PCH_CPU(x)			(((x) & 0xf) << 1)

/* Shared Macros */
#define AGX5_PWRMGR(_reg)					(AGX5_PWRMGR_BASE + \
								(AGX5_PWRMGR_##_reg))

/* POWER MANAGER ERROR CODE */
#define AGX5_PWRMGR_HANDOFF_PERIPHERAL				-1
#define AGX5_PWRMGR_PSS_STAT_BUSY_E_BUSY			0x0
#define AGX5_PWRMGR_PSS_STAT_BUSY(x)				(((x) & 0x000000FF) >> 0)

int pss_sram_power_off(handoff *hoff_ptr);
int wait_verify_fsm(uint16_t timeout, uint32_t peripheral_handoff);

#endif
