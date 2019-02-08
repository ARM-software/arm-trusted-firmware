#
# Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

SPD_INCLUDES		:=

SPD_SOURCES		:=	services/spd/trusty/trusty.c		\
				services/spd/trusty/trusty_helpers.S

ifeq (${TRUSTY_SPD_WITH_GENERIC_SERVICES},1)
SPD_SOURCES		+=	services/spd/trusty/generic-arm64-smcall.c
endif

BL31_CFLAGS	+=		-DPLAT_XLAT_TABLES_DYNAMIC=1

NEED_BL32		:=	yes

CTX_INCLUDE_FPREGS	:=	1
