#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# The differences between the platform are covered by the include files.
include plat/allwinner/common/allwinner-common.mk

PLAT_BL_COMMON_SOURCES	+=	drivers/mentor/i2c/mi2cv.c

# Broken Watchdog
SUNXI_WDT_QUIRK := 1
$(eval $(call add_define,SUNXI_WDT_QUIRK))
