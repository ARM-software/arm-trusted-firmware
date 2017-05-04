/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IO_FIP_H__
#define __IO_FIP_H__

struct io_dev_connector;

int register_io_dev_fip(const struct io_dev_connector **dev_con);

#endif /* __IO_FIP_H__ */
