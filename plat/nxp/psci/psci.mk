#
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
#------------------------------------------------------------------------------
#
# Select the PSCI files
#
# -----------------------------------------------------------------------------

PSCI_SOURCES	:=			 ${PLAT_PSCI_PATH}/plat_psci.c	\
					${PLAT_PSCI_PATH}/$(ARCH)/psci_utils.S	\
					plat/common/plat_psci_common.c

# -----------------------------------------------------------------------------

