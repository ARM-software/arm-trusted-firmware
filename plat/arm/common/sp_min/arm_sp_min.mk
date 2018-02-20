#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# SP MIN source files common to ARM standard platforms

# Skip building BL1 and BL2 if RESET_TO_SP_MIN flag is set.
ifeq (${RESET_TO_SP_MIN},1)
    BL1_SOURCES =
    BL2_SOURCES =
endif

BL32_SOURCES		+=	plat/arm/common/arm_pm.c			\
				plat/arm/common/arm_topology.c			\
				plat/arm/common/sp_min/arm_sp_min_setup.c	\
				plat/common/aarch32/platform_mp_stack.S		\
				plat/common/plat_psci_common.c

