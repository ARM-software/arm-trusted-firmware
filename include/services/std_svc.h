/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STD_SVC_H
#define STD_SVC_H

/* SMC function IDs for Standard Service queries */

#define ARM_STD_SVC_CALL_COUNT		0x8400ff00
#define ARM_STD_SVC_UID			0x8400ff01
/*					0x8400ff02 is reserved */
#define ARM_STD_SVC_VERSION		0x8400ff03

/* ARM Standard Service Calls version numbers */
#define STD_SVC_VERSION_MAJOR		0x0
#define STD_SVC_VERSION_MINOR		0x1

/*
 * Get the ARM Standard Service argument from EL3 Runtime.
 * This function must be implemented by EL3 Runtime and the
 * `svc_mask` identifies the service. `svc_mask` is a bit
 * mask identifying the range of SMC function IDs available
 * to the service.
 */
uintptr_t get_arm_std_svc_args(unsigned int svc_mask);

#endif /* STD_SVC_H */
