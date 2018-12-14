/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <endian.h>

#include <platform_def.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "ns_access.h"

static void enable_devices_ns_access(struct csu_ns_dev *ns_dev, uint32_t num)
{
	uint32_t *base = (uint32_t *)CONFIG_SYS_FSL_CSU_ADDR;
	uint32_t *reg;
	uint32_t val;
	int i;

	for (i = 0; i < num; i++) {
		reg = base + ns_dev[i].ind / 2;
		val = be32toh(mmio_read_32((uintptr_t)reg));
		if (ns_dev[i].ind % 2 == 0) {
			val &= 0x0000ffff;
			val |= ns_dev[i].val << 16;
		} else {
			val &= 0xffff0000;
			val |= ns_dev[i].val;
		}
		mmio_write_32((uintptr_t)reg, htobe32(val));
	}
}

void enable_layerscape_ns_access(void)
{
	enable_devices_ns_access(ns_dev, ARRAY_SIZE(ns_dev));
}
