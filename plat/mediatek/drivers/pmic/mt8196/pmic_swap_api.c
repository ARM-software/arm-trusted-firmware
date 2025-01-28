/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <drivers/pmic/pmic_swap_api.h>

/* No need to check second pmic mt6373 */
bool is_second_pmic_pp_swap(void)
{
	return false;
}
