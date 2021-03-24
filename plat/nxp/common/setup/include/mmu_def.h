/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef MMU_MAP_DEF_H
#define MMU_MAP_DEF_H

#include <lib/xlat_tables/xlat_tables_defs.h>

#include <platform_def.h>


#define LS_MAP_CCSR		MAP_REGION_FLAT(NXP_CCSR_ADDR, \
					NXP_CCSR_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE)

#ifdef NXP_DCSR_ADDR
#define LS_MAP_DCSR		MAP_REGION_FLAT(NXP_DCSR_ADDR, \
					NXP_DCSR_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE)
#endif

#define LS_MAP_CONSOLE		MAP_REGION_FLAT(NXP_DUART1_ADDR, \
					NXP_DUART_SIZE, \
					MT_DEVICE | MT_RW | MT_NS)

#define LS_MAP_OCRAM		MAP_REGION_FLAT(NXP_OCRAM_ADDR, \
					NXP_OCRAM_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE)

#endif /* MMU_MAP_DEF_H */
