/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _NVM_OTP_H
#define _NVM_OTP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cc_crypto_boot_defs.h"
#include "cc_pal_types.h"

/*------------------------------------
    DEFINES
-------------------------------------*/



/**
 * @brief The NVM_GetSwVersion function is a NVM interface function -
 *        The function retrieves the SW version from the SRAM/NVM.
 *        In case of OTP, we support up to 16 anti-rollback counters (taken from the certificate)
 *
 * @param[in] hwBaseAddress -  CryptoCell base address
 *
 * @param[in] counterId -  relevant only for OTP (valid values: 1,2)
 *
 * @param[out] swVersion   -  the minimum SW version
 *
 * @return CCError_t - On success the value CC_OK is returned, and on failure   -a value from NVM_error.h
 */
CCError_t NVM_GetSwVersion(unsigned long hwBaseAddress, CCSbSwVersionId_t counterId, uint32_t *swVersion);


/**
 * @brief The NVM_SetSwVersion function is a NVM interface function -
 *        The function writes the SW version into the SRAM/NVM.
 *        In case of OTP, we support up to 16 anti-rollback counters (taken from the certificate)
 *
 * @param[in] hwBaseAddress -  CryptoCell base address
 *
 * @param[in] counterId -  relevant only for OTP (valid values: 1,2)
 *
 * @param[in] swVersion   -  the minimum SW version
 *
 * @return CCError_t - On success the value CC_OK is returned, and on failure   -a value from NVM_error.h
 */
CCError_t NVM_SetSwVersion(unsigned long hwBaseAddress, CCSbSwVersionId_t counterId, uint32_t swVersion);

#ifdef __cplusplus
}
#endif

#endif
