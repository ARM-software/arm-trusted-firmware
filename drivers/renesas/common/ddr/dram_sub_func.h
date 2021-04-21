/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRAM_SUB_FUNC_H
#define DRAM_SUB_FUNC_H

#define DRAM_UPDATE_STATUS_ERR	-1
#define DRAM_BOOT_STATUS_COLD	0
#define DRAM_BOOT_STATUS_WARM	1

int32_t rcar_dram_update_boot_status(uint32_t status);
void rcar_dram_get_boot_status(uint32_t *status);

#endif /* DRAM_SUB_FUNC_H */
