/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TBB_KEY_H_
#define TBB_KEY_H_

#include "key.h"

/*
 * Enumerate the keys that are used to establish the chain of trust
 */
enum {
	ROT_KEY,
	TRUSTED_WORLD_KEY,
	NON_TRUSTED_WORLD_KEY,
	SCP_FW_CONTENT_CERT_KEY,
	SOC_FW_CONTENT_CERT_KEY,
	TRUSTED_OS_FW_CONTENT_CERT_KEY,
	NON_TRUSTED_FW_CONTENT_CERT_KEY
};

#endif /* TBB_KEY_H_ */
