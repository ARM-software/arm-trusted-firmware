/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ethosn_cert.h>

#include <juno_tbb_cert.h>
#include <juno_tbb_ext.h>
#include <juno_tbb_key.h>

static cert_t juno_plat_tbb_certificates[] = {
	ETHOSN_NPU_FW_KEY_CERT_DEF,
	ETHOSN_NPU_FW_CONTENT_CERT_DEF,
};

PLAT_REGISTER_COT(juno_plat_tbb_certificates);
