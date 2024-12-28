/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ERRNO_H
#define ERRNO_H

/** Success */
#define SUCCESS		0
/** Error for permission */
#define EPERM		1
/** Verification Failed */
#define EFAILVERIFY	9
/** Try Again later */
#define EAGAIN		11
/** Device Busy */
#define EBUSY		16
/** No such device */
#define ENODEV		19
/** Invalid args */
#define EINVAL		22
/** Initialization failed */
#define EINIT		25
/** Deferred */
#define EDEFER		517
/** Timed out */
#define ETIMEDOUT	110
/** Operation failed */
#define EFAIL		427
/** Internal error due to corruption */
#define ECONFUSED	436

#endif
