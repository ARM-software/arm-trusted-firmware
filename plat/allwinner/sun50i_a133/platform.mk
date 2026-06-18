#
# Copyright (c) 2020-2026, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

SUNXI_BL31_IN_DRAM	:=	1

# SCPI support requires the SCP firmware, which is still a work in progress.
SUNXI_PSCI_USE_SCPI	:=	0
SUNXI_PSCI_USE_NATIVE	:=	1

# The differences between the platform are covered by the include files.
include plat/allwinner/common/allwinner-common.mk
include plat/allwinner/common/allwinner-common-a53.mk

BL31_SOURCES		+=	common/fdt_wrappers.c		\
				drivers/mentor/i2c/mi2cv.c
