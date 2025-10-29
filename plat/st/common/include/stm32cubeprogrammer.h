/*
 * Copyright (c) 2021-2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32CUBEPROGRAMMER_H
#define STM32CUBEPROGRAMMER_H

#include <stdint.h>

#include <usb_dfu.h>

/* Phase definition */
#define PHASE_FLASHLAYOUT	0U
#define PHASE_SSBL		3U
#define PHASE_CMD		0xF1U
#define PHASE_RESET		0xFFU

/* Functions provided by plat */
uint8_t usb_dfu_get_phase(uint8_t alt);

int stm32cubeprog_usb_load(struct usb_handle *usb_core_handle,
			   uint8_t phase,
			   uintptr_t ssbl_base,
			   size_t ssbl_len);

int stm32cubeprog_uart_load(uintptr_t instance, uint8_t phase,
			    uintptr_t base, size_t len);

#endif /* STM32CUBEPROGRAMMER_H */
