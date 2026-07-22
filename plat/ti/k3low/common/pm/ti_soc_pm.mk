#
# Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES += -I${PLAT_PATH}/common/pm/include/

BL31_SOURCES += ${PLAT_PATH}/common/pm/ti_clocks.c \
		${PLAT_PATH}/common/pm/ti_devices.c \
		${PLAT_PATH}/common/pm/ti_host_idx_mapping.c
