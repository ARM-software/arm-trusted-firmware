#
# Copyright (c) 2024, Texas Instruments Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES		+=	\
				-I${PLAT_PATH}/common/drivers/pm/include/			\

BL31_SOURCES		+=	\
				${PLAT_PATH}/common/drivers/pm/trace/trace.c    \

include ${PLAT_PATH}/common/drivers/pm/clock/clock.mk
include ${PLAT_PATH}/common/drivers/pm/device/device.mk
include ${PLAT_PATH}/common/drivers/pm/psc/psc.mk


