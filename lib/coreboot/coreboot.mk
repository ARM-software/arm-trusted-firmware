#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL31_SOURCES	+=	$(addprefix lib/coreboot/,	\
			coreboot_table.c)

BL31_SOURCES	+=	drivers/coreboot/cbmem_console/${ARCH}/cbmem_console.S
