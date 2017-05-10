/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SECURE_BOOT_BASE_FUNC_H
#define _SECURE_BOOT_BASE_FUNC_H


#ifdef __cplusplus
extern "C"
{
#endif

#include "cc_pal_types.h"
#include "secureboot_gen_defs.h"


/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/

/**
 * @brief This function calculates the HASH over the given data and than verify
 *	  RSA signature on that hashed data
 *
 * @param[in] hwBaseAddr -  CryptoCell base address
 * @param[in] pData - pointer to the data to be verified
 * @param[in] pNParams - a pointer to the public key parameters
 * @param[in] pSignature - a pointer to the signature structure
 * @param[in] sizeOfData - size of the data to calculate the HASH on (in bytes)
 * @param[in] RSAAlg - RSA algorithm to use
 *
 * @return CCError_t - On success the value CC_OK is returned,
 *         on failure - a value from BootImagesVerifier_error.h
 */
CCError_t CCSbVerifySignature(unsigned long hwBaseAddress,
				uint32_t *pData,
				CCSbNParams_t *pNParams,
				CCSbSignature_t *pSignature,
				uint32_t sizeOfData,
				CCSbRsaAlg_t RSAAlg);

#ifdef __cplusplus
}
#endif

#endif
