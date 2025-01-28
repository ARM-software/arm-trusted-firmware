/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_PRIVATE_H
#define PLATFORM_PRIVATE_H

#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <drivers/generic_delay_timer.h>
#include <board_def.h>

#define ADDR_DOWN(_adr) (_adr & XLAT_ADDR_MASK(2U))
#define SIZE_UP(_adr, _sz) (round_up((_adr + _sz), XLAT_BLOCK_SIZE(2U)) - ADDR_DOWN(_adr))

#define K3_MAP_REGION_FLAT(_adr, _sz, _attr) \
	MAP_REGION_FLAT(ADDR_DOWN(_adr), SIZE_UP(_adr, _sz), _attr)

extern const mmap_region_t plat_k3_mmap[];

void ti_init_scmi_server(void);

/* Any kind of SOC specific init can be done here */
void ti_soc_init(void);

#endif /* PLATFORM_PRIVATE_H */

