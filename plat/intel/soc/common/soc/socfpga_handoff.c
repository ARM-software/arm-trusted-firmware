/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include "socfpga_handoff.h"

#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) |	\
				(((x) & 0x0000FF00) << 8) | ((x) << 24))

int socfpga_get_handoff(handoff *reverse_hoff_ptr)
{
	int i;
	uint32_t *buffer;
	handoff *handoff_ptr = (handoff *) PLAT_HANDOFF_OFFSET;

	memcpy(reverse_hoff_ptr, handoff_ptr, sizeof(handoff));
	buffer = (uint32_t *)reverse_hoff_ptr;

	/* convert big endian to little endian */
	for (i = 0; i < sizeof(handoff) / 4; i++)
		buffer[i] = SWAP_UINT32(buffer[i]);

	if (reverse_hoff_ptr->header_magic != HANDOFF_MAGIC_HEADER)
		return -1;
	if (reverse_hoff_ptr->pinmux_sel_magic != HANDOFF_MAGIC_PINMUX_SEL)
		return -1;
	if (reverse_hoff_ptr->pinmux_io_magic != HANDOFF_MAGIC_IOCTLR)
		return -1;
	if (reverse_hoff_ptr->pinmux_fpga_magic != HANDOFF_MAGIC_FPGA)
		return -1;
	if (reverse_hoff_ptr->pinmux_delay_magic != HANDOFF_MAGIC_IODELAY)
		return -1;

	return 0;
}
