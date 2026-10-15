#
# Copyright (c) 2026, STMicroelectronics - All Rights Reserved
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Override TBBR Cert to update generic certificate

CRTTOOL_DEFINES += PLAT_DEF_OID
CRTTOOL_DEFINES += PDEF_CERTS
CRTTOOL_DEFINES += PDEF_EXTS

CRTTOOL_INCLUDE_DIRS	+= ${PLAT_DIR}include

CRTTOOL_SOURCES		+= ${PLAT_DIR}stm32mp2_tbb_cert.c
