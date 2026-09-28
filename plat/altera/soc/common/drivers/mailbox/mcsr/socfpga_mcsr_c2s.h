/*
 * Copyright (c) 2019-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SOCFPGA_MCSR_C2S_H_
#define _SOCFPGA_MCSR_C2S_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* Descriptor Structure                                                       */
/* -------------------------------------------------------------------------- */
struct mcsr_c2s_desc {
	uint32_t addr_low;	/* Lower 32-bit address */
	uint32_t word1;		/* Upper address + size field */
};

/* -------------------------------------------------------------------------- */
/* Descriptor Constants                                                       */
/* -------------------------------------------------------------------------- */
/* Number of descriptors in the ring buffer */
#define MCSR_C2S_NUM_DESC			11U
#define MCSR_C2S_ADDR_HIGH_MASK		0x000000FFU
#define MCSR_C2S_SIZE_MASK			0x07FF0000U
#define MCSR_C2S_SIZE_SHIFT			16

/* -------------------------------------------------------------------------- */
/* Public API Functions                                                       */
/* -------------------------------------------------------------------------- */

/* Initialize the C2S mailbox */
void mcsr_c2s_init(void);

/* Set descriptor fields */
void mcsr_c2s_desc_set(struct mcsr_c2s_desc *d, uint64_t addr, uint32_t size_words);

/* Retrieve address or size from descriptor */
uint64_t mcsr_c2s_desc_get_addr(const struct mcsr_c2s_desc *d);
uint32_t mcsr_c2s_desc_get_size(const struct mcsr_c2s_desc *d);

/* Ring buffer operations */
bool mcsr_c2s_push(const struct mcsr_c2s_desc *d);  /* enqueue SW→HW */
bool mcsr_c2s_pop(struct mcsr_c2s_desc *d);         /* dequeue HW→SW */
bool mcsr_c2s_is_full(void);
bool mcsr_c2s_is_empty(void);

/* Head pointer utilities */
uint32_t mcsr_c2s_get_current_cin(void);
uint32_t mcsr_c2s_get_current_cout(void);
uint32_t mcsr_c2s_get_next_cin(void);

#endif /* _SOCFPGA_MCSR_C2S_H_ */
