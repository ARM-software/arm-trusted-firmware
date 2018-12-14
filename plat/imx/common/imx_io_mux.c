/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <imx_regs.h>
#include <imx_io_mux.h>

void imx_io_muxc_set_pad_alt_function(uint32_t pad_mux_offset, uint32_t alt_function)
{
	uintptr_t addr = (uintptr_t)(MXC_IO_MUXC_BASE + pad_mux_offset);

	mmio_write_32(addr, alt_function);
}

void imx_io_muxc_set_pad_features(uint32_t pad_feature_offset, uint32_t pad_features)
{
	uintptr_t addr = (uintptr_t)(MXC_IO_MUXC_BASE + pad_feature_offset);

	mmio_write_32(addr, pad_features);
}
