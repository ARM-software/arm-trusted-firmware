#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2026 Free Mobile - Vincent Jardin
#
# Build glue for the FlexSPI NOR io_storage backend. Platforms
# pull this in from their platform.mk when they want io_open() /
# io_read() / io_write() / io_erase() against the chip on
# FlexSPI Flash A CS0.
#

NXP_IO_XSPI_NOR_PATH	:= drivers/nxp/io
NXP_IO_XSPI_NOR_SOURCES	+= ${NXP_IO_XSPI_NOR_PATH}/io_xspi_nor.c

PLAT_INCLUDES		+= -Iinclude/drivers/nxp/io
