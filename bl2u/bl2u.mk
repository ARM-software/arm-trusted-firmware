#
# Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL2U_SOURCES		+=	bl2u/bl2u_main.c			\
				bl2u/aarch64/bl2u_entrypoint.S		\
				common/aarch64/early_exceptions.S	\
				plat/common/aarch64/platform_up_stack.S

BL2U_LINKERFILE		:=	bl2u/bl2u.ld.S
