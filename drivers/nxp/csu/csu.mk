#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------
ifeq (${CSU_ADDED},)

CSU_ADDED		:= 1

CSU_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/csu

PLAT_INCLUDES		+= -I$(CSU_DRIVERS_PATH)

CSU_SOURCES		+= $(CSU_DRIVERS_PATH)/csu.c

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
