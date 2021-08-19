#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/fconf/fconf.mk

AMU_SOURCES	:=	lib/extensions/amu/${ARCH}/amu.c \
			lib/extensions/amu/${ARCH}/amu_helpers.S

ifneq (${ENABLE_AMU_AUXILIARY_COUNTERS},0)
        ifeq (${ENABLE_AMU},0)
                $(error AMU auxiliary counter support (`ENABLE_AMU_AUXILIARY_COUNTERS`) requires AMU support (`ENABLE_AMU`))
        endif
endif

ifneq (${ENABLE_AMU_FCONF},0)
        ifeq (${ENABLE_AMU_AUXILIARY_COUNTERS},0)
                $(error AMU FCONF support (`ENABLE_AMU_FCONF`) is not necessary when auxiliary counter support (`ENABLE_AMU_AUXILIARY_COUNTERS`) is disabled)
        endif

        AMU_SOURCES	+=	${FCONF_AMU_SOURCES}
endif
