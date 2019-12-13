/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_BRCM_H
#define PLAT_BRCM_H

#include <stdint.h>

#include <lib/cassert.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables.h>

#include <platform_def.h>

struct image_info;

/* Global variables */
extern const mmap_region_t plat_brcm_mmap[];

uint32_t brcm_get_spsr_for_bl32_entry(void);
uint32_t brcm_get_spsr_for_bl33_entry(void);
const mmap_region_t *plat_brcm_get_mmap(void);
int bcm_bl2_handle_scp_bl2(struct image_info *image_info);
void plat_brcm_io_setup(void);
void plat_brcm_process_flags(uint16_t plat_toc_flags);

#endif /* PLAT_BRCM_H */
