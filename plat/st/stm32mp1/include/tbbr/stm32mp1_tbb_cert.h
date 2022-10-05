/*
 * Copyright (c) 2015-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_TBB_CERT_H
#define STM32MP1_TBB_CERT_H

#include <tbbr/tbb_cert.h>

/*
 * Enumerate the certificates that are used to establish the chain of trust
 */
enum {
	STM32MP_CONFIG_CERT = FWU_CERT + 1
};

#endif /* STM32MP1_TBB_CERT_H */
