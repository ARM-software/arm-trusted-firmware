/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CRYPTO_DRIVER_H
#define _CRYPTO_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cc_pal_sb_plat.h"
#include "cc_sec_defs.h"

/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/
/*!
 * @brief This function gives the functionality of integrated hash
 *
 * @param[in] hwBaseAddress	- CryptoCell base address
 * @param[out] hashResult	- the HASH result.
 *
 */
CCError_t SBROM_CryptoHash(unsigned long hwBaseAddress, CCDmaAddr_t inputDataAddr, uint32_t BlockSize,
				CCHashResult_t hashResult);

#ifdef __cplusplus
}
#endif

#endif
