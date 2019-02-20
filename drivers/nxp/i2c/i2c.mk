#
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

I2C_DRIVERS_PATH        :=      ${PLAT_DRIVERS_PATH}/i2c

BL2_SOURCES		+=  $(I2C_DRIVERS_PATH)/i2c.c
PLAT_INCLUDES		+= -I$(I2C_DRIVERS_PATH)
