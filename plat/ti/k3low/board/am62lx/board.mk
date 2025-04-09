#
# Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# We dont have system level coherency capability
USE_COHERENT_MEM	:=	0

BL32_BASE ?= 0x80200000
$(eval $(call add_define,BL32_BASE))

PRELOADED_BL33_BASE ?= 0x82000000
$(eval $(call add_define,PRELOADED_BL33_BASE))

K3_HW_CONFIG_BASE ?= 0x88000000
$(eval $(call add_define,K3_HW_CONFIG_BASE))
