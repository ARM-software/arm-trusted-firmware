/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IO_PRIVATE_H
#define IO_PRIVATE_H

/*
 * Return codes reported by 'io_*' APIs
 * The value of fail should not overlap with define of the errno.
 * The errno is in "include/lib/stdlib/sys/errno.h".
 */
#define IO_SUCCESS		(0)
#define IO_FAIL			(-0x81)
#define IO_NOT_SUPPORTED	(-0x82)
#define IO_RESOURCES_EXHAUSTED	(-0x83)

#endif /* IO_PRIVATE_H */
