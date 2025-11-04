#
# Copyright (c) 2015-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# TSP source files common to ARM standard platforms
BL32_SOURCES		+=	plat/arm/common/arm_topology.c			\
				plat/arm/common/tsp/arm_tsp_setup.c		\
				plat/common/aarch64/platform_mp_stack.S

ifeq (${TRANSFER_LIST},1)
BL32_SOURCES	+=	$(TRANSFER_LIST_SOURCES)
ifeq (${MEASURED_BOOT},1)
        BL32_LIBS += $(LIBEVLOG_LIBS)
        BL32_INCLUDE_DIRS += $(LIBEVLOG_INCLUDE_DIRS)
endif
endif
