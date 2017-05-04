#
# Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL32_SOURCES		+=	plat/arm/board/juno/juno_topology.c	\
				plat/arm/css/common/css_topology.c	\
				${JUNO_GIC_SOURCES}

include plat/arm/common/tsp/arm_tsp.mk
