#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------

ifeq (${GPIO_ADDED},)

GPIO_ADDED		:= 1

PLAT_INCLUDES		+= -I$(PLAT_DRIVERS_INCLUDE_PATH)/gpio

GPIO_SOURCES		:= $(PLAT_DRIVERS_PATH)/gpio/nxp_gpio.c

ifeq (${BL_COMM_GPIO_NEEDED},yes)
BL_COMMON_SOURCES	+= ${GPIO_SOURCES}
else
ifeq (${BL2_GPIO_NEEDED},yes)
BL2_SOURCES		+= ${GPIO_SOURCES}
endif
ifeq (${BL31_GPIO_NEEDED},yes)
BL31_SOURCES		+= ${GPIO_SOURCES}
endif
endif

endif
#------------------------------------------------
