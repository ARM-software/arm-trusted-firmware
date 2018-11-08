/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IO_SEMIHOSTING_H
#define IO_SEMIHOSTING_H

struct io_dev_connector;

int register_io_dev_sh(const struct io_dev_connector **dev_con);

#endif /* IO_SEMIHOSTING_H */
