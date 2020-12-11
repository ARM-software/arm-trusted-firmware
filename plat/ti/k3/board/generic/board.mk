#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL32_BASE ?= 0x9e800000
$(eval $(call add_define,BL32_BASE))

PRELOADED_BL33_BASE ?= 0x80080000
$(eval $(call add_define,PRELOADED_BL33_BASE))

K3_HW_CONFIG_BASE ?= 0x82000000
$(eval $(call add_define,K3_HW_CONFIG_BASE))

# Define sec_proxy usage as the full prioritized communication scheme
K3_SEC_PROXY_LITE	:=	0
$(eval $(call add_define,K3_SEC_PROXY_LITE))

# System coherency is managed in hardware
USE_COHERENT_MEM	:=	1

PLAT_INCLUDES		+=	\
				-Iplat/ti/k3/board/generic/include	\
