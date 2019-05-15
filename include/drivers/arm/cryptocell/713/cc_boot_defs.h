/*
 * Copyright (c) 2017-2020 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef  _CC_BOOT_DEFS_H
#define  _CC_BOOT_DEFS_H

/*!
 @file
 @brief This file contains general definitions of types and enums of Boot APIs.
 */

#ifdef __cplusplus
extern "C"
{
#endif

/*! Version counters value. */
typedef enum {

    CC_SW_VERSION_TRUSTED = 0,          /*!<  Trusted counter. */
    CC_SW_VERSION_NON_TRUSTED,          /*!<  Non trusted counter. */
    CC_SW_VERSION_MAX = 0x7FFFFFFF      /*!< Reserved */
} CCSbSwVersionId_t;

/*! The hash boot key definition. */
typedef enum {
    CC_SB_HASH_BOOT_KEY_0_128B = 0,     /*!< Hbk0: 128-bit truncated SHA-256 digest of PubKB0. Used by ICV */
    CC_SB_HASH_BOOT_KEY_1_128B = 1,     /*!< Hbk1: 128-bit truncated SHA-256 digest of PubKB1. Used by OEM */
    CC_SB_HASH_BOOT_KEY_256B = 2,       /*!< Hbk: 256-bit SHA-256 digest of public key. */
    CC_SB_HASH_BOOT_NOT_USED = 0xF,     /*!< Hbk is not used. */
    CC_SB_HASH_MAX_NUM = 0x7FFFFFFF,    /*!< Reserved. */
} CCSbPubKeyIndexType_t;

/*! Chip state. */
typedef enum {
    CHIP_STATE_NOT_INITIALIZED = 0,     /*! Chip is not initialized. */
    CHIP_STATE_TEST = 1,                /*! Chip is in Production state. */
    CHIP_STATE_PRODUCTION = 2,          /*! Chip is in Production state. */
    CHIP_STATE_ERROR = 3,               /*! Chip is in Error state. */
} CCBsvChipState_t;
#ifdef __cplusplus
}
#endif

#endif /*_CC_BOOT_DEFS_H */

/**
@}
 */
