#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# TSP source files common to ARM standard platforms
BL32_SOURCES		+=	plat/layerscape/common/ls_topology.c			\
				plat/layerscape/common/tsp/ls_tsp_setup.c		\
				plat/common/aarch64/platform_mp_stack.S
