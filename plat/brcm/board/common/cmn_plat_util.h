/*
 * Copyright (c) 2015 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CMN_PLAT_UTIL_H
#define CMN_PLAT_UTIL_H

#include <lib/mmio.h>

/* BOOT source */
#define BOOT_SOURCE_MASK	7
#define BOOT_SOURCE_QSPI	0
#define BOOT_SOURCE_NAND	1
#define BOOT_SOURCE_SPI_NAND	2
#define BOOT_SOURCE_UART	3
#define BOOT_SOURCE_RES4	4
#define BOOT_SOURCE_EMMC	5
#define BOOT_SOURCE_ATE		6
#define BOOT_SOURCE_USB		7
#define BOOT_SOURCE_MAX		8
#define BOOT_SOURCE_UNKNOWN	(-1)

#define KHMAC_SHA256_KEY_SIZE	32

#define SOFT_PWR_UP_RESET_L0	0
#define SOFT_SYS_RESET_L1	1
#define SOFT_RESET_L3		0x3

#define BOOT_SOURCE_SOFT_DATA_OFFSET	8
#define BOOT_SOURCE_SOFT_ENABLE_OFFSET	14
#define BOOT_SOURCE_SOFT_ENABLE_MASK	BIT(BOOT_SOURCE_SOFT_ENABLE_OFFSET)

typedef struct _key {
	uint8_t  hmac_sha256[KHMAC_SHA256_KEY_SIZE];
} cmn_key_t;

uint32_t boot_source_get(void);
void bl1_platform_wait_events(void);
void plat_soft_reset(uint32_t reset);

#endif
