/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FVP_PWRC_H__
#define __FVP_PWRC_H__

/* FVP Power controller register offset etc */
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
void fvp_pwrc_write_pcoffr(u_register_t);
void fvp_pwrc_write_ppoffr(u_register_t);
void fvp_pwrc_write_pponr(u_register_t);
void fvp_pwrc_set_wen(u_register_t);
void fvp_pwrc_clr_wen(u_register_t);
unsigned int fvp_pwrc_read_psysr(u_register_t);
unsigned int fvp_pwrc_get_cpu_wkr(u_register_t);

#endif /*__ASSEMBLY__*/

#endif /* __FVP_PWRC_H__ */
