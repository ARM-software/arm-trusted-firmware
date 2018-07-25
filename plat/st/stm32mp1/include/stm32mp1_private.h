/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __STM32MP1_PRIVATE_H__
#define __STM32MP1_PRIVATE_H__

void stm32mp1_io_setup(void);
void configure_mmu(void);

void stm32mp1_arch_security_setup(void);
void stm32mp1_security_setup(void);

void stm32mp1_save_boot_ctx_address(uintptr_t address);
uintptr_t stm32mp1_get_boot_ctx_address(void);

void stm32mp1_gic_pcpu_init(void);
void stm32mp1_gic_init(void);

#endif /* __STM32MP1_PRIVATE_H__ */
