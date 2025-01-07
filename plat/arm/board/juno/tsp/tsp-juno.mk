#
# Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL32_SOURCES		+=	plat/arm/board/juno/juno_topology.c	\
				plat/arm/css/common/css_topology.c	\
				${GIC_SOURCES}				\
				${JUNO_SECURITY_SOURCES}

include plat/arm/common/tsp/arm_tsp.mk
