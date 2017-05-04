#
# Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# TSP source files common to ARM standard platforms
BL32_SOURCES		+=	plat/arm/common/arm_topology.c			\
				plat/arm/common/tsp/arm_tsp_setup.c		\
				plat/common/aarch64/platform_mp_stack.S
