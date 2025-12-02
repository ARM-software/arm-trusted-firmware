/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PWRC_H
#define PWRC_H

#ifndef __ASSEMBLER__
void rcar_pwrc_disable_interrupt_wakeup(u_register_t mpidr);
void rcar_pwrc_enable_interrupt_wakeup(u_register_t mpidr);
uint32_t rcar_pwrc_cpu_on_check(u_register_t mpidr);

void rcar_pwrc_code_copy_to_system_ram(void);
void rcar_pwrc_suspend_to_ram(void);

void plat_secondary_reset(void);
#endif

#endif /* PWRC_H */
