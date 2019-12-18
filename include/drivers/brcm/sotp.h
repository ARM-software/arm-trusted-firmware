/*
 * Copyright (c) 2016-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOTP_H
#define SOTP_H

#include <stddef.h>
#include <stdint.h>

#include <platform_sotp.h>

#define SOTP_ROW_NO_ECC 0
#define SOTP_ROW_ECC 1

#define SOTP_STATUS_1 (SOTP_REGS_OTP_BASE + 0x001c)
#define SOTP_FAIL_BITS 0x18000000000
#define SOTP_ECC_ERR_DETECT 0x8000000000000000

#define SOTP_REGS_SOTP_CHIP_STATES (SOTP_REGS_OTP_BASE + 0x0028)
#define SOTP_REGS_OTP_WR_LOCK (SOTP_REGS_OTP_BASE + 0x0038)

#define SOTP_CHIP_STATES_MANU_DEBUG_MASK (1 << 8)
#define SOTP_DEVICE_SECURE_CFG0_OTP_ERASED_MASK (3 << 16)
#define SOTP_REGS_SOTP_CHIP_STATES_OTP_ERASED_MASK (1 << 16)

#define SOTP_DEVICE_SECURE_CFG0_CID_MASK (3 << 2)
#define SOTP_DEVICE_SECURE_CFG0_AB_MASK  (3 << 6)
#define SOTP_DEVICE_SECURE_CFG0_DEV_MASK (3 << 8)

#define SOTP_BOOT_SOURCE_SHIFT 8
/* bits 14 and 15 */
#define SOTP_BOOT_SOURCE_ENABLE_MASK (0xC0 << SOTP_BOOT_SOURCE_SHIFT)
/* bits 8 to 13 */
#define SOTP_BOOT_SOURCE_BITS0 (0x03 << SOTP_BOOT_SOURCE_SHIFT)
#define SOTP_BOOT_SOURCE_BITS1 (0x0C << SOTP_BOOT_SOURCE_SHIFT)
#define SOTP_BOOT_SOURCE_BITS2 (0x30 << SOTP_BOOT_SOURCE_SHIFT)
#define SOTP_BOOT_SOURCE_MASK (0x3F << SOTP_BOOT_SOURCE_SHIFT)

#define SOTP_ATF_CFG_ROW_ID SOTP_DEVICE_SECURE_CFG2_ROW
/* bits 28 and 29 */
#define SOTP_SBL_MASK (3 << 28)
/* bits 30 and 31 */
#define SOTP_ATF_NVCOUNTER_ENABLE_MASK ((uint64_t)3 << 30)
/* bits 32 and 33 */
#define SOTP_ATF_WATCHDOG_ENABLE_MASK ((uint64_t)3 << 32)
/* bits 34 and 35 */
#define SOTP_ATF_PLL_ON ((uint64_t)3 << 34)
/* bits 36 and 37 */
#define SOTP_ATF_RESET_RETRY ((uint64_t)3 << 36)
/* bits 38 to 40 */
#define SOTP_ATF_LOG_LEVEL_SHIFT 38
#define SOTP_ATF_LOG_LEVEL ((uint64_t)7 << SOTP_ATF_LOG_LEVEL_SHIFT)

#define SOTP_ATF2_CFG_ROW_ID SOTP_DEVICE_SECURE_CFG3_ROW
/* bits 16 and 17 */
#define SOTP_ROMKEY_MASK (3 << 16)
/* bits 18 and 19 */
#define SOTP_EC_EN_MASK (3 << 18)

#define SOTP_ENC_DEV_TYPE_AB_DEV ((uint64_t)0x19999800000)
#define SOTP_ENC_DEV_TYPE_MASK   ((uint64_t)0x1ffff800000)

uint64_t sotp_mem_read(uint32_t offset, uint32_t sotp_add_ecc);
void sotp_mem_write(uint32_t addr, uint32_t sotp_add_ecc, uint64_t wdata);
int sotp_read_key(uint8_t *key, size_t keysize, int start_row, int end_row);
int sotp_key_erased(void);
uint32_t sotp_redundancy_reduction(uint32_t sotp_row_data);
#endif
