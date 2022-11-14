/*
 * Copyright (c) 2015-2022, ARM Limited and Contributors. All rights reserved.
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
void stm32mp1_syscfg_enable_io_compensation_start(void);
void stm32mp1_syscfg_enable_io_compensation_finish(void);
void stm32mp1_syscfg_disable_io_compensation(void);
uint32_t stm32mp1_syscfg_get_chip_version(void);
uint32_t stm32mp1_syscfg_get_chip_dev_id(void);
#if STM32MP13
void stm32mp1_syscfg_boot_mode_enable(void);
void stm32mp1_syscfg_boot_mode_disable(void);
#endif
#if STM32MP15
static inline void stm32mp1_syscfg_boot_mode_enable(void){}
static inline void stm32mp1_syscfg_boot_mode_disable(void){}
#endif

void stm32mp1_deconfigure_uart_pins(void);

void stm32mp1_init_scmi_server(void);
#endif /* STM32MP1_PRIVATE_H */
