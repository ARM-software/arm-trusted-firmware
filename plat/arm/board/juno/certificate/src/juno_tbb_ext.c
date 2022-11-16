/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ethosn_cert.h>

#include <juno_tbb_ext.h>
#include <juno_tbb_key.h>

static ext_t juno_plat_tbb_extensions[] = {
	ETHOSN_NPU_FW_CONTENT_CERT_PK_EXT_DEF,
	ETHOSN_NPU_FW_HASH_EXT_DEF,
};

PLAT_REGISTER_EXTENSIONS(juno_plat_tbb_extensions);
