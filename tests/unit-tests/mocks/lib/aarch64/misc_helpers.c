/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "arch_helpers.h"

struct mock_sys_reg sysreg;

void gpt_tlbi_by_pa_ll(uint64_t pa, size_t size)
{
	return;
}
