#
# Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# SP_MIN source files specific to FVP platform
BL32_SOURCES	+=	drivers/cfi/v2m/v2m_flash.c				\
			lib/utils/mem_region.c					\
			plat/arm/board/corstone700/common/corstone700_helpers.S	\
			plat/arm/board/corstone700/common/corstone700_topology.c	\
			plat/arm/board/corstone700/common/corstone700_security.c	\
			plat/arm/board/corstone700/common/corstone700_plat.c		\
			plat/arm/board/corstone700/common/corstone700_pm.c		\
			plat/arm/board/corstone700/sp_min/corstone700_sp_min_setup.c	\
			${CORSTONE700_GIC_SOURCES}

ifneq (${ENABLE_STACK_PROTECTOR},0)
	ifneq (${ENABLE_STACK_PROTECTOR},none)
		BL32_SOURCES += plat/arm/board/corstone700/common/corstone700_stack_protector.c
	endif
endif

include plat/arm/common/sp_min/arm_sp_min.mk
