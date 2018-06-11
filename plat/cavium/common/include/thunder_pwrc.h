/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018 Facebook Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __THUNDER_PWRC_H__
#define __THUNDER_PWRC_H__

/* Thunder Power controller register offset etc */
#define PPOFFR_OFF		0x0
#define PPONR_OFF		0x4
#define PCOFFR_OFF		0x8
#define PWKUPR_OFF		0xc
#define PSYSR_OFF		0x10

#define PWKUPR_WEN		(1ull << 31)

#define PSYSR_AFF_L2		(1 << 31)
#define PSYSR_AFF_L1		(1 << 30)
#define PSYSR_AFF_L0		(1 << 29)
#define PSYSR_WEN		(1 << 28)
#define PSYSR_PC		(1 << 27)
#define PSYSR_PP		(1 << 26)

#define PSYSR_WK_SHIFT		24
#define PSYSR_WK_WIDTH		0x2
#define PSYSR_WK_MASK		((1 << PSYSR_WK_WIDTH) - 1)
#define PSYSR_WK(x)		(x >> PSYSR_WK_SHIFT) & PSYSR_WK_MASK

#define WKUP_COLD		0x0
#define WKUP_RESET		0x1
#define WKUP_PPONR		0x2
#define WKUP_GICREQ		0x3

#define PSYSR_INVALID		0xffffffff

#ifndef __ASSEMBLY__

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
void thunder_pwrc_write_pcoffr(unsigned long);
void thunder_pwrc_write_ppoffr(unsigned long);
void thunder_pwrc_write_pponr(unsigned long);
void thunder_pwrc_set_wen(unsigned long);
void thunder_pwrc_clr_wen(unsigned long);
unsigned int thunder_pwrc_read_psysr(unsigned long);
unsigned int thunder_pwrc_get_cpu_wkr(unsigned long);

#endif /*__ASSEMBLY__*/

#endif /* __THUNDER_PWRC_H__ */
