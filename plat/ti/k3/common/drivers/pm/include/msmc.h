/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MSMC_H
#define MSMC_H

#include <lib/bitops.h>

#define MSMC_PID		0x000
#define MSMC_SMCC		0x004
#define MSMC_SMCERRAR		0x008
#define MSMC_SMCERRXR		0x00c
#define MSMC_SMEDCC		0x010
#define MSMC_SMCEA		0x014
#define MSMC_SMSECC		0x018
#define MSMC_SMPFAR		0x01c
#define MSMC_SMPFXR		0x020
#define MSMC_SMPFR		0x024
#define MSMC_SMPFCR		0x028
#define MSMC_SMIDCFG		0x02c
#define MSMC_SBNDC0		0x030
#define MSMC_SBNDC1		0x034
#define MSMC_SBNDC2		0x038
#define MSMC_SBNDC3		0x03c
#define MSMC_SBNDC4		0x040
#define MSMC_SBNDC5		0x044
#define MSMC_SBNDC6		0x048
#define MSMC_SBNDC7		0x04c
#define MSMC_SBNDM		0x050
#define MSMC_SBNDE		0x054
#define MSMC_SMEDCTST		0x058
#define MSMC_CFGLCK		0x05c
#define MSMC_CFGULCK		0x060
#define MSMC_CFGLCKSTAT		0x064
#define MSMC_SMS_MPAX_LCK	0x068
#define MSMC_SMS_MPAX_ULCK	0x06c
#define MSMC_SMS_MPAX_LCKSTAT	0x070
#define MSMC_SES_MPAX_LCK	0x074
#define MSMC_SES_MPAX_ULCK	0x078
#define MSMC_SES_MPAX_LCKSTAT	0x07c
#define MSMC_SMESTAT		0x080
#define MSMC_SMIRSTAT		0x084
#define MSMC_SMIRWS		0x084
#define MSMC_SMIRC		0x088
#define MSMC_SMIESTAT		0x08c
#define MSMC_SMIEWS		0x08c
#define MSMC_SMIEC		0x090
#define MSMC_SMATH		0x0a0
#define MSMC_SMAH		0x0a4
#define MSMC_SMARM		0x0a8
#define MSMC_SMAWM		0x0ac
#define MSMC_SMAV		0x0b0
#define MSMC_SMAMP		0x0b4
#define MSMC_SMAEP		0x0b8
#define MSMC_SMAC		0x0bc
#define MSMC_SMCFG		0x0c0
#define MSMC_SMNCERRAR		0x0c4
#define MSMC_SMNCERRXR		0x0c8
#define MSMC_SMNCEA		0x0cc
#define MSMC_SBNDC8		0x0d0
#define MSMC_SBNDC9		0x0d4
#define MSMC_SBNDC10		0x0d8
#define MSMC_SBNDC11		0x0dc
#define MSMC_SMSECIL		0x0e0
#define MSMC_SMSECIH		0x0e4
#define MSMC_SMSECEL		0x0e8
#define MSMC_SMSECEH		0x0ec
#define MSMC_SGSECIL		0x0f0
#define MSMC_SGSECIH		0x0f4
#define MSMC_SGSECEL		0x0f8
#define MSMC_SGSECEH		0x0fc
#define MSMC_COHSTAT		0x100
#define MSMC_COHCTRL		0x104
#define MSMC_SMS_MPAXL(PID, N)	(0x200 + ((PID) * 8 + (N)) * 8)
#define MSMC_SMS_MPAXH(PID, N)	(0x204 + ((PID) * 8 + (N)) * 8)
#define MSMC_SES_MPAXL(PID, N)	(0x600 + ((PID) * 8 + (N)) * 8)
#define MSMC_SES_MPAXH(PID, N)	(0x604 + ((PID) * 8 + (N)) * 8)

#define MSMC_SEGSZ_DIS		0x00
#define MSMC_SEGSZ_4KB		0x0b
#define MSMC_SEGSZ_8KB		0x0c
#define MSMC_SEGSZ_16KB		0x0d
#define MSMC_SEGSZ_32KB		0x0e
#define MSMC_SEGSZ_64KB		0x0f
#define MSMC_SEGSZ_128KB	0x10
#define MSMC_SEGSZ_256KB	0x11
#define MSMC_SEGSZ_512KB	0x12
#define MSMC_SEGSZ_1MB		0x13
#define MSMC_SEGSZ_2MB		0x14
#define MSMC_SEGSZ_4MB		0x15
#define MSMC_SEGSZ_8MB		0x16
#define MSMC_SEGSZ_16MB		0x17
#define MSMC_SEGSZ_32MB		0x18
#define MSMC_SEGSZ_64MB		0x19
#define MSMC_SEGSZ_128MB	0x1a
#define MSMC_SEGSZ_256MB	0x1b
#define MSMC_SEGSZ_512MB	0x1c
#define MSMC_SEGSZ_1GB		0x1d
#define MSMC_SEGSZ_2GB		0x1e
#define MSMC_SEGSZ_4GB		0x1f

#define MSMC_SES_UX		BIT(0)
#define MSMC_SES_UW		BIT(1)
#define MSMC_SES_UR		BIT(2)
#define MSMC_SES_SX		BIT(3)
#define MSMC_SES_SW		BIT(4)
#define MSMC_SES_SR		BIT(5)
#define MSMC_SES_EMU		BIT(6)
#define MSMC_SES_NS		BIT(7)
#define MSMC_SES_ALL		0xff

#endif
