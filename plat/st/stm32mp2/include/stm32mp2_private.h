/*
 * Copyright (c) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP2_PRIVATE_H
#define STM32MP2_PRIVATE_H

void configure_mmu(void);

uint32_t stm32mp_syscfg_get_chip_dev_id(void);

/* Get DDRDBG peripheral IO memory base address */
uintptr_t stm32_ddrdbg_get_base(void);

/* Wrappers for OTP / BSEC functions */
static inline uint32_t stm32_otp_probe(void)
{
	return bsec_probe();
}

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

static inline bool stm32_otp_is_closed_device(void)
{
	return bsec_mode_is_closed_device();
}

#endif /* STM32MP2_PRIVATE_H */
