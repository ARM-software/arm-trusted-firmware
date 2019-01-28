/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FVP_PWRC_H
#define FVP_PWRC_H

/* FVP Power controller register offset etc */
#define PPOFFR_OFF		U(0x0)
#define PPONR_OFF		U(0x4)
#define PCOFFR_OFF		U(0x8)
#define PWKUPR_OFF		U(0xc)
#define PSYSR_OFF		U(0x10)

#define PWKUPR_WEN		BIT_32(31)

#define PSYSR_AFF_L2		BIT_32(31)
#define PSYSR_AFF_L1		BIT_32(30)
#define PSYSR_AFF_L0		BIT_32(29)
#define PSYSR_WEN		BIT_32(28)
#define PSYSR_PC		BIT_32(27)
#define PSYSR_PP		BIT_32(26)

#define PSYSR_WK_SHIFT		24
#define PSYSR_WK_WIDTH		0x2
#define PSYSR_WK_MASK		((1U << PSYSR_WK_WIDTH) - 1U)
#define PSYSR_WK(x)		((x) >> PSYSR_WK_SHIFT) & PSYSR_WK_MASK

#define WKUP_COLD		U(0x0)
#define WKUP_RESET		U(0x1)
#define WKUP_PPONR		U(0x2)
#define WKUP_GICREQ		U(0x3)

#define PSYSR_INVALID		U(0xffffffff)

#ifndef __ASSEMBLY__

#include <stdint.h>

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
void fvp_pwrc_write_pcoffr(u_register_t mpidr);
void fvp_pwrc_write_ppoffr(u_register_t mpidr);
void fvp_pwrc_write_pponr(u_register_t mpidr);
void fvp_pwrc_set_wen(u_register_t mpidr);
void fvp_pwrc_clr_wen(u_register_t mpidr);
unsigned int fvp_pwrc_read_psysr(u_register_t mpidr);
unsigned int fvp_pwrc_get_cpu_wkr(u_register_t mpidr);

#endif /*__ASSEMBLY__*/

#endif /* FVP_PWRC_H */
