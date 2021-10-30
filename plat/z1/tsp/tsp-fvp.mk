#
# Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# TSP source files specific to FVP platform
BL32_SOURCES		+=	drivers/arm/fvp/fvp_pwrc.c			\
				plat/arm/board/fvp/aarch64/fvp_helpers.S	\
				plat/arm/board/fvp/fvp_topology.c		\
				plat/arm/board/fvp/tsp/fvp_tsp_setup.c		\
				${FVP_GIC_SOURCES}

include plat/arm/common/tsp/arm_tsp.mk
