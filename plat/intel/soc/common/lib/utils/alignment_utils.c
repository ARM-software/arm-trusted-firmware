/*
 * Copyright (c) 2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include "alignment_utils.h"
#include "socfpga_mailbox.h"

bool is_size_4_bytes_aligned(uint32_t size)
{
	if ((size % MBOX_WORD_BYTE) != 0U) {
		return false;
	} else {
		return true;
	}
}

bool is_8_bytes_aligned(uint32_t data)
{
	if ((data % (MBOX_WORD_BYTE * 2U)) != 0U) {
		return false;
	} else {
		return true;
	}
}

/* As of now used on only Agilex5 platform. */
bool is_16_bytes_aligned(uint32_t data)
{
	if ((data % (MBOX_WORD_BYTE * 4U)) != 0U)
		return false;
	else
		return true;
}

bool is_32_bytes_aligned(uint32_t data)
{
	if ((data % (8U * MBOX_WORD_BYTE)) != 0U) {
		return false;
	} else {
		return true;
	}
}
