#
# Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES += -Idrivers/ti/pd/include/ \
		 -Idrivers/ti/common/include \
		 -Idrivers/ti/common/pm/include

BL31_SOURCES += drivers/ti/pd/ti_device_clk.c \
		drivers/ti/pd/ti_device_pm.c \
		drivers/ti/pd/ti_device.c \
		drivers/ti/pd/ti_device_prepare.c \
		drivers/ti/pd/ti_psc.c \
		drivers/ti/pd/ti_device_psc.c \
		drivers/ti/pd/ti_device_handler.c
