/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LPM_DDR_H__
#define __LPM_DDR_H__

#include <plat/common/platform.h>

/**
 *  \brief  Put DDR in self refresh for rtc only mode
 *
 *  \return ret SUCCESS on success
 */
__wkupsramfunc int32_t put_ddr_in_rtc_lpm(void);

/**
 *  \brief  Put ddr in self refresh
 *
 * @param enable bool to chose between enable and disable
 *
 *  \return ret SUCCESS on success
 */
__wkupsramfunc void put_ddr_in_sr(bool enable);

/**
 *  \brief  Restore DDR register configs
 *
 *  \return ret SUCCESS on success
 */
__wkupsramfunc int32_t restore_ddr_reg_configs(void);

/**
 *  \brief  Save DDR register configs
 *
 *  \return ret SUCCESS on success
 */
__wkupsramfunc int32_t save_ddr_reg_configs(void);

#endif /* __LPM_DDR_H__ */
