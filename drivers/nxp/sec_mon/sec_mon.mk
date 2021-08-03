#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${ADD_SNVS},)

ADD_SNVS		:= 1

PLAT_INCLUDES		+= -I$(PLAT_DRIVERS_INCLUDE_PATH)/sec_mon

SNVS_SOURCES		+= $(PLAT_DRIVERS_PATH)/sec_mon/snvs.c

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
