#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
#------------------------------------------------------------------------------
#
# Select the PSCI files
#
# -----------------------------------------------------------------------------

ifeq (${ADD_PSCI},)

ADD_PSCI		:= 1
PLAT_PSCI_PATH		:= $(PLAT_COMMON_PATH)/psci

PSCI_SOURCES		:= ${PLAT_PSCI_PATH}/plat_psci.c	\
			   ${PLAT_PSCI_PATH}/$(ARCH)/psci_utils.S	\
			   plat/common/plat_psci_common.c

PLAT_INCLUDES		+= -I${PLAT_PSCI_PATH}/include

ifeq (${BL_COMM_PSCI_NEEDED},yes)
BL_COMMON_SOURCES	+= ${PSCI_SOURCES}
else
ifeq (${BL2_PSCI_NEEDED},yes)
BL2_SOURCES		+= ${PSCI_SOURCES}
endif
ifeq (${BL31_PSCI_NEEDED},yes)
BL31_SOURCES		+= ${PSCI_SOURCES}
endif
endif
endif
# -----------------------------------------------------------------------------
