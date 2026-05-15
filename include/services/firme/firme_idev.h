/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FIRME_IDEV_H
#define FIRME_IDEV_H

#include <services/firme/firme_abi.h>

#define FIRME_INTEGRATED_DEVICE_MGMT_VERSION_MAJOR	U(0)
#define FIRME_INTEGRATED_DEVICE_MGMT_VERSION_MINOR	U(0)

#define FIRME_IDEV_FNUM_OP_START		U(0x10)
#define FIRME_IDEV_FNUM_OP_CONTINUE		U(0x11)

#define FIRME_IDEV_OP_START_FID			FIRME_FID(FIRME_IDEV_FNUM_OP_START)
#define FIRME_IDEV_OP_CONTINUE_FID		FIRME_FID(FIRME_IDEV_FNUM_OP_CONTINUE)

#endif /* FIRME_IDEV_H */
