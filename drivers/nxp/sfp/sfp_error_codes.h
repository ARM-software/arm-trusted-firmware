/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef SFP_ERROR_CODES_H
#define SFP_ERROR_CODES_H

 /* Error codes */
#define ERROR_FUSE_BARKER           0x1
#define ERROR_READFB_CMD            0x2
#define ERROR_PROGFB_CMD            0x3
#define ERROR_SRKH_ALREADY_BLOWN    0x4
#define ERROR_SRKH_WRITE            0x5
#define ERROR_OEMUID_ALREADY_BLOWN  0x6
#define ERROR_OEMUID_WRITE          0x7
#define ERROR_DCV_ALREADY_BLOWN     0x8
#define ERROR_DCV_WRITE             0x9
#define ERROR_DRV_ALREADY_BLOWN     0xa
#define ERROR_DRV_HAMMING_ERROR     0xb
#define ERROR_OTPMK_ALREADY_BLOWN   0xc
#define ERROR_OTPMK_HAMMING_ERROR   0xd
#define ERROR_OTPMK_USER_MIN        0xe
#define ERROR_OSPR1_ALREADY_BLOWN   0xf
#define ERROR_OSPR1_WRITE           0x10
#define ERROR_SC_ALREADY_BLOWN      0x11
#define ERROR_SC_WRITE              0x12
#define ERROR_POVDD_GPIO_FAIL       0x13
#define ERROR_GPIO_SET_FAIL         0x14
#define ERROR_GPIO_RESET_FAIL       0x15
#define ERROR_OTPMK_SEC_DISABLED    0x16

#endif /* SFP_ERROR_CODES_H */
