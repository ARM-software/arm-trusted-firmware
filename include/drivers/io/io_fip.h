/*
 * Copyright (c) 2014-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IO_FIP_H
#define IO_FIP_H

struct io_dev_connector;

int register_io_dev_fip(const struct io_dev_connector **dev_con);
int fip_dev_get_plat_toc_flag(io_dev_info_t *dev_info, uint16_t *plat_toc_flag);

#endif /* IO_FIP_H */
