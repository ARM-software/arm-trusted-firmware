/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <types/errno.h>
#include <types/short_types.h>
#include <types/address_types.h>
#include <lib/mmio.h>

#define readl(REG) mmio_read_32(REG)
#define readw(REG) mmio_read_16(REG)
#define readb(REG) mmio_read_8(REG)

#define ti_clk_writel(VAL, REG) mmio_write_32((uintptr_t)REG, VAL)
#define writew(VAL, REG) mmio_write_16((uintptr_t)REG, VAL)
#define writeb(VAL, REG) mmio_write_8((uintptr_t)REG, VAL)

