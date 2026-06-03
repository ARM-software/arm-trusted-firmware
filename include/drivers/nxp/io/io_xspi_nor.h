/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * io_storage backend for the NXP FlexSPI NOR controller.
 *
 * Layered on the existing FlexSPI NOR primitives in
 * drivers/nxp/flexspi/nor/. Implements the standard io_storage
 * dispatch slots (open, seek, size, read, write, erase, close)
 * over a single attached NOR chip on FlexSPI Flash A CS0.
 *
 * Open spec: io_block_spec_t {.offset, .length}, where @offset is
 * the flash-relative byte offset of the region to address and
 * @length is its size. The dispatched read/write/erase ops act on
 * a per-entity cursor inside that region and do not cross its
 * boundary.
 */

#ifndef IO_XSPI_NOR_H
#define IO_XSPI_NOR_H

#include <drivers/io/io_driver.h>

/*
 * Register the FlexSPI NOR backend with the io_storage core.
 * Must be called once during platform IO setup before any
 * io_dev_open() targeting this backend. Returns 0 on success and
 * writes the connector pointer to @dev_con; negative errno on
 * failure.
 *
 * The backend assumes fspi_init() has already initialised the
 * controller; this matches the existing NXP NOR boot path where
 * fspi_init() runs from plat_io_setup() before io_dev_open().
 */
int register_io_dev_xspi_nor(const io_dev_connector_t **dev_con);

#endif /* IO_XSPI_NOR_H */
