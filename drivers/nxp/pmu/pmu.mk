#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------
ifeq (${PMU_ADDED},)

PMU_ADDED		:= 1

PMU_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/pmu

PLAT_INCLUDES		+= -I$(PMU_DRIVERS_PATH)

PMU_SOURCES		+= $(PMU_DRIVERS_PATH)/pmu.c

ifeq (${BL_COMM_PMU_NEEDED},yes)
BL_COMMON_SOURCES	+= ${PMU_SOURCES}
else
ifeq (${BL2_PMU_NEEDED},yes)
BL2_SOURCES		+= ${PMU_SOURCES}
endif
ifeq (${BL31_PMU_NEEDED},yes)
BL31_SOURCES		+= ${PMU_SOURCES}
endif
endif
endif
#------------------------------------------------
