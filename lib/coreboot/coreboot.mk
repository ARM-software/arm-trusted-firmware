#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

COREBOOT := 0
$(eval $(call assert_boolean,COREBOOT))
$(eval $(call add_define,COREBOOT))

ifeq (${COREBOOT},1)

ifneq (${ARCH},aarch64)
$(error "coreboot only supports Trusted Firmware on AArch64.")
endif

BL31_SOURCES	+=	$(addprefix lib/coreboot/,	\
			coreboot_table.c)

BL31_SOURCES	+=	drivers/coreboot/cbmem_console/${ARCH}/cbmem_console.S

endif	# COREBOOT
