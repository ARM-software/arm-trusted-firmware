#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${ADD_TZASC},)

ADD_TZASC		:= 1

TZASC_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/tzc

PLAT_INCLUDES		+= -I$(TZASC_DRIVERS_PATH)

ifeq ($(TZC_ID), TZC400)
TZASC_SOURCES		+= drivers/arm/tzc/tzc400.c\
			   $(TZASC_DRIVERS_PATH)/plat_tzc400.c
else ifeq ($(TZC_ID), NONE)
    $(info -> No TZC present on platform)
else
    $(error -> TZC type not set!)
endif

ifeq (${BL_COMM_TZASC_NEEDED},yes)
BL_COMMON_SOURCES	+= ${TZASC_SOURCES}
else
ifeq (${BL2_TZASC_NEEDED},yes)
BL2_SOURCES		+= ${TZASC_SOURCES}
endif
ifeq (${BL31_TZASC_NEEDED},yes)
BL31_SOURCES		+= ${TZASC_SOURCES}
endif
endif

endif
