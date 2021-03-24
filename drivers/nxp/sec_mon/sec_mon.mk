#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${ADD_SNVS},)

ADD_SNVS		:= 1

SNVS_DRIVERS_PATH	:= ${PLAT_DRIVERS_PATH}/sec_mon

PLAT_INCLUDES		+= -I$(SNVS_DRIVERS_PATH)

SNVS_SOURCES		+= $(SNVS_DRIVERS_PATH)/snvs.c

ifeq (${BL_COMM_SNVS_NEEDED},yes)
BL_COMMON_SOURCES	+= ${SNVS_SOURCES}
else
ifeq (${BL2_SNVS_NEEDED},yes)
BL2_SOURCES		+= ${SNVS_SOURCES}
endif
ifeq (${BL31_SNVS_NEEDED},yes)
BL31_SOURCES		+= ${SNVS_SOURCES}
endif
endif
endif
