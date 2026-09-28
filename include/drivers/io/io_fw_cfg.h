/*
 * Copyright (c) 2026, Kuan-Wei Chiu <visitorckw@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IO_FW_CFG_H
#define IO_FW_CFG_H

struct io_dev_connector;

int register_io_dev_fw_cfg(const struct io_dev_connector **dev_con);

#endif /* IO_FW_CFG_H */
