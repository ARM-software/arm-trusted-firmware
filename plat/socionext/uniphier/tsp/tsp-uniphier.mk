#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL32_SOURCES		+=	plat/common/plat_gicv3.c		\
				plat/common/aarch64/platform_mp_stack.S \
				$(PLAT_PATH)/tsp/uniphier_tsp_setup.c
