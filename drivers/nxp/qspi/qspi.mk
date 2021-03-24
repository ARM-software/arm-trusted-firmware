#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${QSPI_ADDED},)

QSPI_ADDED		:= 1

QSPI_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/qspi

QSPI_SOURCES		:=  $(QSPI_DRIVERS_PATH)/qspi.c

PLAT_INCLUDES		+= -I$(QSPI_DRIVERS_PATH)

ifeq (${BL_COMM_QSPI_NEEDED},yes)
BL_COMMON_SOURCES	+= ${QSPI_SOURCES}
else
ifeq (${BL2_QSPI_NEEDED},yes)
BL2_SOURCES		+= ${QSPI_SOURCES}
endif
ifeq (${BL31_QSPI_NEEDED},yes)
BL31_SOURCES		+= ${QSPI_SOURCES}
endif
endif

endif
