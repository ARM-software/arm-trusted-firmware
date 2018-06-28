/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IO_MMC_H__
#define __IO_MMC_H__

struct io_dev_connector;

int register_io_dev_emmc(const struct io_dev_connector **dev_con);

#endif /* __IO_MMC_H__ */
