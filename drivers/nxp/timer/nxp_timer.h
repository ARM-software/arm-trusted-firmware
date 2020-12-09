/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#
#ifndef NXP_TIMER_H
#define NXP_TIMER_H

 /* System Counter Offset and Bit Mask */
#define SYS_COUNTER_CNTCR_OFFSET	0x0
#define SYS_COUNTER_CNTCR_EN		0x00000001
#define CNTCR_EN_MASK			0x1

#ifndef __ASSEMBLER__
uint64_t get_timer_val(uint64_t start);

#ifdef IMAGE_BL31
void ls_configure_sys_timer(uintptr_t ls_sys_timctl_base,
			    uint8_t ls_config_cntacr,
			    uint8_t plat_ls_ns_timer_frame_id);
void enable_init_timer(void);
#endif

/*
 * Initialise the nxp on-chip free rolling usec counter as the delay
 * timer.
 */
void delay_timer_init(uintptr_t nxp_timer_addr);
void ls_bl31_timer_init(uintptr_t nxp_timer_addr);
#endif	/* __ASSEMBLER__ */

#endif /* NXP_TIMER_H */
