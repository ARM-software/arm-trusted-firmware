/*
 * Copyright (c) 2015-2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EMMC_CONFIG_H
#define EMMC_CONFIG_H

/* RCA */
#define EMMC_RCA		1UL
/* 314ms (freq = 400KHz, timeout Counter = 0x04(SDCLK * 2^17)  */
#define EMMC_RW_DATA_TIMEOUT	0x40UL
/* how many times to try after fail. Don't change. */
#define EMMC_RETRY_COUNT	0
#define EMMC_CMD_MAX		60UL	/* Don't change. */

#define LOADIMAGE_FLAGS_DMA_ENABLE	0x00000001UL

#endif /* EMMC_CONFIG_H */
