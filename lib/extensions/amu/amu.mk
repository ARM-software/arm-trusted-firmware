#
# Copyright (c) 2021-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

AMU_SOURCES	:=	lib/extensions/amu/${ARCH}/amu.c

ifneq (${ENABLE_AMU_AUXILIARY_COUNTERS},0)
        ifeq (${ENABLE_FEAT_AMU},0)
                $(error "ENABLE_AMU_AUXILIARY_COUNTERS requires ENABLE_FEAT_AMU")
        endif
        ifeq (${ENABLE_FEAT_AMUv1p1},0)
                $(error "ENABLE_AMU_AUXILIARY_COUNTERS requires ENABLE_FEAT_AMUv1p1")
        endif
endif
