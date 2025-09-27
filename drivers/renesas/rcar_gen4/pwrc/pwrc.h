/*
 * Copyright (c) 2015-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PWRC_H
#define PWRC_H

#ifndef __ASSEMBLER__
void rcar_pwrc_disable_interrupt_wakeup(u_register_t mpidr);
void rcar_pwrc_enable_interrupt_wakeup(u_register_t mpidr);
void rcar_pwrc_clusteroff(u_register_t mpidr);
void rcar_pwrc_cpuoff(u_register_t mpidr);
void rcar_pwrc_cpuon(u_register_t mpidr);
void rcar_pwrc_setup(void);
uint32_t rcar_pwrc_get_mpidr_cluster(u_register_t mpidr);
uint32_t rcar_pwrc_cpu_on_check(u_register_t mpidr);

void rcar_pwrc_code_copy_to_system_ram(void);
void rcar_pwrc_suspend_to_ram(void);

void plat_secondary_reset(void);

int32_t rcar_pwrc_cpu_migrate_info(u_register_t *resident_cpu);
bool rcar_pwrc_mpidr_is_boot_cpu(u_register_t mpidr);
#endif /*__ASSEMBLER__*/

extern uint32_t rcar_pwrc_switch_stack(uintptr_t jump, uintptr_t stack,
				       void *arg);

#endif /* PWRC_H */
