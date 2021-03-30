#
# Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# The differences between the platform are covered by the include files.
include plat/allwinner/common/allwinner-common.mk

BL31_SOURCES		+=	drivers/allwinner/axp/axp803.c		\
				drivers/allwinner/sunxi_rsb.c

FDT_ASSUME_MASK := "(ASSUME_LATEST | ASSUME_NO_ROLLBACK | ASSUME_LIBFDT_ORDER)"
$(eval $(call add_define,FDT_ASSUME_MASK))

# Put NOBITS memory in SRAM A1, overwriting U-Boot's SPL.
SEPARATE_NOBITS_REGION	:=	1
