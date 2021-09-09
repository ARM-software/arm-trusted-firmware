/*
 * Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_PRIVATE_H
#define STM32MP1_PRIVATE_H

#include <stdint.h>

void configure_mmu(void);

void stm32mp1_arch_security_setup(void);
void stm32mp1_security_setup(void);

void stm32mp1_gic_pcpu_init(void);
void stm32mp1_gic_init(void);

void stm32mp1_syscfg_init(void);
void stm32mp1_syscfg_enable_io_compensation(void);
void stm32mp1_syscfg_disable_io_compensation(void);

#if STM32MP_USE_STM32IMAGE
uint32_t stm32mp_get_ddr_ns_size(void);
#endif /* STM32MP_USE_STM32IMAGE */

void stm32mp1_init_scmi_server(void);
#endif /* STM32MP1_PRIVATE_H */
