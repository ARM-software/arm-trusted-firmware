/*
 * Copyright (c) 2017-2020 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BSV_ERROR_H
#define _BSV_ERROR_H

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This file defines the error code types that are returned from the Boot Services APIs.

@defgroup cc_bsv_error CryptoCell Boot Services error codes
@{
@ingroup cc_bsv
*/

/*! Defines the base address for Boot Services errors. */
#define CC_BSV_BASE_ERROR                             0x0B000000
/*! Defines the base address for Boot Services cryptographic errors. */
#define CC_BSV_CRYPTO_ERROR                           0x0C000000

/*! Illegal input parameter. */
#define CC_BSV_ILLEGAL_INPUT_PARAM_ERR                (CC_BSV_BASE_ERROR + 0x00000001)
/*! Illegal HUK value. */
#define CC_BSV_ILLEGAL_HUK_VALUE_ERR                  (CC_BSV_BASE_ERROR + 0x00000002)
/*! Illegal Kcp value. */
#define CC_BSV_ILLEGAL_KCP_VALUE_ERR                  (CC_BSV_BASE_ERROR + 0x00000003)
/*! Illegal Kce value. */
#define CC_BSV_ILLEGAL_KCE_VALUE_ERR                  (CC_BSV_BASE_ERROR + 0x00000004)
/*! Illegal Kpicv value. */
#define CC_BSV_ILLEGAL_KPICV_VALUE_ERR                (CC_BSV_BASE_ERROR + 0x00000005)
/*! Illegal Kceicv value. */
#define CC_BSV_ILLEGAL_KCEICV_VALUE_ERR               (CC_BSV_BASE_ERROR + 0x00000006)
/*! Illegal EKcst value. */
#define CC_BSV_ILLEGAL_EKCST_VALUE_ERR                (CC_BSV_BASE_ERROR + 0x00000007)
/*! Hash boot key not programmed in the OTP. */
#define CC_BSV_HASH_NOT_PROGRAMMED_ERR                (CC_BSV_BASE_ERROR + 0x00000008)
/*! Illegal Hash boot key zero count in the OTP. */
#define CC_BSV_HBK_ZERO_COUNT_ERR                     (CC_BSV_BASE_ERROR + 0x00000009)
/*! Illegal LCS. */
#define CC_BSV_ILLEGAL_LCS_ERR                        (CC_BSV_BASE_ERROR + 0x0000000A)
/*! OTP write compare failure. */
#define CC_BSV_OTP_WRITE_CMP_FAIL_ERR                 (CC_BSV_BASE_ERROR + 0x0000000B)
/*! OTP access error */
#define CC_BSV_OTP_ACCESS_ERR                         (CC_BSV_BASE_ERROR + 0x0000000C)
/*! Erase key in OTP failed. */
#define CC_BSV_ERASE_KEY_FAILED_ERR                   (CC_BSV_BASE_ERROR + 0x0000000D)
/*! Illegal PIDR. */
#define CC_BSV_ILLEGAL_PIDR_ERR                       (CC_BSV_BASE_ERROR + 0x0000000E)
/*! Illegal CIDR. */
#define CC_BSV_ILLEGAL_CIDR_ERR                       (CC_BSV_BASE_ERROR + 0x0000000F)
/*! Device failed to move to fatal error state. */
#define CC_BSV_FAILED_TO_SET_FATAL_ERR                (CC_BSV_BASE_ERROR + 0x00000010)
/*! Failed to set RMA LCS. */
#define CC_BSV_FAILED_TO_SET_RMA_ERR                  (CC_BSV_BASE_ERROR + 0x00000011)
/*! Illegal RMA indication. */
#define CC_BSV_ILLEGAL_RMA_INDICATION_ERR             (CC_BSV_BASE_ERROR + 0x00000012)
/*! Boot Services version is not initialized. */
#define CC_BSV_VER_IS_NOT_INITIALIZED_ERR             (CC_BSV_BASE_ERROR + 0x00000013)
/*! APB secure mode is locked. */
#define CC_BSV_APB_SECURE_IS_LOCKED_ERR               (CC_BSV_BASE_ERROR + 0x00000014)
/*! APB privilege mode is locked. */
#define CC_BSV_APB_PRIVILEG_IS_LOCKED_ERR             (CC_BSV_BASE_ERROR + 0x00000015)
/*! Illegal operation. */
#define CC_BSV_ILLEGAL_OPERATION_ERR                  (CC_BSV_BASE_ERROR + 0x00000016)
/*! Illegal asset size. */
#define CC_BSV_ILLEGAL_ASSET_SIZE_ERR                 (CC_BSV_BASE_ERROR + 0x00000017)
/*! Illegal asset value. */
#define CC_BSV_ILLEGAL_ASSET_VAL_ERR                  (CC_BSV_BASE_ERROR + 0x00000018)
/*! Kpicv is locked. */
#define CC_BSV_KPICV_IS_LOCKED_ERR                    (CC_BSV_BASE_ERROR + 0x00000019)
/*! Illegal SW version. */
#define CC_BSV_ILLEGAL_SW_VERSION_ERR                 (CC_BSV_BASE_ERROR + 0x0000001A)
/*! AO write operation. */
#define CC_BSV_AO_WRITE_FAILED_ERR                    (CC_BSV_BASE_ERROR + 0x0000001B)
/*! Chip state is already initialized. */
#define CC_BSV_CHIP_INITIALIZED_ERR                   (CC_BSV_BASE_ERROR + 0x0000001C)
/*! SP is not enabled. */
#define CC_BSV_SP_NOT_ENABLED_ERR                     (CC_BSV_BASE_ERROR + 0x0000001D)
/*! Production secure provisioning - header fields. */
#define CC_BSV_PROD_PKG_HEADER_ERR                    (CC_BSV_BASE_ERROR + 0x0000001E)
/*! Production secure provisioning - header MAC. */
#define CC_BSV_PROD_PKG_HEADER_MAC_ERR                (CC_BSV_BASE_ERROR + 0x0000001F)
/*! Overrun buffer or size. */
#define CC_BSV_OVERRUN_ERR                            (CC_BSV_BASE_ERROR + 0x00000020)
/*! Kceicv is locked. */
#define CC_BSV_KCEICV_IS_LOCKED_ERR                   (CC_BSV_BASE_ERROR + 0x00000021)
/*! Chip indication is CHIP_STATE_ERROR. */
#define CC_BSV_CHIP_INDICATION_ERR                    (CC_BSV_BASE_ERROR + 0x00000022)
/*! Device is locked in fatal error state. */
#define CC_BSV_FATAL_ERR_IS_LOCKED_ERR                (CC_BSV_BASE_ERROR + 0x00000023)
/*! Device has security disable feature enabled. */
#define CC_BSV_SECURE_DISABLE_ERROR                   (CC_BSV_BASE_ERROR + 0x00000024)
/*! Device has Kcst in disabled state */
#define CC_BSV_KCST_DISABLE_ERROR                     (CC_BSV_BASE_ERROR + 0x00000025)


/*! Illegal data-in pointer. */
#define CC_BSV_CRYPTO_INVALID_DATA_IN_POINTER_ERROR         (CC_BSV_CRYPTO_ERROR + 0x00000001)
/*! Illegal data-out pointer. */
#define CC_BSV_CRYPTO_INVALID_DATA_OUT_POINTER_ERROR        (CC_BSV_CRYPTO_ERROR + 0x00000002)
/*! Illegal data size. */
#define CC_BSV_CRYPTO_INVALID_DATA_SIZE_ERROR               (CC_BSV_CRYPTO_ERROR + 0x00000003)
/*! Illegal key type. */
#define CC_BSV_CRYPTO_INVALID_KEY_TYPE_ERROR                (CC_BSV_CRYPTO_ERROR + 0x00000004)
/*! Illegal key size. */
#define CC_BSV_CRYPTO_INVALID_KEY_SIZE_ERROR                (CC_BSV_CRYPTO_ERROR + 0x00000005)
/*! Invalid key pointer. */
#define CC_BSV_CRYPTO_INVALID_KEY_POINTER_ERROR             (CC_BSV_CRYPTO_ERROR + 0x00000006)
/*! Illegal key DMA type. */
#define CC_BSV_CRYPTO_INVALID_KEY_DMA_TYPE_ERROR            (CC_BSV_CRYPTO_ERROR + 0x00000007)
/*! Illegal IV pointer. */
#define CC_BSV_CRYPTO_INVALID_IV_POINTER_ERROR              (CC_BSV_CRYPTO_ERROR + 0x00000008)
/*! Illegal cipher mode. */
#define CC_BSV_CRYPTO_INVALID_CIPHER_MODE_ERROR             (CC_BSV_CRYPTO_ERROR + 0x00000009)
/*! Illegal result buffer pointer. */
#define CC_BSV_CRYPTO_INVALID_RESULT_BUFFER_POINTER_ERROR   (CC_BSV_CRYPTO_ERROR + 0x0000000A)
/*! Invalid DMA type. */
#define CC_BSV_CRYPTO_INVALID_DMA_TYPE_ERROR                (CC_BSV_CRYPTO_ERROR + 0x0000000B)
/*! Invalid in/out buffers overlapping. */
#define CC_BSV_CRYPTO_DATA_OUT_DATA_IN_OVERLAP_ERROR        (CC_BSV_CRYPTO_ERROR + 0x0000000C)
/*! Invalid KDF label size. */
#define CC_BSV_CRYPTO_ILLEGAL_KDF_LABEL_ERROR               (CC_BSV_CRYPTO_ERROR + 0x0000000D)
/*! Invalid KDF Context size. */
#define CC_BSV_CRYPTO_ILLEGAL_KDF_CONTEXT_ERROR             (CC_BSV_CRYPTO_ERROR + 0x0000000E)
/*! Invalid CCM key. */
#define CC_BSV_CCM_INVALID_KEY_ERROR                        (CC_BSV_CRYPTO_ERROR + 0x0000000f)
/*! Invalid CCM Nonce. */
#define CC_BSV_CCM_INVALID_NONCE_ERROR                      (CC_BSV_CRYPTO_ERROR + 0x00000010)
/*! Invalid CCM associated data. */
#define CC_BSV_CCM_INVALID_ASSOC_DATA_ERROR                 (CC_BSV_CRYPTO_ERROR + 0x00000011)
/*! Invalid CCM text data. */
#define CC_BSV_CCM_INVALID_TEXT_DATA_ERROR                  (CC_BSV_CRYPTO_ERROR + 0x00000012)
/*! Invalid CCM-MAC buffer. */
#define CC_BSV_CCM_INVALID_MAC_BUF_ERROR                    (CC_BSV_CRYPTO_ERROR + 0x00000013)
/*! CCM-MAC comparison failed. */
#define CC_BSV_CCM_TAG_LENGTH_ERROR                         (CC_BSV_CRYPTO_ERROR + 0x00000014)
/*! CCM-MAC comparison failed. */
#define CC_BSV_CCM_MAC_INVALID_ERROR                        (CC_BSV_CRYPTO_ERROR + 0x00000015)
/*! Illegal flow mode. */
#define CC_BSV_CRYPTO_INVALID_FLOW_MODE_ERROR               (CC_BSV_CRYPTO_ERROR + 0x00000016)

#ifdef __cplusplus
}
#endif

#endif

/**
@}
 */



