#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# SP MIN source files common to ARM standard platforms
BL32_SOURCES		+=	plat/arm/common/arm_pm.c			\
				plat/arm/common/arm_topology.c			\
				plat/arm/common/sp_min/arm_sp_min_setup.c	\
				plat/common/aarch32/platform_mp_stack.S		\
				plat/common/plat_psci_common.c

