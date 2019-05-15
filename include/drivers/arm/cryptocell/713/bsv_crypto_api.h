/*
 * Copyright (c) 2017-2020 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BSV_CRYPTO_API_H
#define _BSV_CRYPTO_API_H

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This file contains the cryptographic ROM APIs of the Boot Services.

@defgroup cc_bsv_crypto_api CryptoCell Boot Services cryptographic ROM APIs
@{
@ingroup cc_bsv
*/

#include "cc_pal_types.h"
#include "cc_sec_defs.h"
#include "cc_address_defs.h"
#include "bsv_crypto_defs.h"

/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/

/*!
@brief This function calculates the SHA-256 digest over contiguous memory
in an integrated operation.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvSha256(
    unsigned long   hwBaseAddress,  /*!< [in] The base address of the CryptoCell HW registers. */
    uint8_t        *pDataIn,        /*!< [in] A pointer to the input buffer to be hashed. The buffer must be contiguous. */
    size_t          dataSize,       /*!< [in] The size of the data to be hashed, in bytes. */
    CCHashResult_t  hashBuff        /*!< [out]  A pointer to a word-aligned 32-byte buffer. */
    );


/*!
@brief This function allows you to calculate SHA256 digest of an image with decryption base on AES-CTR,
with HW or user key.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure. (in this case, hashBuff will be returned clean, while the output data should be cleaned by the user).
*/
CCError_t CC_BsvCryptoImageDecrypt( unsigned long      hwBaseAddress,   /*!< [in] The base address of the CryptoCell HW registers. */
                                    CCBsvflowMode_t    flow,            /*!< [in] The supported operations are: HASH, AES to HASH, AES and HASH. */
                                    CCBsvKeyType_t     keyType,         /*!< [in] The key type to use: Kce, Kceicv, or user key. */
                                    uint8_t           *pUserKey,        /*!< [in] A pointer to the user key buffer in case keyType is CC_BSV_USER_KEY. */
                                    size_t             userKeySize,     /*!< [in] The user key size in bytes (128bits) in case keyType is CC_BSV_USER_KEY. */
                                    uint8_t           *pIvBuf,          /*!< [in] A pointer to the IV / counter buffer. */
                                    uint8_t           *pInputData,      /*!< [in] A pointer to the input data. */
                                    uint8_t           *pOutputData,     /*!< [out] A pointer to the output buffer. (optional – should be null in case of hash only). */
                                    size_t             dataSize,        /*!< [in] The size of the input data in bytes. MUST be multiple of AES block size. */
                                    CCHashResult_t     hashBuff         /*!< [out] A pointer to a word-aligned 32-byte digest output buffer. */
                                    );

#ifdef __cplusplus
}
#endif

#endif

/**
@}
 */

