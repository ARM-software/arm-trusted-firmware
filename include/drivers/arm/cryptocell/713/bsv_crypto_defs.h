/*
 * Copyright (c) 2017-2020 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BSV_CRYPTO_DEFS_H
#define _BSV_CRYPTO_DEFS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This file contains the definitions of the cryptographic ROM APIs.

@defgroup cc_bsv_crypto_defs CryptoCell Boot Services cryptographic ROM API definitions
@{
@ingroup cc_bsv
*/

/*! AES supported HW key code table. */
typedef enum {

    CC_BSV_USER_KEY = 0,        /*!< Definition for a user key. */
    CC_BSV_HUK_KEY = 1,         /*!< Definition for the HW unique key. */
    CC_BSV_RTL_KEY = 2,         /*!< Definition for the RTL key. */
    CC_BSV_SESSION_KEY = 3,     /*!< Definition for the Session key. */
    CC_BSV_CE_KEY = 4,          /*!< Definition for the Kce. */
    CC_BSV_PLT_KEY = 5,         /*!< Definition for the Platform key. */
    CC_BSV_KCST_KEY = 6,        /*!< Definition for Kcst. */
    CC_BSV_ICV_PROV_KEY = 0xd,  /*!< Definition for the Kpicv. */
    CC_BSV_ICV_CE_KEY = 0xe,    /*!< Definition for the Kceicv. */
    CC_BSV_PROV_KEY = 0xf,      /*!< Definition for the Kcp. */
    CC_BSV_END_OF_KEY_TYPE = INT32_MAX, /*!< Reserved. */
}CCBsvKeyType_t;

/*! AES directions. */
typedef enum bsvAesDirection {
    BSV_AES_DIRECTION_ENCRYPT = 0, /*!< Encrypt.*/
    BSV_AES_DIRECTION_DECRYPT = 1, /*!< Decrypt.*/
    BSV_AES_NUM_OF_ENCRYPT_MODES,  /*!< The maximal number of operations. */
    BSV_AES_DIRECTION_RESERVE32B = INT32_MAX /*!< Reserved.*/
}bsvAesDirection_t;

/*! Definitions of the cryptographic flow supported as part of the Secure Boot. */
typedef enum {
    CC_BSV_CRYPTO_HASH_MODE     = 0,            /*!< Hash mode only. */
    CC_BSV_CRYPTO_AES_CTR_AND_HASH_MODE  = 1,   /*!< Data goes into the AES and Hash engines. */
    CC_BSV_CRYPTO_AES_CTR_TO_HASH_MODE = 2      /*!< Data goes into the AES and from the AES to the Hash engine. */
}CCBsvflowMode_t;

/*! CryptoImage HW completion sequence mode */
typedef enum
{
    BSV_CRYPTO_COMPLETION_NO_WAIT = 0, /*!< The driver waits only before reading the output. */
    BSV_CRYPTO_COMPLETION_WAIT_UPON_END = 1 /*!< The driver waits after each chunk of data. */
}bsvCryptoCompletionMode_t;


/*! AES-CMAC result size, in words. */
#define CC_BSV_CMAC_RESULT_SIZE_IN_WORDS    4  /* 128b */
/*! AES-CMAC result size, in bytes. */
#define CC_BSV_CMAC_RESULT_SIZE_IN_BYTES    16 /* 128b */
/*! AES-CCM 128bit key size, in bytes. */
#define CC_BSV_CCM_KEY_SIZE_BYTES               16
/*! AES-CCM 128bit key size, in words. */
#define CC_BSV_CCM_KEY_SIZE_WORDS               4
/*! AES-CCM NONCE size, in bytes. */
#define CC_BSV_CCM_NONCE_SIZE_BYTES     12


/*! AES-CMAC result buffer. */
typedef uint32_t CCBsvCmacResult_t[CC_BSV_CMAC_RESULT_SIZE_IN_WORDS];
/*! AES-CCM key buffer.*/
typedef uint32_t CCBsvCcmKey_t[CC_BSV_CCM_KEY_SIZE_WORDS];
/*! AES-CCM nonce buffer.*/
typedef uint8_t CCBsvCcmNonce_t[CC_BSV_CCM_NONCE_SIZE_BYTES];
/*! AES-CCM MAC buffer.*/
typedef uint8_t CCBsvCcmMacRes_t[CC_BSV_CMAC_RESULT_SIZE_IN_BYTES];


#ifdef __cplusplus
}
#endif

#endif

/**
@}
 */

