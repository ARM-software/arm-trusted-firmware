/*
 * Copyright (c) 2024-2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __K3LOW_LPM_TIMEOUT_H__
#define __K3LOW_LPM_TIMEOUT_H__

#include <plat/common/platform.h>

/**
 * @brief Function with 1 micro second delay
 */
__wkupsramfunc void k3low_lpm_delay_1us(void);

#endif /* __K3LOW_LPM_TIMEOUT_H__ */
