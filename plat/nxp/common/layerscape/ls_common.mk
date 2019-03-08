#
# Copyright 2018,2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_BL_COMMON_SOURCES	+=	plat/nxp/common/layerscape/ls_helpers.S		\
				plat/nxp/common/layerscape/ls_common.c


# Enable workarounds for platform specific errata
ifeq ($(ERRATA), yes)
include ${PLAT_COMMON_PATH}/layerscape/errata.mk
endif

ifeq (${TRUSTED_BOARD_BOOT},1)
ifeq ($(SECURE_BOOT),)
SECURE_BOOT := yes
endif
endif

include $(PLAT_TOOL_PATH)/create_pbl.mk
