/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PDEF_TBB_KEY_H
#define PDEF_TBB_KEY_H

#include <tbbr/tbb_key.h>

/*
 * Enumerate the pltform defined keys that are used to establish the chain of trust
 */
enum {
	DDR_FW_CONTENT_KEY = NON_TRUSTED_FW_CONTENT_CERT_KEY + 1,
};
#endif /* PDEF_TBB_KEY_H */
