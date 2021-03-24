/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <plat_common.h>

#pragma weak board_enable_povdd
#pragma weak board_disable_povdd

bool board_enable_povdd(void)
{
#ifdef CONFIG_POVDD_ENABLE
	return true;
#else
	return false;
#endif
}

bool board_disable_povdd(void)
{
#ifdef CONFIG_POVDD_ENABLE
	return true;
#else
	return false;
#endif
}
