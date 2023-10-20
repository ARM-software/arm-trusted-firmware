/*
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#ifndef PLAT_FDT_H
#define PLAT_FDT_H

void prepare_dtb(void);

#if defined(XILINX_OF_BOARD_DTB_ADDR)
int32_t is_valid_dtb(void *fdt);
#endif

#endif /* PLAT_FDT_H */
