#
# Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES += -I${PLAT_PATH}/common/pm/include/

include drivers/ti/clk/ti_clk.mk
include ${PLAT_PATH}/common/pm/clock/clock.mk
include ${PLAT_PATH}/common/pm/device/device.mk

