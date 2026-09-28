/*
 * Copyright (c) 2022-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/cadence/cdns_nand.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils.h>

#include "nand.h"

int nand_init(void)
{
	return cdns_nand_host_init();
}
