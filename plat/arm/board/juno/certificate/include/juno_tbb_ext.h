/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_TBB_EXT_H
#define JUNO_TBB_EXT_H

#include <tbbr/tbb_ext.h>

/* Juno platform defined TBBR extensions */
enum {
	ETHOSN_NPU_FW_CONTENT_CERT_PK_EXT = FWU_HASH_EXT + 1,
	ETHOSN_NPU_FW_HASH_EXT,
};

#endif /* JUNO_TBB_EXT_H */
