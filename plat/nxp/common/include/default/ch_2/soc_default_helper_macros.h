/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef SOC_DEFAULT_HELPER_MACROS_H
#define SOC_DEFAULT_HELPER_MACROS_H

#ifdef NXP_OCRAM_TZPC_ADDR

/* 0x1: means 4 KB
 * 0x2: means 8 KB
 */
#define TZPC_BLOCK_SIZE			0x1000
#endif

/* DDR controller offsets and defines */
#ifdef NXP_DDR_ADDR

#define DDR_CFG_2_OFFSET                0x114
#define CFG_2_FORCE_REFRESH             0x80000000

#endif /* NXP_DDR_ADDR */

 /* Reset block register offsets */
#ifdef NXP_RESET_ADDR

/* Register Offset */
#define RST_RSTCR_OFFSET		0x0
#define RST_RSTRQMR1_OFFSET		0x10
#define RST_RSTRQSR1_OFFSET		0x18
#define BRR_OFFSET			0x60

/* helper macros */
#define RSTRQSR1_SWRR			0x800
#define RSTRQMR_RPTOE_MASK		(1 << 19)

#endif /* NXP_RESET_ADDR */

/* Secure-Register-File register offsets and bit masks */
#ifdef NXP_RST_ADDR
/* Register Offset */
#define CORE_HOLD_OFFSET		0x140
#define RSTCNTL_OFFSET			0x180

/* Helper macros */
#define SW_RST_REQ_INIT			0x1
#endif

#ifdef NXP_RCPM_ADDR
/* RCPM Register Offsets */
#define RCPM_PCPH20SETR_OFFSET		0x0D4
#define RCPM_PCPH20CLRR_OFFSET		0x0D8
#define RCPM_POWMGTCSR_OFFSET		0x130
#define RCPM_IPPDEXPCR0_OFFSET		0x140
#define RCPM_POWMGTCSR_LPM20_REQ	0x00100000
#endif /* NXP_RCPM_ADDR */

#define DCFG_SBEESR2_ADDR		0x20140534
#define DCFG_MBEESR2_ADDR		0x20140544
/* SBEESR and MBEESR bit mask */
#define OCRAM_EESR_MASK			0x00000060

#endif	/*	SOC_DEFAULT_HELPER_MACROS_H	*/
