/*
 * Copyright (c) 2020-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include "socfpga_mcsr_c2s.h"

/* -------------------------------------------------------------------------- */
/* Hardware Register Definitions                                              */
/* -------------------------------------------------------------------------- */

/* RO registers */
#define C2S_RO_BASE				SOCFPGA_MAILBOX_C2S_BASE
#define C2S_STATUS_REG			(*(volatile uint32_t *)(C2S_RO_BASE + 0x00))
/* HW tail pointer */
#define C2S_COUT_REG			(*(volatile uint32_t *)(C2S_RO_BASE + 0x04))

/* RW region */
#define C2S_RW_BASE				SOCFPGA_MAILBOX_C2S_RINGBUF_BASE
#define C2S_CTRL_REG			(*(volatile uint32_t *)(C2S_RW_BASE + 0x00))
#define C2S_CIN_REG			(*(volatile uint32_t *)(C2S_RW_BASE + 0x60))
#define C2S_RESET_REG			(*(volatile uint32_t *)(C2S_RW_BASE + 0x64))

#define C2S_DESC_BASE			(C2S_RW_BASE + 0x04)
#define C2S_DESC_REGS			2U

/* -------------------------------------------------------------------------- */
/* Internal Helper Functions                                                  */
/* -------------------------------------------------------------------------- */
static inline uintptr_t c2s_desc_addr(uint32_t index)
{
	return C2S_DESC_BASE + index * C2S_DESC_REGS * sizeof(uint32_t);
}

static inline void c2s_write_desc(uint32_t index, const struct mcsr_c2s_desc *d)
{
	volatile uint32_t *reg = (volatile uint32_t *)c2s_desc_addr(index);

	reg[0] = d->addr_low;
	reg[1] = d->word1;
}

static inline void c2s_read_desc(uint32_t index, struct mcsr_c2s_desc *d)
{
	volatile uint32_t *reg = (volatile uint32_t *)c2s_desc_addr(index);

	d->addr_low = reg[0];
	d->word1	= reg[1];
}

void mcsr_c2s_init(void)
{
	/* Reset ring buffer */
	C2S_CIN_REG = 0;
	C2S_RESET_REG = 1;
}

uint32_t mcsr_c2s_get_current_cin(void)
{
	return C2S_CIN_REG;
}

uint32_t mcsr_c2s_get_current_cout(void)
{
	return C2S_COUT_REG;
}

uint32_t mcsr_c2s_get_next_cin(void)
{
	return (C2S_CIN_REG + 1U) % MCSR_C2S_NUM_DESC;
}

/* -------------------------------------------------------------------------- */
/* Descriptor API                                                             */
/* -------------------------------------------------------------------------- */
void mcsr_c2s_desc_set(struct mcsr_c2s_desc *d, uint64_t addr, uint32_t size_words)
{
	d->addr_low = (uint32_t)(addr & 0xFFFFFFFFULL);
	uint32_t addr_high = (uint32_t)((addr >> 32) & MCSR_C2S_ADDR_HIGH_MASK);

	d->word1 = (addr_high & MCSR_C2S_ADDR_HIGH_MASK) |
				((size_words << MCSR_C2S_SIZE_SHIFT) & MCSR_C2S_SIZE_MASK);
}

uint64_t mcsr_c2s_desc_get_addr(const struct mcsr_c2s_desc *d)
{
	uint64_t high = d->word1 & MCSR_C2S_ADDR_HIGH_MASK;

	return (high << 32) | d->addr_low;
}

uint32_t mcsr_c2s_desc_get_size(const struct mcsr_c2s_desc *d)
{
	return (d->word1 & MCSR_C2S_SIZE_MASK) >> MCSR_C2S_SIZE_SHIFT;
}

/* -------------------------------------------------------------------------- */
/* Ring Buffer                                                                */
/* -------------------------------------------------------------------------- */
bool mcsr_c2s_is_full(void)
{
	uint32_t head = C2S_CIN_REG;
	uint32_t tail = C2S_COUT_REG;

	return ((head + 1U) % MCSR_C2S_NUM_DESC) == tail;
}

bool mcsr_c2s_is_empty(void)
{
	return C2S_CIN_REG == C2S_COUT_REG;
}

bool mcsr_c2s_push(const struct mcsr_c2s_desc *d)
{
	if (mcsr_c2s_is_full()) {
		return false;
	}

	uint32_t head = C2S_CIN_REG;

	c2s_write_desc(head, d);
	C2S_CIN_REG = (head + 1U) % MCSR_C2S_NUM_DESC;

	return true;
}

bool mcsr_c2s_pop(struct mcsr_c2s_desc *d)
{
	if (mcsr_c2s_is_empty()) {
		return false;
	}

	uint32_t tail = C2S_COUT_REG;

	c2s_read_desc(tail, d);

	C2S_COUT_REG = (tail + 1U) % MCSR_C2S_NUM_DESC;

	return true;
}
