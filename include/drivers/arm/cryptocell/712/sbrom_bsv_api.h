/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SBROM_BSV_API_H
#define _SBROM_BSV_API_H

#ifdef __cplusplus
extern "C"
{
#endif

/*! @file
@brief This file contains all SBROM library APIs and definitions.
*/
#include "cc_pal_types.h"

/* Life cycle state definitions */
#define CC_BSV_CHIP_MANUFACTURE_LCS		0x0 /*!< CM lifecycle value. */
#define CC_BSV_DEVICE_MANUFACTURE_LCS		0x1 /*!< DM lifecycle value. */
#define CC_BSV_SECURITY_DISABLED_LCS		0x3 /*!< SD lifecycle value. */
#define CC_BSV_SECURE_LCS			0x5 /*!< Secure lifecycle value. */
#define CC_BSV_RMA_LCS				0x7 /*!< RMA lifecycle value. */

/*----------------------------
      PUBLIC FUNCTIONS
-----------------------------------*/

/*!
@brief This function should be the first ARM TrustZone CryptoCell TEE SBROM library API called.
It verifies the HW product and version numbers.

@return CC_OK	On success.
@return A non-zero value from sbrom_bsv_error.h on failure.
*/
CCError_t CC_BsvSbromInit(
	unsigned long hwBaseAddress 	/*!< [in] HW registers base address. */
	);


/*!
@brief This function can be used for checking the LCS value, after CC_BsvLcsGetAndInit was called by the Boot ROM.

@return CC_OK	On success.
@return A non-zero value from sbrom_bsv_error.h on failure.
*/
CCError_t CC_BsvLcsGet(
	unsigned long hwBaseAddress,	/*!< [in] HW registers base address. */
	uint32_t *pLcs			/*!< [out] Returned lifecycle state. */
	);

/*!
@brief This function retrieves the HW security lifecycle state, performs validity checks,
and additional initializations in case the LCS is RMA (sets the Kce to fixed value).
\note	Invalid LCS results in an error returned.
In this case, the customer's code must completely disable the device.

@return CC_OK	On success.
@return A non-zero value from sbrom_bsv_error.h on failure.
*/
CCError_t CC_BsvLcsGetAndInit(
	unsigned long hwBaseAddress,	/*!< [in] HW registers base address. */
	uint32_t *pLcs		/*!< [out] Returned lifecycle state. */
	);

#ifdef __cplusplus
}
#endif

#endif
