/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IO_MEMDRV_H
#define IO_MEMDRV_H

struct io_dev_connector;
int32_t rcar_register_io_dev_memdrv(const io_dev_connector_t **connector);

#endif /* IO_MEMDRV_H */
