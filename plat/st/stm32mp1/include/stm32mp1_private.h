/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_PRIVATE_H
#define STM32MP1_PRIVATE_H

#include <stdint.h>

void configure_mmu(void);

void stm32mp1_arch_security_setup(void);
void stm32mp1_security_setup(void);

void stm32mp_syscfg_init(void);
void stm32mp_syscfg_enable_io_compensation_start(void);
void stm32mp_syscfg_enable_io_compensation_finish(void);
void stm32mp_syscfg_disable_io_compensation(void);
uint32_t stm32mp_syscfg_get_chip_version(void);
uint32_t stm32mp_syscfg_get_chip_dev_id(void);
#if STM32MP13
void stm32mp_syscfg_boot_mode_enable(void);
void stm32mp_syscfg_boot_mode_disable(void);
#endif
#if STM32MP15
static inline void stm32mp_syscfg_boot_mode_enable(void){}
static inline void stm32mp_syscfg_boot_mode_disable(void){}
#endif

void stm32mp1_deconfigure_uart_pins(void);

void stm32mp1_init_scmi_server(void);

/* Wrappers for OTP / BSEC functions */
static inline uint32_t stm32_otp_read(uint32_t *val, uint32_t otp)
{
	return bsec_read_otp(val, otp);
}

static inline uint32_t stm32_otp_shadow_read(uint32_t *val, uint32_t otp)
{
	return bsec_shadow_read_otp(val, otp);
}

static inline uint32_t stm32_otp_write(uint32_t val, uint32_t otp)
{
	return bsec_write_otp(val, otp);
}

static inline uint32_t stm32_otp_set_sr_lock(uint32_t otp)
{
	return bsec_set_sr_lock(otp);
}

static inline uint32_t stm32_otp_read_sw_lock(uint32_t otp, bool *value)
{
	return bsec_read_sw_lock(otp, value);
}

#endif /* STM32MP1_PRIVATE_H */
