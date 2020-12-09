#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
#------------------------------------------------------------------------------
#
# Select the CORE files
#
# -----------------------------------------------------------------------------

ifeq (${ADD_CONSOLE},)

ADD_CONSOLE		:= 1

PLAT_INCLUDES		+=	-I$(PLAT_DRIVERS_PATH)/console

ifeq ($(CONSOLE), NS16550)
NXP_CONSOLE		:=	NS16550

$(eval $(call add_define_val,NXP_CONSOLE,${NXP_CONSOLE}))

CONSOLE_SOURCES		:=	$(PLAT_DRIVERS_PATH)/console/16550_console.S	\
				$(PLAT_DRIVERS_PATH)/console/console_16550.c
else
ifeq ($(CONSOLE), PL011)
CONSOLE_SOURCES		:=	drivers/arm/pl011/aarch64/pl011_console.S	\
				${PLAT_DRIVERS_PATH}/console/console_pl011.c
else
	$(error -> CONSOLE not set!)
endif
endif

ifeq (${BL_COMM_CONSOLE_NEEDED},yes)
BL_COMMON_SOURCES	+= ${CONSOLE_SOURCES}
else
ifeq (${BL2_CONSOLE_NEEDED},yes)
BL2_SOURCES		+= ${CONSOLE_SOURCES}
endif
ifeq (${BL31_CONSOLE_NEEDED},yes)
BL31_SOURCES		+= ${CONSOLE_SOURCES}
endif
endif
endif
# -----------------------------------------------------------------------------
