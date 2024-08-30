/*
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include "socfpga_handoff.h"

#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) |	\
				(((x) & 0x0000FF00) << 8) | ((x) << 24))

int socfpga_get_handoff(handoff *reverse_hoff_ptr)
{
	int i;
	int j;
	uint32_t *buffer;
	uint32_t *handoff_ptr = (uint32_t *) PLAT_HANDOFF_OFFSET;
	uint32_t *reverse_hoff_ptr_dst = (uint32_t *) reverse_hoff_ptr;

	if (sizeof(*handoff_ptr) > sizeof(handoff)) {
		return -EOVERFLOW;
	}

	for (j = 0; j < sizeof(handoff) / 4; j++) {
		memcpy_s((void *) (reverse_hoff_ptr_dst + j), 1,
			(void *) (handoff_ptr + j), 1);
	}

	buffer = (uint32_t *)reverse_hoff_ptr_dst;

	/* convert big endian to little endian */
	for (i = 0; i < sizeof(handoff) / 4; i++)
		buffer[i] = SWAP_UINT32(buffer[i]);

	if (reverse_hoff_ptr->header_magic != HANDOFF_MAGIC_HEADER) {
		return -1;
	}
	if (reverse_hoff_ptr->pinmux_sel_magic != HANDOFF_MAGIC_PINMUX_SEL) {
		return -1;
	}
	if (reverse_hoff_ptr->pinmux_io_magic != HANDOFF_MAGIC_IOCTLR) {
		return -1;
	}
	if (reverse_hoff_ptr->pinmux_fpga_magic != HANDOFF_MAGIC_FPGA) {
		return -1;
	}
	if (reverse_hoff_ptr->pinmux_delay_magic != HANDOFF_MAGIC_IODELAY) {
		return -1;
	}
	if (reverse_hoff_ptr->clock_magic != HANDOFF_MAGIC_CLOCK) {
		return -1;
	}

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	if (reverse_hoff_ptr->peripheral_pwr_gate_magic != HANDOFF_MAGIC_PERIPHERAL) {
		return -1;
	}

	if (reverse_hoff_ptr->ddr_magic != HANDOFF_MAGIC_DDR) {
		return -1;
	}
#endif

	return 0;
}
