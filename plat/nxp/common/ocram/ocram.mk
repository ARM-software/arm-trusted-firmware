#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

PLAT_OCRAM_PATH		:= $(PLAT_COMMON_PATH)/ocram

OCRAM_SOURCES		:= ${PLAT_OCRAM_PATH}/$(ARCH)/ocram.S

BL2_SOURCES		+= ${OCRAM_SOURCES}

PLAT_INCLUDES           += -I${PLAT_COMMON_PATH}/ocram
