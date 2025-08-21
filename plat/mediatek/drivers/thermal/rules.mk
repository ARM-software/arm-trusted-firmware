#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := thermal

PLAT_INCLUDES += -I${LOCAL_DIR}
PLAT_INCLUDES += -I${LOCAL_DIR}/inc
PLAT_INCLUDES += -I${LOCAL_DIR}/${MTK_SOC}
PLAT_INCLUDES += -I${MTK_PLAT}/include/drivers/thermal/${MTK_SOC}

SUB_RULES-y := ${LOCAL_DIR}/${MTK_SOC}

$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES-y)))
