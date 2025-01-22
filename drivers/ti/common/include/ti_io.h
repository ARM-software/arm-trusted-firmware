/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * I/O Register Access
 *
 * This header provides convenience macros for reading and writing 32-bit
 * memory-mapped I/O registers using the underlying mmio functions.
 */

#ifndef TI_PM_IO_H
#define TI_PM_IO_H

#include <lib/mmio.h>


#define readl(REG) mmio_read_32(REG)

#define writel(VAL, REG) mmio_write_32((uintptr_t)REG, VAL)

#endif
