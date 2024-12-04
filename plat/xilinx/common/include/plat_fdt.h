/*
 * Copyright (c) 2023-2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#ifndef PLAT_FDT_H
#define PLAT_FDT_H

void prepare_dtb(void);
uintptr_t plat_retrieve_dt_addr(void);
int32_t is_valid_dtb(void *fdt);

#endif /* PLAT_FDT_H */
