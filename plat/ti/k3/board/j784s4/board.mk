#
# Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Define sec_proxy usage as the full prioritized communication scheme
K3_SEC_PROXY_LITE	:=	0
$(eval $(call add_define,K3_SEC_PROXY_LITE))

# Use a 4 cycle data RAM latency for J784s4
K3_DATA_RAM_4_LATENCY	:=	1
$(eval $(call add_define,K3_DATA_RAM_4_LATENCY))

# Delay snoop exclusive handling for J784s4
K3_EXCLUSIVE_SNOOP_DELAY	:=	1
$(eval $(call add_define,K3_EXCLUSIVE_SNOOP_DELAY))

# System coherency is managed in hardware
USE_COHERENT_MEM	:=	1

# In generic K3 we use the sec_proxy for TI SCI transport
K3_TI_SCI_TRANSPORT    =       ${PLAT_PATH}/common/drivers/sec_proxy/sec_proxy.c

K3_PSCI_SOURCES		+=	\
				${PLAT_PATH}/common/k3_psci.c		\
