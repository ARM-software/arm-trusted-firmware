/*
 * Copyright (c) 2017-2020, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stm32mp_shared_resources.h>

/* Currently allow full access by non-secure to platform clock services */
bool stm32mp_nsec_can_access_clock(unsigned long clock_id)
{
	return true;
}

/* Currently allow full access by non-secure to platform reset services */
bool stm32mp_nsec_can_access_reset(unsigned int reset_id)
{
	return true;
}
