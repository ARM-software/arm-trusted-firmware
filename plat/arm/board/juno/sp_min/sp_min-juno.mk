#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# SP_MIN source files specific to JUNO platform
BL32_SOURCES	+=	lib/cpus/aarch32/cortex_a53.S		\
			lib/cpus/aarch32/cortex_a57.S		\
			lib/cpus/aarch32/cortex_a72.S		\
			plat/arm/board/juno/juno_topology.c	\
			plat/arm/css/common/css_pm.c		\
			plat/arm/css/common/css_topology.c	\
			plat/arm/soc/common/soc_css_security.c	\
			plat/arm/css/drivers/scp/css_pm_scpi.c	\
			plat/arm/css/drivers/scpi/css_mhu.c	\
			plat/arm/css/drivers/scpi/css_scpi.c	\
			${JUNO_GIC_SOURCES}			\
			${JUNO_INTERCONNECT_SOURCES}		\
			${JUNO_SECURITY_SOURCES}

include plat/arm/common/sp_min/arm_sp_min.mk
