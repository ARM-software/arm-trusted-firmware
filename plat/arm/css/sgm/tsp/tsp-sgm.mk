#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL32_SOURCES		+= 	${SGM_GIC_SOURCES}			\
				${CSS_SGM_BASE}/sgm_plat_config.c	\
				plat/arm/board/sgm/tsp/sgm_tsp_setup.c

include plat/arm/common/tsp/arm_tsp.mk
