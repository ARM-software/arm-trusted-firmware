#
# Copyright (c) 2019-2020, ARM Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Add Firmware Configuration files
FCONF_SOURCES		:=	lib/fconf/fconf.c
FCONF_DYN_SOURCES	:=	lib/fconf/fconf_dyn_cfg_getter.c

BL1_SOURCES		+=	${FCONF_SOURCES} ${FCONF_DYN_SOURCES}
BL2_SOURCES		+=	${FCONF_SOURCES} ${FCONF_DYN_SOURCES}
