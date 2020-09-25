/*
 * Copyright (c) 2017-2020, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRIVERS_ST_ETZPC_H
#define DRIVERS_ST_ETZPC_H

#include <stdbool.h>
#include <stdint.h>

/* Define security level for each peripheral (DECPROT) */
enum etzpc_decprot_attributes {
	ETZPC_DECPROT_S_RW = 0,
	ETZPC_DECPROT_NS_R_S_W = 1,
	ETZPC_DECPROT_MCU_ISOLATION = 2,
	ETZPC_DECPROT_NS_RW = 3,
	ETZPC_DECPROT_MAX = 4,
};

void etzpc_configure_decprot(uint32_t decprot_id,
			     enum etzpc_decprot_attributes decprot_attr);
enum etzpc_decprot_attributes etzpc_get_decprot(uint32_t decprot_id);
void etzpc_lock_decprot(uint32_t decprot_id);

void etzpc_configure_tzma(uint32_t tzma_id, uint16_t tzma_value);
uint16_t etzpc_get_tzma(uint32_t tzma_id);
void etzpc_lock_tzma(uint32_t tzma_id);
bool etzpc_get_lock_tzma(uint32_t tzma_id);

uint8_t etzpc_get_num_per_sec(void);
uint8_t etzpc_get_revision(void);
uintptr_t etzpc_get_base_address(void);

int etzpc_init(void);

#endif /* DRIVERS_ST_ETZPC_H */
