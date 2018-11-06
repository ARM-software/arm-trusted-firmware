/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file  emmc_config.h
 * @brief Configuration file
 *
 */

#ifndef EMMC_CONFIG_H
#define EMMC_CONFIG_H

/* ************************ HEADER (INCLUDE) SECTION *********************** */

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */

/** @brief MMC driver config
 */
#define EMMC_RCA                1UL	/* RCA  */
#define EMMC_RW_DATA_TIMEOUT    0x40UL	/* 314ms (freq = 400KHz, timeout Counter = 0x04(SDCLK * 2^17)  */
#define EMMC_RETRY_COUNT        0	/* how many times to try after fail. Don't change. */
#define EMMC_CMD_MAX            60UL	/* Don't change. */

/** @brief etc
 */
#define LOADIMAGE_FLAGS_DMA_ENABLE              0x00000001UL

/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */

/* ********************** DECLARATION OF EXTERNAL DATA ********************* */

/* ************************** FUNCTION PROTOTYPES ************************** */

/* ********************************* CODE ********************************** */

#endif /* EMMC_CONFIG_H */
/* ******************************** END ************************************ */
