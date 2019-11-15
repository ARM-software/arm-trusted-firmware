/*
 * Copyright (c) 2020, Linaro Limited. All rights reserved.
 * Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IO_ENCRYPTED_H
#define IO_ENCRYPTED_H

struct io_dev_connector;

int register_io_dev_enc(const struct io_dev_connector **dev_con);

#endif /* IO_ENCRYPTED_H */
