#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${ADD_TIMER},)

ADD_TIMER		:= 1

TIMER_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/timer

PLAT_INCLUDES		+= -I$(TIMER_DRIVERS_PATH)
TIMER_SOURCES	+= drivers/delay_timer/delay_timer.c	\
			   $(PLAT_DRIVERS_PATH)/timer/nxp_timer.c

ifeq (${BL_COMM_TIMER_NEEDED},yes)
BL_COMMON_SOURCES	+= ${TIMER_SOURCES}
else
ifeq (${BL2_TIMER_NEEDED},yes)
BL2_SOURCES		+= ${TIMER_SOURCES}
endif
ifeq (${BL31_TIMER_NEEDED},yes)
BL31_SOURCES		+= ${TIMER_SOURCES}
endif
endif
endif
