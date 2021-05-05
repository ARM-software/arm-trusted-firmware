#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/extensions/amu/amu.mk
include lib/fconf/fconf.mk

ifneq (${ENABLE_MPMM},0)
        ifneq ($(ARCH),aarch64)
                $(error MPMM support (`ENABLE_MPMM`) can only be enabled in AArch64 images (`ARCH`))
        endif

        ifeq (${ENABLE_AMU_AUXILIARY_COUNTERS},0) # For MPMM gear AMU counters
                $(error MPMM support (`ENABLE_MPM`) requires auxiliary AMU counter support (`ENABLE_AMU_AUXILIARY_COUNTERS`))
        endif
endif

MPMM_SOURCES	:=	lib/mpmm/mpmm.c
MPMM_SOURCES	+=	${AMU_SOURCES}

ifneq (${ENABLE_MPMM_FCONF},0)
        ifeq (${ENABLE_MPMM},0)
                $(error MPMM FCONF support (`ENABLE_MPMM_FCONF`) requires MPMM support (`ENABLE_MPMM`))
        endif

        MPMM_SOURCES	+= ${FCONF_MPMM_SOURCES}
endif
