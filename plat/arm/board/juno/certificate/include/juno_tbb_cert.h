/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_TBB_CERT_H
#define JUNO_TBB_CERT_H

#include <tbbr/tbb_cert.h>

/*
 * Juno platform certificates that are used to establish the COT
 */
enum {
	ETHOSN_NPU_FW_KEY_CERT = FWU_CERT + 1,
	ETHOSN_NPU_FW_CONTENT_CERT,
};

#endif /* JUNO_TBB_CERT_H */
