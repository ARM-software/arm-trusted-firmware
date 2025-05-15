/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PDEF_TBB_CERT_H
#define PDEF_TBB_CERT_H

#include <tbbr/tbb_cert.h>

/*
 * Enumerate the certificates that are used to establish the chain of trust
 */
enum {
	DDR_FW_KEY_CERT = FWU_CERT + 1,
	DDR_UDIMM_FW_CONTENT_CERT,
	DDR_RDIMM_FW_CONTENT_CERT
};

#endif /* PDEF_TBB_CERT_H */
