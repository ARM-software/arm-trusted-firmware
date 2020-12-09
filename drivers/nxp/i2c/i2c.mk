#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${ADD_I2C},)

ADD_I2C			:= 1
I2C_DRIVERS_PATH        := ${PLAT_DRIVERS_PATH}/i2c

I2C_SOURCES		+= $(I2C_DRIVERS_PATH)/i2c.c
PLAT_INCLUDES		+= -I$(I2C_DRIVERS_PATH)

ifeq (${BL_COMM_I2C_NEEDED},yes)
BL_COMMON_SOURCES	+= ${I2C_SOURCES}
else
ifeq (${BL2_I2C_NEEDED},yes)
BL2_SOURCES		+= ${I2C_SOURCES}
endif
ifeq (${BL31_I2C_NEEDED},yes)
BL31_SOURCES		+= ${I2C_SOURCES}
endif
endif
endif
