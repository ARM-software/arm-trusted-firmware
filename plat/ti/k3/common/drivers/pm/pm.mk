#
# Copyright (c) 2024, Texas Instruments Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES		+=	\
				-I${PLAT_PATH}/common/drivers/pm/include/			\

include ${PLAT_PATH}/common/drivers/pm/clock/clock.mk
include ${PLAT_PATH}/common/drivers/pm/device/device.mk
include ${PLAT_PATH}/common/drivers/pm/misc/misc.mk


