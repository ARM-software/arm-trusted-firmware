#
# Copyright 2020-2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${NOR_ADDED},)

NOR_ADDED		:= 1

NOR_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/ifc/nor

NOR_SOURCES		:=  $(NOR_DRIVERS_PATH)/ifc_nor.c

PLAT_INCLUDES		+= -I$(PLAT_DRIVERS_INCLUDE_PATH)/ifc

ifeq (${BL_COMM_IFC_NOR_NEEDED},yes)
BL_COMMON_SOURCES	+= ${NOR_SOURCES}
else
ifeq (${BL2_IFC_NOR_NEEDED},yes)
BL2_SOURCES		+= ${NOR_SOURCES}
endif
ifeq (${BL31_IFC_NOR_NEEDED},yes)
BL31_SOURCES		+= ${NOR_SOURCES}
endif
endif

endif
