#
# Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# SP_MIN source files specific to JUNO platform
BL32_SOURCES	+=	drivers/cfi/v2m/v2m_flash.c		\
			lib/cpus/aarch32/cortex_a53.S		\
			lib/cpus/aarch32/cortex_a57.S		\
			lib/cpus/aarch32/cortex_a72.S		\
			lib/utils/mem_region.c			\
			plat/arm/board/juno/juno_pm.c		\
			plat/arm/board/juno/juno_topology.c	\
			plat/arm/common/arm_nor_psci_mem_protect.c	\
			plat/arm/soc/common/soc_css_security.c	\
			${JUNO_GIC_SOURCES}			\
			${JUNO_INTERCONNECT_SOURCES}		\
			${JUNO_SECURITY_SOURCES}

include plat/arm/common/sp_min/arm_sp_min.mk
include plat/arm/css/common/sp_min/css_sp_min.mk
