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

# xspi_nor_diag uses the JEDEC RDID/SFDP/SR1 from fspi.c
# Enabled on demand: make PLAT=lx2160ardb NXP_XSPI_DIAG=1 ...
ifeq ($(NXP_XSPI_DIAG),1)
XSPI_BOOT_SOURCES	+= ${FLEXSPI_DRIVERS_PATH}/xspi_nor_diag.c
$(eval $(call add_define,NXP_XSPI_DIAG))
endif

# Vendor-specific helpers (opcode sets, lock-register models). Linked
# in only when the corresponding CONFIG_<CHIP> is set in the
# platform's FLASH_TYPE selection (see flash_info.h).
ifeq (${FLASH_TYPE},GD55LB02GF)
XSPI_BOOT_SOURCES	+= ${FLEXSPI_DRIVERS_PATH}/flash_vendor_gigadevice.c
endif
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
