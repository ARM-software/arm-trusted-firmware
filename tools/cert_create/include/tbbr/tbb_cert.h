/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TBB_CERT_H
#define TBB_CERT_H

#include "cert.h"

/*
 * Enumerate the certificates that are used to establish the chain of trust
 */
enum {
	TRUSTED_BOOT_FW_CERT,
	TRUSTED_KEY_CERT,
	SCP_FW_KEY_CERT,
	SCP_FW_CONTENT_CERT,
	SOC_FW_KEY_CERT,
	SOC_FW_CONTENT_CERT,
	TRUSTED_OS_FW_KEY_CERT,
	TRUSTED_OS_FW_CONTENT_CERT,
	NON_TRUSTED_FW_KEY_CERT,
	NON_TRUSTED_FW_CONTENT_CERT,
	SIP_SECURE_PARTITION_CONTENT_CERT,
	FWU_CERT
};

#endif /* TBB_CERT_H */
