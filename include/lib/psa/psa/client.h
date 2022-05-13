
/*
 * Copyright (c) 2018-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PSA_CLIENT_H
#define PSA_CLIENT_H

#include <stddef.h>
#include <stdint.h>

#include <psa/error.h>

#ifndef IOVEC_LEN
#define IOVEC_LEN(arr) ((uint32_t)(sizeof(arr)/sizeof(arr[0])))
#endif
/*********************** PSA Client Macros and Types *************************/
/**
 * The version of the PSA Framework API that is being used to build the calling
 * firmware. Only part of features of FF-M v1.1 have been implemented. FF-M v1.1
 * is compatible with v1.0.
 */
#define PSA_FRAMEWORK_VERSION	(0x0101u)
/**
 * Return value from psa_version() if the requested RoT Service is not present
 * in the system.
 */
#define PSA_VERSION_NONE	(0u)
/**
 * The zero-value null handle can be assigned to variables used in clients and
 * RoT Services, indicating that there is no current connection or message.
 */
#define PSA_NULL_HANDLE		((psa_handle_t)0)
/**
 * Tests whether a handle value returned by psa_connect() is valid.
 */
#define PSA_HANDLE_IS_VALID(handle)	((psa_handle_t)(handle) > 0)
/**
 * Converts the handle value returned from a failed call psa_connect() into
 * an error code.
 */
#define PSA_HANDLE_TO_ERROR(handle)	((psa_status_t)(handle))
/**
 * Maximum number of input and output vectors for a request to psa_call().
 */
#define PSA_MAX_IOVEC		(4u)
/**
 * An IPC message type that indicates a generic client request.
 */
#define PSA_IPC_CALL		(0)
typedef int32_t psa_handle_t;
/**
 * A read-only input memory region provided to an RoT Service.
 */
typedef struct psa_invec {
	const void *base;	/*!< the start address of the memory buffer */
	size_t len;		/*!< the size in bytes                      */
} psa_invec;
/**
 * A writable output memory region provided to an RoT Service.
 */
typedef struct psa_outvec {
	void *base;		/*!< the start address of the memory buffer */
	size_t len;		/*!< the size in bytes                      */
} psa_outvec;

/**
 * Call an RoT Service on an established connection.
 *
 * handle	A handle to an established connection.
 * type		The request type. Must be zero(PSA_IPC_CALL) or positive.
 * in_vec	Array of input psa_invec structures.
 * in_len	Number of input psa_invec structures.
 * out_vec	Array of output psa_outvec structures.
 * out_len	Number of output psa_outvec structures.
 *
 * Return value >=0	RoT Service-specific status value.
 * Return value <0	RoT Service-specific error code.
 *
 * PSA_ERROR_PROGRAMMER_ERROR:
 *	- The connection has been terminated by the RoT Service.
 *
 * The call is a PROGRAMMER ERROR if one or more of the following are true:
 *	- An invalid handle was passed.
 *	- The connection is already handling a request.
 *	- type < 0.
 *	- An invalid memory reference was provided.
 *	- in_len + out_len > PSA_MAX_IOVEC.
 *	- The message is unrecognized by the RoT.
 *	- Service or incorrectly formatted.
 */
psa_status_t psa_call(psa_handle_t handle,
		      int32_t type,
		      const psa_invec *in_vec,
		      size_t in_len,
		      psa_outvec *out_vec,
		      size_t out_len);

#endif /* PSA_CLIENT_H */
