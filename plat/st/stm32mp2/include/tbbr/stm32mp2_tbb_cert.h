/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP2_TBB_CERT_H
#define STM32MP2_TBB_CERT_H

#include <tbbr/tbb_cert.h>
#include <tbbr/tbb_ext.h>

/*
 * Enumerate the certificates that are used to establish the chain of trust
 */
enum {
	STM32MP_CONFIG_CERT = FWU_CERT + 1
};

/* Plat Defined TBBR extensions */
enum {
	DDR_FW_HASH_EXT = FWU_HASH_EXT + 1,
};

#endif /* STM32MP2_TBB_CERT_H */
