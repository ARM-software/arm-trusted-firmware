#
# Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Define sec_proxy usage as the lite version
K3_SEC_PROXY_LITE	:=	1
$(eval $(call add_define,K3_SEC_PROXY_LITE))

# We dont have system level coherency capability
USE_COHERENT_MEM	:=	0
