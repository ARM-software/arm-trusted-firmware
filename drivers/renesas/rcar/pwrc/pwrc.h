/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PWRC_H
#define PWRC_H

#define PPOFFR_OFF		0x0
#define PPONR_OFF		0x4
#define PCOFFR_OFF		0x8
#define PWKUPR_OFF		0xc
#define PSYSR_OFF		0x10

#define PWKUPR_WEN		(1ull << 31)

#define PSYSR_AFF_L2		(1U << 31)
#define PSYSR_AFF_L1		(1 << 30)
#define PSYSR_AFF_L0		(1 << 29)
#define PSYSR_WEN		(1 << 28)
#define PSYSR_PC		(1 << 27)
#define PSYSR_PP		(1 << 26)

#define PSYSR_WK_SHIFT		(24)
#define PSYSR_WK_MASK		(0x3)
#define PSYSR_WK(x)		(((x) >> PSYSR_WK_SHIFT) & PSYSR_WK_MASK)

#define WKUP_COLD		0x0
#define WKUP_RESET		0x1
#define WKUP_PPONR		0x2
#define WKUP_GICREQ		0x3

#define	RCAR_INVALID		(0xffffffffU)
#define PSYSR_INVALID		0xffffffff

#define	RCAR_CLUSTER_A53A57	(0U)
#define	RCAR_CLUSTER_CA53	(1U)
#define	RCAR_CLUSTER_CA57	(2U)

#ifndef __ASSEMBLER__
void rcar_pwrc_disable_interrupt_wakeup(uint64_t mpidr);
void rcar_pwrc_enable_interrupt_wakeup(uint64_t mpidr);
void rcar_pwrc_clusteroff(uint64_t mpidr);
void rcar_pwrc_cpuoff(uint64_t mpidr);
void rcar_pwrc_cpuon(uint64_t mpidr);
int32_t rcar_pwrc_cpu_on_check(uint64_t mpidr);
void rcar_pwrc_setup(void);

uint32_t rcar_pwrc_get_cpu_wkr(uint64_t mpidr);
uint32_t rcar_pwrc_status(uint64_t mpidr);
uint32_t rcar_pwrc_get_cluster(void);
uint32_t rcar_pwrc_get_mpidr_cluster(uint64_t mpidr);
uint32_t rcar_pwrc_get_cpu_num(uint32_t cluster_type);
void rcar_pwrc_restore_timer_state(void);
void plat_secondary_reset(void);

void rcar_pwrc_code_copy_to_system_ram(void);

#if !PMIC_ROHM_BD9571
void rcar_pwrc_system_reset(void);
#endif

#if RCAR_SYSTEM_SUSPEND
void rcar_pwrc_go_suspend_to_ram(void);
void rcar_pwrc_set_suspend_to_ram(void);
void rcar_pwrc_init_suspend_to_ram(void);
void rcar_pwrc_suspend_to_ram(void);
#endif

extern uint32_t rcar_pwrc_switch_stack(uintptr_t jump, uintptr_t stack,
				       void *arg);
#endif

#endif /* PWRC_H */
