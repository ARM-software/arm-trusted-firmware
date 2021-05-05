#
# Copyright (c) 2019-2021, ARM Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include common/fdt_wrappers.mk

FCONF_SOURCES		:=	lib/fconf/fconf.c
FCONF_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

FCONF_DYN_SOURCES	:=	lib/fconf/fconf_dyn_cfg_getter.c
FCONF_DYN_SOURCES	+=	${FDT_WRAPPERS_SOURCES}

FCONF_AMU_SOURCES	:=	lib/fconf/fconf_amu_getter.c
FCONF_AMU_SOURCES	+=	${FDT_WRAPPERS_SOURCES}

FCONF_MPMM_SOURCES	:=	lib/fconf/fconf_mpmm_getter.c
FCONF_MPMM_SOURCES	+=	${FDT_WRAPPERS_SOURCES}
