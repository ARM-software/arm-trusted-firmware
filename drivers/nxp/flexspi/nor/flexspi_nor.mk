#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${XSPI_NOR},)
XSPI_NOR	:= 1

FLEXSPI_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/flexspi/nor

PLAT_XSPI_INCLUDES	+= -I$(FLEXSPI_DRIVERS_PATH)

XSPI_BOOT_SOURCES	+= $(FLEXSPI_DRIVERS_PATH)/flexspi_nor.c	\
			   ${FLEXSPI_DRIVERS_PATH}/fspi.c
ifeq ($(DEBUG),1)
XSPI_BOOT_SOURCES	+= ${FLEXSPI_DRIVERS_PATH}/test_fspi.c
endif

PLAT_XSPI_INCLUDES	+= -Iinclude/drivers/nxp/flexspi

PLAT_INCLUDES		+= ${PLAT_XSPI_INCLUDES}

ifeq (${BL_COMM_XSPI_NEEDED},yes)
BL_COMMON_SOURCES	+= ${XSPI_BOOT_SOURCES}
else
ifeq (${BL2_XSPI_NEEDED},yes)
BL2_SOURCES		+= ${XSPI_BOOT_SOURCES}
endif
ifeq (${BL31_XSPI_NEEDED},yes)
BL31_SOURCES		+= ${XSPI_BOOT_SOURCES}
endif
endif

endif
