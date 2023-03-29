/*
 * Copyright (c) 2014-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Copyright (c) 2014, Linaro Limited. All rights reserved. */

#ifndef TEESMC_OPTEED_H
#define TEESMC_OPTEED_H

#include "teesmc_opteed_macros.h"

/*
 * This section specifies SMC function IDs used when returning from TEE to the
 * secure monitor.
 *
 * All SMC Function IDs indicates SMC32 Calling Convention but will carry
 * full 64 bit values in the argument registers if invoked from Aarch64
 * mode. This violates the SMC Calling Convention, but since this
 * convention only coveres API towards Normal World it's something that
 * only concerns the OP-TEE Dispatcher in Trusted Firmware-A and OP-TEE
 * OS at Secure EL1.
 */

/*
 * Issued when returning from initial entry.
 *
 * Register usage:
 * r0/x0	SMC Function ID, TEESMC_OPTEED_RETURN_ENTRY_DONE
 * r1/x1	Pointer to entry vector
 */
#define TEESMC_OPTEED_FUNCID_RETURN_ENTRY_DONE		0
#define TEESMC_OPTEED_RETURN_ENTRY_DONE \
	TEESMC_OPTEED_RV(TEESMC_OPTEED_FUNCID_RETURN_ENTRY_DONE)



/*
 * Issued when returning from "cpu_on" vector
 *
 * Register usage:
 * r0/x0	SMC Function ID, TEESMC_OPTEED_RETURN_ON_DONE
 * r1/x1	0 on success and anything else to indicate error condition
 */
#define TEESMC_OPTEED_FUNCID_RETURN_ON_DONE		1
#define TEESMC_OPTEED_RETURN_ON_DONE \
	TEESMC_OPTEED_RV(TEESMC_OPTEED_FUNCID_RETURN_ON_DONE)

/*
 * Issued when returning from "cpu_off" vector
 *
 * Register usage:
 * r0/x0	SMC Function ID, TEESMC_OPTEED_RETURN_OFF_DONE
 * r1/x1	0 on success and anything else to indicate error condition
 */
#define TEESMC_OPTEED_FUNCID_RETURN_OFF_DONE		2
#define TEESMC_OPTEED_RETURN_OFF_DONE \
	TEESMC_OPTEED_RV(TEESMC_OPTEED_FUNCID_RETURN_OFF_DONE)

/*
 * Issued when returning from "cpu_suspend" vector
 *
 * Register usage:
 * r0/x0	SMC Function ID, TEESMC_OPTEED_RETURN_SUSPEND_DONE
 * r1/x1	0 on success and anything else to indicate error condition
 */
#define TEESMC_OPTEED_FUNCID_RETURN_SUSPEND_DONE	3
#define TEESMC_OPTEED_RETURN_SUSPEND_DONE \
	TEESMC_OPTEED_RV(TEESMC_OPTEED_FUNCID_RETURN_SUSPEND_DONE)

/*
 * Issued when returning from "cpu_resume" vector
 *
 * Register usage:
 * r0/x0	SMC Function ID, TEESMC_OPTEED_RETURN_RESUME_DONE
 * r1/x1	0 on success and anything else to indicate error condition
 */
#define TEESMC_OPTEED_FUNCID_RETURN_RESUME_DONE		4
#define TEESMC_OPTEED_RETURN_RESUME_DONE \
	TEESMC_OPTEED_RV(TEESMC_OPTEED_FUNCID_RETURN_RESUME_DONE)

/*
 * Issued when returning from "std_smc" or "fast_smc" vector
 *
 * Register usage:
 * r0/x0	SMC Function ID, TEESMC_OPTEED_RETURN_CALL_DONE
 * r1-4/x1-4	Return value 0-3 which will passed to normal world in
 *		r0-3/x0-3
 */
#define TEESMC_OPTEED_FUNCID_RETURN_CALL_DONE		5
#define TEESMC_OPTEED_RETURN_CALL_DONE \
	TEESMC_OPTEED_RV(TEESMC_OPTEED_FUNCID_RETURN_CALL_DONE)

/*
 * Issued when returning from "fiq" vector
 *
 * Register usage:
 * r0/x0	SMC Function ID, TEESMC_OPTEED_RETURN_FIQ_DONE
 */
#define TEESMC_OPTEED_FUNCID_RETURN_FIQ_DONE		6
#define TEESMC_OPTEED_RETURN_FIQ_DONE \
	TEESMC_OPTEED_RV(TEESMC_OPTEED_FUNCID_RETURN_FIQ_DONE)

/*
 * Issued when returning from "system_off" vector
 *
 * Register usage:
 * r0/x0	SMC Function ID, TEESMC_OPTEED_RETURN_SYSTEM_OFF_DONE
 */
#define TEESMC_OPTEED_FUNCID_RETURN_SYSTEM_OFF_DONE	7
#define TEESMC_OPTEED_RETURN_SYSTEM_OFF_DONE \
	TEESMC_OPTEED_RV(TEESMC_OPTEED_FUNCID_RETURN_SYSTEM_OFF_DONE)

/*
 * Issued when returning from "system_reset" vector
 *
 * Register usage:
 * r0/x0	SMC Function ID, TEESMC_OPTEED_RETURN_SYSTEM_RESET_DONE
 */
#define TEESMC_OPTEED_FUNCID_RETURN_SYSTEM_RESET_DONE	8
#define TEESMC_OPTEED_RETURN_SYSTEM_RESET_DONE \
	TEESMC_OPTEED_RV(TEESMC_OPTEED_FUNCID_RETURN_SYSTEM_RESET_DONE)

/*
 * This section specifies SMC function IDs used when the secure monitor is
 * invoked from the non-secure world.
 */

/*
 * Load OP-TEE image from the payload specified in the registers.
 *
 * WARNING: Use this cautiously as it could lead to insecure loading of the
 * Trusted OS. Further details are in opteed.mk.
 *
 * Call register usage:
 * x0 SMC Function ID, OPTEE_SMC_CALL_LOAD_IMAGE
 * x1 Upper 32bit of a 64bit size for the payload
 * x2 Lower 32bit of a 64bit size for the payload
 * x3 Upper 32bit of the physical address for the payload
 * x4 Lower 32bit of the physical address for the payload
 *
 * The payload consists of a optee_header struct that contains optee_image
 * structs in a flex array, immediately following that in memory is the data
 * referenced by the optee_image structs.
 * Example:
 *
 * struct optee_header (with n images specified)
 * image 0 data
 * image 1 data
 * ...
 * image n-1 data
 *
 * Returns 0 on success and an error code otherwise.
 */
#define NSSMC_OPTEED_FUNCID_LOAD_IMAGE 2
#define NSSMC_OPTEED_CALL_LOAD_IMAGE \
	NSSMC_OPTEED_CALL(NSSMC_OPTEED_FUNCID_LOAD_IMAGE)

/*
 * Returns the UID of the OP-TEE image loading service if image loading is
 * enabled and the image had not been loaded yet. Otherwise this call will be
 * passed through to OP-TEE where it will return the OP-TEE UID.
 */
#define NSSMC_OPTEED_FUNCID_CALLS_UID 0xFF01
#define NSSMC_OPTEED_CALL_UID \
	NSSMC_OPTEED_CALL(NSSMC_OPTEED_FUNCID_CALLS_UID)

#endif /*TEESMC_OPTEED_H*/
