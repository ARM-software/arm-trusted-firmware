/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCUPM_CFG_H
#define MCUPM_CFG_H

#if defined(MCUPM_VERSION_v1)
#define CPU_EB_MBOX3_OFFSET		0xCE0
#elif defined(MCUPM_VERSION_v2) /* MCUPM_VERSION_v1 */
#define CPU_EB_MBOX3_OFFSET		0x7E0
#define CPU_EB_MCUSYS_CNT_OFST		0x944
#elif defined(MCUPM_VERSION_v3) /* MCUPM_VERSION_v2 */
#define CPU_EB_MBOX3_OFFSET		0x7E0
#define CPU_EB_MCUSYS_CNT_OFST		0xE44
#define CPU_EB_RET_STA_OFFSET		0x350
#endif /* MCUPM_VERSION_v3 */

#endif /* MCUPM_CFG_H */
