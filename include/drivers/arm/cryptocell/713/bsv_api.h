/*
 * Copyright (c) 2017-2020 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BSV_API_H
#define _BSV_API_H

#ifdef __cplusplus
extern "C"
{
#endif

/*!
@file
@brief This file contains the Boot Services APIs and definitions.

@defgroup cc_bsv_api CryptoCell Boot Services APIs and definitions
@{
@ingroup cc_bsv
*/

#include "cc_pal_types.h"
#include "cc_sec_defs.h"
#include "cc_boot_defs.h"

/* Life cycle state definitions. */
#define CC_BSV_CHIP_MANUFACTURE_LCS    0x0 /*!< The CM life-cycle state (LCS) value. */
#define CC_BSV_DEVICE_MANUFACTURE_LCS  0x1 /*!< The DM life-cycle state (LCS) value. */
#define CC_BSV_SECURE_LCS              0x5 /*!< The Secure life-cycle state (LCS) value. */
#define CC_BSV_RMA_LCS                 0x7 /*!< The RMA life-cycle state (LCS) value. */
#define CC_BSV_INVALID_LCS             0xff /*!< The invalid life-cycle state (LCS) value. */

/*----------------------------
      TYPES
-----------------------------------*/

/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/


/*!
@brief This function verifies the product and version numbers of the HW, and initializes it.

\warning This function must be the first CryptoCell-7xx SBROM library API called.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvInit(
    unsigned long hwBaseAddress     /*!< [in] The base address of the CryptoCell HW registers. */
    );

/*!
@brief This function retrieves the HW LCS and performs validity checks.

If the LCS is RMA, it also sets the OTP secret keys to a fixed value.

@note An error is returned if there is an invalid LCS. If this happens, your code must
completely disable the device.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvGetAndInitLcs(
    unsigned long hwBaseAddress,    /*!< [in] The base address of the CryptoCell HW registers. */
    uint32_t *pLcs                  /*!< [out] The value of the current LCS. */
    );

/*!
@brief This function retrieves the LCS from the NVM manager.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvLcsGet(
    unsigned long hwBaseAddress,    /*!< [in] The base address of the CryptoCell HW registers. */
    uint32_t *pLcs                  /*!< [out] The value of the current LCS. */
    );

/*!
@brief This function reads software revocation counter from OTP memory, according to the provided sw version index.
SW version is stored in NVM counter and represented by ones. Meaning seVersion=5 would be stored as binary 0b11111;
hence:
    the maximal of trusted is 32
    the maximal of non-trusted is 224

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvSwVersionGet(
    unsigned long hwBaseAddress,        /*!< [in] HW registers base address. */
    CCSbSwVersionId_t id,               /*!< [in] Enumeration defining the trusted/non-trusted counter to read. */
    uint32_t *swVersion                 /*!< [out] The value of the requested counter as read from OTP memory. */
    );

/*!
@brief This function sets the NVM counter according to swVersionID (trusted/non-trusted).

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvSwVersionSet(
    unsigned long hwBaseAddress,        /*!< [in] HW registers base address. */
    CCSbSwVersionId_t id,               /*!< [in] Enumeration defining the trusted/non-trusted counter to read. */
    uint32_t swVersion                  /*!< [in] New value of the counter to be programmed in OTP memory. */
    );

/*!
@brief This function sets the "fatal error" flag in the NVM manager, to disable the use of
any HW keys or security services.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvFatalErrorSet(
    unsigned long hwBaseAddress         /*!< [in] The base address of the CryptoCell HW registers. */
    );

/*!
@brief This function retrieves the public key hash from OTP memory, according to the provided index.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvPubKeyHashGet(
    unsigned long hwBaseAddress,        /*!< [in] HW registers base address. */
    CCSbPubKeyIndexType_t keyIndex,     /*!< [in] Enumeration defining the key hash to retrieve: 128-bit HBK0, 128-bit HBK1, or 256-bit HBK. */
    uint32_t *hashedPubKey,             /*!< [out] A buffer to contain the public key HASH. */
    uint32_t hashResultSizeWords        /*!< [in] The size of the hash in 32-bit words:
                            - Must be 4 for 128-bit hash.
                            - Must be 8 for 256bit hash. */
    );

/*!
@brief This function permanently sets the RMA LCS for the ICV and the OEM.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvRMAModeEnable(
    unsigned long hwBaseAddress         /*!< [in] The base address of the CryptoCell HW registers. */
    );

/*!
@brief This function is called by the ICV code, to disable the OEM code from changing the ICV RMA bit flag.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvICVRMAFlagBitLock(
    unsigned long hwBaseAddress         /*!< [in] The base address of the CryptoCell HW registers. */
    );

/*!
@brief This function locks the defined ICV class keys from further usage.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvICVKeyLock(
    unsigned long hwBaseAddress,        /*!< [in] HW registers base address. */
    CCBool_t isICVProvisioningKeyLock,  /*!< [in] Should the provisioning key be locked. */
    CCBool_t isICVCodeEncKeyLock        /*!< [in] Should the encryption key be locked. */
    );


/*!
@brief This function retrieves the value of "secure disable" bit.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvSecureDisableGet(
    unsigned long hwBaseAddress,        /*!< [in] HW registers base address. */
    CCBool_t *isSDEnabled               /*!< [out] The value of the SD Enable bit. */
    );


/*!
@brief This function derives the platform key (Kplt) from the Kpicv, and then decrypts the customer key (Kcst)
from the EKcst (burned in the OTP). The decryption is done only in Secure and RMA LCS mode using AES-ECB.
The customer ROM should invoke this function during early boot, prior to running any non-ROM code, only if Kcst exists.
The resulting Kcst is saved in a HW register.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvCustomerKeyDecrypt(
    unsigned long hwBaseAddress         /*!< [in] The base address of the CryptoCell HW registers. */
    );
#ifdef __cplusplus
}
#endif

/*!
@brief This function derives the unique SoC_ID for the device, as hashed (Hbk || AES_CMAC (HUK)).

@note SoC_ID is required to create debug certificates.

The OEM or ICV must provide a method for a developer to discover the SoC_ID of a target
device without having to first enable debugging.
One suggested implementation is to have the device ROM code compute the SoC_ID and place
it in a specific location in the flash memory, from where it can be accessed by the developer.

@return \c CC_OK on success.
@return A non-zero value from bsv_error.h on failure.
*/
CCError_t CC_BsvSocIDCompute(
    unsigned long hwBaseAddress, /*!< [in] The base address of the CryptoCell HW registers. */
    CCHashResult_t hashResult    /*!< [out] The derived SoC_ID. */
    );

#endif /* _BSV_API_H */

/**
@}
 */

