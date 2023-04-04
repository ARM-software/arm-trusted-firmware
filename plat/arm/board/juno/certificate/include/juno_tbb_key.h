/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_TBB_KEY_H
#define JUNO_TBB_KEY_H

#include <tbbr/tbb_key.h>

/*
 * Juno platform keys that are used to establish the COT
 */
enum {
	ETHOSN_NPU_FW_CONTENT_CERT_KEY =
		NON_TRUSTED_FW_CONTENT_CERT_KEY + 1,
};
#endif /* JUNO_TBB_KEY_H */
