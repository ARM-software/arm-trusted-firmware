/*
 * Copyright (c) 2019-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SOCFPGA_MCSR_S2C_H_
#define _SOCFPGA_MCSR_S2C_H_

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Descriptor for S2C mailbox */
struct mcsr_s2c_desc {
	uint32_t addr_low;
	uint32_t word1;
};

/* Initialize the S2C mailbox head */
void mcsr_s2c_init(void);

/* Enqueue descriptor (SW → HW) */
bool mcsr_s2c_push(const struct mcsr_s2c_desc *desc, uint32_t index);

/* Dequeue descriptor (HW → SW) */
bool mcsr_s2c_pop(struct mcsr_s2c_desc *desc);

/* Descriptor helpers */
void mcsr_s2c_desc_set(struct mcsr_s2c_desc *desc, uint64_t addr, uint32_t size_words);
uint64_t mcsr_s2c_desc_get_addr(const struct mcsr_s2c_desc *desc);
uint32_t mcsr_s2c_desc_get_size(const struct mcsr_s2c_desc *desc);

/* Ring buffer helpers */
uint32_t mcsr_s2c_get_current_cin(void);
uint32_t mcsr_s2c_get_next_cin(void);
uint32_t mcsr_s2c_get_next_index(uint32_t index);
void mcsr_s2c_update_cin(uint32_t index);
uint32_t mcsr_s2c_get_current_cout(void);
uint32_t mcsr_s2c_get_previous_cout(uint32_t cout);
bool mcsr_s2c_sw_is_full(uint8_t cin, uint8_t cout);
bool mcsr_s2c_is_full(void);
bool mcsr_s2c_is_empty(void);

#endif /* _SOCFPGA_MCSR_S2C_H_ */
