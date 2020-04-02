/*
 * Copyright (c) 2017-2020 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BSV_CRYPTO_ASYM_API_H
#define _BSV_CRYPTO_ASYM_API_H

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This file contains the cryptographic Asymmetric ROM APIs of the Boot Services.

@defgroup cc_bsv_crypto_asym_api CryptoCell Boot Services cryptographic Asymmetric ROM APIs
@{
@ingroup cc_bsv
*/

#include "cc_pal_types.h"
#include "cc_pka_hw_plat_defs.h"
#include "cc_sec_defs.h"
#include "bsv_crypto_api.h"

/*! Defines the workspace size in bytes needed for internal Asymmetric operations. */
#define BSV_RSA_WORKSPACE_MIN_SIZE (4*BSV_CERT_RSA_KEY_SIZE_IN_BYTES +\
                                    2*RSA_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_BYTES)

/*! Definition for the RSA public modulus array. */
typedef uint32_t CCBsvNBuff_t[BSV_CERT_RSA_KEY_SIZE_IN_WORDS];

/*! Definition for the RSA Barrett mod tag array. */
typedef uint32_t CCBsvNpBuff_t[RSA_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_BYTES];

/*! Definition for the RSA signature array. */
typedef uint32_t CCBsvSignature_t[BSV_CERT_RSA_KEY_SIZE_IN_WORDS];


/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/

/*!
@brief This function performs the primitive operation of RSA, meaning exponent and modulus.
    outBuff = (pInBuff ^ Exp) mod NBuff. ( Exp = 0x10001 )

    The function supports 2k and 3K bit size of modulus, based on compile time define.
    There are no restriction on pInBuff location, however its size must be equal to BSV_RSA_KEY_SIZE_IN_BYTES and its
    value must be smaller than the modulus.


@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvRsaPrimVerify (unsigned long hwBaseAddress, /*!< [in] The base address of the CryptoCell HW registers. */
                                CCBsvNBuff_t NBuff,         /*!< [in] The modulus buffer big endian format. */
                                CCBsvNpBuff_t NpBuff,       /*!< [in] The barret tag buffer big endian format - optional. */
                                uint32_t *pInBuff,          /*!< [in] The DataIn buffer to be encrypted. */
                                size_t inBuffSize,          /*!< [in] The DataIn buffer size in bytes, must be BSV_RSA_KEY_SIZE_IN_BYTES. */
                                CCBsvSignature_t pOutBuff, /*!< [out] The encrypted buffer in big endian format. */
                                uint32_t *pWorkSpace,       /*!< [in] The pointer to user allocated buffer for internal use. */
                                size_t  workBufferSize      /*!< [in] The size in bytes of pWorkSpace, must be at-least BSV_RSA_WORKSPACE_MIN_SIZE. */
);


/*!
@brief This function performs RSA PSS verify.

    The function should support 2k and 3K bit size of modulus, based on compile time define.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvRsaPssVerify (unsigned long hwBaseAddress,  /*!< [in] The base address of the CryptoCell HW registers. */
                                CCBsvNBuff_t NBuff,         /*!< [in] The modulus buffer big endian format. */
                                CCBsvNpBuff_t NpBuff,       /*!< [in] The barret tag buffer big endian format - optional. */
                                CCBsvSignature_t signature, /*!< [in] The signature buffer to verify - big endian format. */
                                CCHashResult_t hashedData,  /*!< [in] The data-in buffer to be verified as sha256 digest. */
                                uint32_t *pWorkSpace,       /*!< [in] The pointer to user allocated buffer for internal use. */
                                size_t  workBufferSize,     /*!< [in] The size in bytes of pWorkSpace, must be at-least BSV_RSA_WORKSPACE_MIN_SIZE. */
                                CCBool_t    *pIsVerified    /*!< [out] The flag indicates whether the signature is verified or not.
                                                                         If verified value will be CC_TRUE, otherwise CC_FALSE */
);



#ifdef __cplusplus
}
#endif

#endif

/**
@}
 */

