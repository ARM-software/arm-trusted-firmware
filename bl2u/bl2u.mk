#
# Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL2U_SOURCES		+=	bl2u/bl2u_main.c			\
				bl2u/${ARCH}/bl2u_entrypoint.S		\
				plat/common/${ARCH}/platform_up_stack.S

ifeq (${ARCH},aarch64)
BL2U_SOURCES		+=	common/aarch64/early_exceptions.S
endif

BL2U_LINKERFILE		:=	bl2u/bl2u.ld.S
