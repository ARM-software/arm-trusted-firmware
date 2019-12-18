/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_INIT_H
#define DDR_INIT_H

#include <fru.h>

#pragma weak ddr_initialize
#pragma weak ddr_secure_region_config
#pragma weak ddr_info_save
#pragma weak get_active_ddr_channel
#pragma weak is_warmboot

void ddr_initialize(struct ddr_info *ddr)
{
}

void ddr_secure_region_config(uint64_t start, uint64_t end)
{
}

void ddr_info_save(void)
{
}

unsigned char get_active_ddr_channel(void)
{
	return 0;
}

static inline unsigned int is_warmboot(void)
{
	return 0;
}
#endif
