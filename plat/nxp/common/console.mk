#
# Copyright 2018 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#------------------------------------------------------------------------------
#
# Select the CORE files
#
# -----------------------------------------------------------------------------

ifeq ($(CONSOLE), NS16550)
CONSOLE_SOURCES		:= 	$(PLAT_DRIVERS_PATH)/uart/16550_console.S	\
				$(PLAT_COMMON_PATH)/console_16550.c
else
    $(error -> CONSOLE not set!)
endif

# -----------------------------------------------------------------------------

