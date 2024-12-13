/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <drivers/pmic/pmic_swap_api.h>

#pragma weak is_second_pmic_pp_swap

bool is_second_pmic_pp_swap(void)
{
	return false;
}
