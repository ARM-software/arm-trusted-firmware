/*
 * Copyright (c) 2024-2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LPM_DDR_H__
#define __LPM_DDR_H__

#include <plat/common/platform.h>

/**
 * @brief Put DDR in self refresh for rtc only mode
 *
 * @return 0 on success
 */
__wkupsramfunc int32_t k3low_put_ddr_in_rtc_lpm(void);

/**
 * @brief Restore DDR controller context and take DDR out of self refresh
 *
 * @return 0 on success
 */
__wkupsramfunc int32_t k3low_ddr_deep_sleep_resume_sequence(void);

/**
 * @brief Save DDR register context, put DDR in self refresh and enable data retention
 *
 * @return 0 on success
 */
__wkupsramfunc int32_t k3low_ddr_deep_sleep_suspend_sequence(void);

#endif /* __LPM_DDR_H__ */
