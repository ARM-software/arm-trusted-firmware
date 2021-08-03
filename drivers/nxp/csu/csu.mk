#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------
ifeq (${CSU_ADDED},)

CSU_ADDED		:= 1

PLAT_INCLUDES		+= -I$(PLAT_DRIVERS_INCLUDE_PATH)/csu

CSU_SOURCES		+= $(PLAT_DRIVERS_PATH)/csu/csu.c

ifeq (${BL_COMM_CSU_NEEDED},yes)
BL_COMMON_SOURCES	+= ${CSU_SOURCES}
else
ifeq (${BL2_CSU_NEEDED},yes)
BL2_SOURCES		+= ${CSU_SOURCES}
endif
ifeq (${BL31_CSU_NEEDED},yes)
BL31_SOURCES		+= ${CSU_SOURCES}
endif
endif

endif
