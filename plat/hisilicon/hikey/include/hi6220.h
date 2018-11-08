/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HI6220_H
#define HI6220_H

#include <hi6220_regs_acpu.h>
#include <hi6220_regs_ao.h>
#include <hi6220_regs_peri.h>
#include <hi6220_regs_pin.h>
#include <hi6220_regs_pmctrl.h>

/*******************************************************************************
 * Implementation defined ACTLR_EL2 bit definitions
 ******************************************************************************/
#define ACTLR_EL2_L2ACTLR_BIT		(1 << 6)
#define ACTLR_EL2_L2ECTLR_BIT		(1 << 5)
#define ACTLR_EL2_L2CTLR_BIT		(1 << 4)
#define ACTLR_EL2_CPUECTLR_BIT		(1 << 1)
#define ACTLR_EL2_CPUACTLR_BIT		(1 << 0)

/*******************************************************************************
 * Implementation defined ACTLR_EL3 bit definitions
 ******************************************************************************/
#define ACTLR_EL3_L2ACTLR_BIT		(1 << 6)
#define ACTLR_EL3_L2ECTLR_BIT		(1 << 5)
#define ACTLR_EL3_L2CTLR_BIT		(1 << 4)
#define ACTLR_EL3_CPUECTLR_BIT		(1 << 1)
#define ACTLR_EL3_CPUACTLR_BIT		(1 << 0)

/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define CCI400_BASE				0xF6E90000
#define CCI400_SL_IFACE3_CLUSTER_IX		3
#define CCI400_SL_IFACE4_CLUSTER_IX		4

#define DWMMC0_BASE				0xF723D000

#define DWUSB_BASE				0xF72C0000

#define EDMAC_BASE				0xf7370000
#define EDMAC_SEC_CTRL				(EDMAC_BASE + 0x694)
#define EDMAC_AXI_CONF(x)			(EDMAC_BASE + 0x820 + (x << 6))
#define EDMAC_SEC_CTRL_INTR_SEC			(1 << 1)
#define EDMAC_SEC_CTRL_GLOBAL_SEC		(1 << 0)
#define EDMAC_CHANNEL_NUMS			16

#define PMUSSI_BASE				0xF8000000

#define SP804_TIMER0_BASE			0xF8008000

#define GPIO0_BASE				0xF8011000
#define GPIO1_BASE				0xF8012000
#define GPIO2_BASE				0xF8013000
#define GPIO3_BASE				0xF8014000
#define GPIO4_BASE				0xF7020000
#define GPIO5_BASE				0xF7021000
#define GPIO6_BASE				0xF7022000
#define GPIO7_BASE				0xF7023000
#define GPIO8_BASE				0xF7024000
#define GPIO9_BASE				0xF7025000
#define GPIO10_BASE				0xF7026000
#define GPIO11_BASE				0xF7027000
#define GPIO12_BASE				0xF7028000
#define GPIO13_BASE				0xF7029000
#define GPIO14_BASE				0xF702A000
#define GPIO15_BASE				0xF702B000
#define GPIO16_BASE				0xF702C000
#define GPIO17_BASE				0xF702D000
#define GPIO18_BASE				0xF702E000
#define GPIO19_BASE				0xF702F000

#endif /* HI6220_H */
