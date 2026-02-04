/*
 * Copyright (c) 2024-2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LPM_GTC_H__
#define __LPM_GTC_H__

#include <plat/common/platform.h>

/**
 * @brief Save GTC counter and disable GTC
 *
 * @return 0 on success
 */
int32_t k3low_lpm_sleep_suspend_gtc(void);

/**
 * @brief Restore GTC counter and enable GTC
 *
 * @return 0 on success
 */
int32_t k3low_lpm_resume_gtc(void);

#endif /* __LPM_GTC_H__ */
