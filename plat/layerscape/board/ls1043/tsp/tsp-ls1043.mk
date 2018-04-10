#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# TSP source files specific to FVP platform
BL32_SOURCES		+=	plat/layerscape/board/ls1043/ls1043_topology.c		\
				plat/layerscape/board/ls1043/tsp/ls1043_tsp_setup.c		\
				${LS1043_GIC_SOURCES}

include plat/layerscape/common/tsp/ls_tsp.mk
