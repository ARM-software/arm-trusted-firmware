/*
 * Copyright (c) 2024-2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP2_PRIVATE_H
#define STM32MP2_PRIVATE_H

void configure_mmu(void);

uint32_t stm32mp_syscfg_get_chip_dev_id(void);
size_t stm32mp_syscfg_get_mm_size(uint8_t bank);
int stm32mp_syscfg_dlyb_init(uint8_t bank, bool bypass_mode,
			     uint32_t period_ps);
int stm32mp_syscfg_dlyb_find_tap(uint8_t bank, int (*check_transfer)(void),
				 bool rx_only, uint8_t *window_len);
int stm32mp_syscfg_dlyb_set_cr(uint8_t bank, uint32_t dlyb_cr);
void stm32mp_syscfg_dlyb_get_cr(uint8_t bank, uint32_t *dlyb_cr);
void stm32mp_syscfg_dlyb_stop(uint8_t bank);

/* IO compensation identifiers */
enum syscfg_io_ids {
	SYSFG_VDDIO1_ID,
	SYSFG_VDDIO2_ID,
	SYSFG_VDDIO3_ID,
#if !STM32MP21
	SYSFG_VDDIO4_ID,
#endif /* !STM32MP21 */
	SYSFG_VDD_IO_ID,
	SYSFG_NB_IO_ID
};

void stm32mp_syscfg_enable_io_comp(enum syscfg_io_ids id);
void stm32mp_syscfg_fixed_io_comp(enum syscfg_io_ids id, uint32_t pmos, uint32_t nmos);

int stm32mp2_pwr_init_io_domains(void);

/* Get RISAF platform instance ID from peripheral IO memory base address */
int stm32_risaf_get_instance(uintptr_t base);

/* Get RISAF peripheral IO memory base address from platform instance ID */
uintptr_t stm32_risaf_get_base(int instance);

/* Get RISAF maximum number of regions from platform instance ID */
int stm32_risaf_get_max_region(int instance);

/* Get RISAF memory base address from platform instance ID */
uintptr_t stm32_risaf_get_memory_base(int instance);

/* Get RISAF memory size in bytes from platform instance ID */
size_t stm32_risaf_get_memory_size(int instance);

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

void stm32mp_ca35_set_vbar(uintptr_t vbar);

#endif /* STM32MP2_PRIVATE_H */
