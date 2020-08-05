#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_PATH	:=	plat/ti/k3
TARGET_BOARD	?=	generic

include ${PLAT_PATH}/common/plat_common.mk
include ${PLAT_PATH}/board/${TARGET_BOARD}/board.mk

# modify BUILD_PLAT to point to board specific build directory
BUILD_PLAT := $(abspath ${BUILD_BASE})/${PLAT}/${TARGET_BOARD}/${BUILD_TYPE}
