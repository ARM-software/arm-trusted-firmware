#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------

ifeq (${GPIO_ADDED},)

GPIO_ADDED		:= 1

GPIO_DRIVERS_PATH	:=  drivers/nxp/gpio

PLAT_INCLUDES		+=  -I$(GPIO_DRIVERS_PATH)

GPIO_SOURCES		:= $(GPIO_DRIVERS_PATH)/nxp_gpio.c

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
