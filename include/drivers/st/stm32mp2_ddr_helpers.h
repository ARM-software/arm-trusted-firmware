/*
 * Copyright (c) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP2_DDR_HELPERS_H
#define STM32MP2_DDR_HELPERS_H

#include <stdbool.h>
#include <stdint.h>

#include <drivers/st/stm32mp2_ddr_regs.h>

enum stm32mp2_ddr_sr_mode {
	DDR_SR_MODE_INVALID = 0,
	DDR_SSR_MODE,
	DDR_HSR_MODE,
	DDR_ASR_MODE,
};

void ddr_activate_controller(struct stm32mp_ddrctl *ctl, bool sr_entry);
void ddr_wait_lp3_mode(bool state);
int ddr_sr_exit_loop(void);
uint32_t ddr_get_io_calibration_val(void);
int ddr_sr_entry(bool standby);
int ddr_sr_exit(void);
enum stm32mp2_ddr_sr_mode ddr_read_sr_mode(void);
void ddr_set_sr_mode(enum stm32mp2_ddr_sr_mode mode);
void ddr_save_sr_mode(void);
void ddr_restore_sr_mode(void);
void ddr_sub_system_clk_init(void);
void ddr_sub_system_clk_off(void);

#endif /* STM32MP2_DDR_HELPERS_H */
