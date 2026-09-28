/*
 * Copyright (c) 2020-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <platform_def.h>
#include "socfpga_mcsr_s2c.h"

/* Register mappings */
#define S2C_BASE			SOCFPGA_MAILBOX_S2C_BASE
#define S2C_STATUS_REG		(*(volatile uint32_t *)(S2C_BASE + 0x00))
#define S2C_MSG_OUT			(*(volatile uint32_t *)(S2C_BASE + 0x04))

#define S2C_CTRL			(*(volatile uint32_t *) \
					 (SOCFPGA_MAILBOX_S2C_RINGBUF_BASE + 0x00))
#define S2C_BUF_IN			(*(volatile uint32_t *) \
					 (SOCFPGA_MAILBOX_S2C_RINGBUF_BASE + 0x60))
#define S2C_COUT_REG		S2C_MSG_OUT
#define S2C_CIN_REG			S2C_BUF_IN
#define S2C_DESC_BASE		(SOCFPGA_MAILBOX_S2C_RINGBUF_BASE + 0x04)

#define S2C_NUM_DESC		11
#define S2C_DESC_REGS		2

/* Descriptor masks */
#define MCSR_ADDR_HIGH_MASK	0xFFU
#define MCSR_SIZE_MASK		0x07FF0000U
#define MCSR_SIZE_SHIFT		16

/* Internal helper: write a descriptor into hardware */
static inline void mcsr_s2c_write_desc(uint32_t index,
				       const struct mcsr_s2c_desc *desc)
{
	volatile uint32_t *hw_desc;

	hw_desc = (volatile uint32_t *)(S2C_DESC_BASE +
					index * S2C_DESC_REGS * sizeof(uint32_t));
	hw_desc[0] = desc->addr_low;
	hw_desc[1] = desc->word1;
}

/* Internal helper: read a descriptor from hardware */
static inline void mcsr_s2c_read_desc(uint32_t index,
				      struct mcsr_s2c_desc *desc)
{
	volatile uint32_t *hw_desc;

	hw_desc = (volatile uint32_t *)(S2C_DESC_BASE +
			index * S2C_DESC_REGS * sizeof(uint32_t));
	desc->addr_low = hw_desc[0];
	desc->word1 = hw_desc[1];
}

/* Initialize mailbox */
void mcsr_s2c_init(void)
{
	S2C_CIN_REG = 0;
}

/* Descriptor helpers */
void mcsr_s2c_desc_set(struct mcsr_s2c_desc *desc, uint64_t addr, uint32_t size_words)
{
	uint32_t addr_high = (uint32_t)((addr >> 32) & MCSR_ADDR_HIGH_MASK);

	desc->addr_low = (uint32_t)(addr & 0xFFFFFFFFULL);
	desc->word1 = (addr_high & MCSR_ADDR_HIGH_MASK) |
		      ((size_words << MCSR_SIZE_SHIFT) & MCSR_SIZE_MASK);
}

uint64_t mcsr_s2c_desc_get_addr(const struct mcsr_s2c_desc *desc)
{
	uint64_t addr;
	uint64_t high = desc->word1 & MCSR_ADDR_HIGH_MASK;

	addr = (high << 32) | desc->addr_low;
	return addr;
}

uint32_t mcsr_s2c_desc_get_size(const struct mcsr_s2c_desc *desc)
{
	uint32_t size;

	size = (desc->word1 & MCSR_SIZE_MASK) >> MCSR_SIZE_SHIFT;
	return size;
}

/* Ring buffer helpers */
uint32_t mcsr_s2c_get_current_cin(void)
{
	uint32_t head = S2C_CIN_REG;
	return head;
}

uint32_t mcsr_s2c_get_next_cin(void)
{
	uint32_t next = (S2C_CIN_REG + 1) % S2C_NUM_DESC;
	return next;
}

uint32_t mcsr_s2c_get_next_index(uint32_t index)
{
	uint32_t next = (index + 1) % S2C_NUM_DESC;
	return next;
}

void mcsr_s2c_update_cin(uint32_t index)
{
	S2C_CIN_REG = index;
}

uint32_t mcsr_s2c_get_current_cout(void)
{
	uint32_t tail = S2C_COUT_REG;
	return tail;
}

uint32_t mcsr_s2c_get_previous_cout(uint32_t cout)
{
	uint32_t prev = (cout == 0) ? (S2C_NUM_DESC - 1) : (cout - 1);
	return prev;
}

bool mcsr_s2c_is_full(void)
{
	bool full;

	if (mcsr_s2c_get_next_cin() == S2C_COUT_REG) {
		full = true;
	} else {
		full = false;
	}

	return full;
}

bool mcsr_s2c_sw_is_full(uint8_t cin, uint8_t cout)
{
	/* Use the SW CIN instead of HW CIN. */
	return ((cin + 1) % S2C_NUM_DESC) == cout;
}

bool mcsr_s2c_is_empty(void)
{
	bool empty;

	if (S2C_CIN_REG == S2C_COUT_REG) {
		empty = true;
	} else {
		empty = false;
	}

	return empty;
}

/* Enqueue descriptor */
bool mcsr_s2c_push(const struct mcsr_s2c_desc *desc, uint32_t index)
{
	bool status;

	if (mcsr_s2c_is_full()) {
		status = false;
	} else {
		mcsr_s2c_write_desc(index, desc);

		status = true;
	}

	return status;
}

/* Dequeue descriptor */
bool mcsr_s2c_pop(struct mcsr_s2c_desc *desc)
{
	bool status;

	if (mcsr_s2c_is_empty()) {
		status = false;
	} else {
		uint32_t tail = S2C_COUT_REG;

		mcsr_s2c_read_desc(tail, desc);

		S2C_COUT_REG = mcsr_s2c_get_next_index(tail);

		status = true;
	}

	return status;
}
