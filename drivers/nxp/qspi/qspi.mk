#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${QSPI_ADDED},)

QSPI_ADDED		:= 1

QSPI_SOURCES		:= $(PLAT_DRIVERS_PATH)/qspi/qspi.c

PLAT_INCLUDES		+= -I$(PLAT_DRIVERS_PATH)/qspi

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
