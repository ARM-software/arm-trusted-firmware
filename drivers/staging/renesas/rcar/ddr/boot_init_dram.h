/*
 * Copyright (c) 2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOOT_INIT_DRAM_H
#define BOOT_INIT_DRAM_H

extern int32_t rcar_dram_init(void);

#define INITDRAM_OK (0)
#define INITDRAM_NG (0xffffffff)
#define INITDRAM_ERR_I (0xffffffff)
#define INITDRAM_ERR_O (0xfffffffe)
#define INITDRAM_ERR_T (0xfffffff0)

#endif /* BOOT_INIT_DRAM_H */
