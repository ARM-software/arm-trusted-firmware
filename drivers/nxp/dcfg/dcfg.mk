#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${ADD_DCFG},)

ADD_DCFG		:= 1

DCFG_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/dcfg

PLAT_INCLUDES		+= -I$(DCFG_DRIVERS_PATH)

DCFG_SOURCES		+= $(DCFG_DRIVERS_PATH)/dcfg.c

ifeq (${BL_COMM_DCFG_NEEDED},yes)
BL_COMMON_SOURCES	+= ${DCFG_SOURCES}
else
ifeq (${BL2_DCFG_NEEDED},yes)
BL2_SOURCES		+= ${DCFG_SOURCES}
endif
ifeq (${BL31_DCFG_NEEDED},yes)
BL31_SOURCES		+= ${DCFG_SOURCES}
endif
endif

endif
