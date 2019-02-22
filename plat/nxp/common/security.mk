#
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#------------------------------------------------------------------------------
#
# Select the SECURITY files
#
# -----------------------------------------------------------------------------

ifeq ($(TZC_ID), TZC380)
SECURITY_SOURCES 	+=	drivers/arm/tzc/tzc380.c
else ifeq ($(TZC_ID), NONE)
    $(info -> No TZC present on platform)
else
    $(error -> TZC type not set!)
endif

ifeq ($(LAYERSCAPE), yes)
SECURITY_SOURCES	+=	${PLAT_COMMON_PATH}/layerscape/secure_boot.c	\
				${PLAT_DRIVERS_PATH}/sfp/sfp.c

PLAT_INCLUDES		+= 	-I${PLAT_DRIVERS_PATH}/sfp		\

SECURITY_SOURCES	+= 	${SEC_SOURCES}
endif

ifeq ($(CHASSIS), 2)
SECURITY_SOURCES 	+=	${PLAT_DRIVERS_PATH}/csu/csu.c
PLAT_INCLUDES		+=	-I${PLAT_DRIVERS_PATH}/csu
endif

# -----------------------------------------------------------------------------

