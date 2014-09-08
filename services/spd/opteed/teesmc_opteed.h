/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Copyright (c) 2014, Linaro Limited. All rights reserved. */

#ifndef TEESMC_OPTEED_H
#define TEESMC_OPTEED_H

/*
 * This file specify SMC function IDs used when returning from TEE to the
 * secure monitor.
 *
 * All SMC Function IDs indicates SMC32 Calling Convention but will carry
 * full 64 bit values in the argument registers if invoked from Aarch64
 * mode. This violates the SMC Calling Convention, but since this
 * convention only coveres API towards Normwal World it's something that
 * only concerns the OP-TEE Dispatcher in ARM Trusted Firmware and OP-TEE
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

#endif /*TEESMC_OPTEED_H*/
