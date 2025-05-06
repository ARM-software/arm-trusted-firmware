#
# Copyright (c) 2022, STMicroelectronics - All Rights Reserved
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Override TBBR Cert to update generic certificate

CRTTOOL_DEFINES += PDEF_CERTS

CRTTOOL_INCLUDE_DIRS	+= ${PLAT_DIR}include

CRTTOOL_SOURCES		+= ${PLAT_DIR}stm32mp1_tbb_cert.c
