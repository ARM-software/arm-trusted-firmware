#
# Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL2_SOURCES		+=	bl2/bl2_main.c				\
				bl2/${ARCH}/bl2_entrypoint.S		\
				bl2/${ARCH}/bl2_arch_setup.c		\
				lib/locks/exclusive/${ARCH}/spinlock.S	\
				plat/common/${ARCH}/platform_up_stack.S

ifeq (${ARCH},aarch64)
BL2_SOURCES		+=	common/aarch64/early_exceptions.S
endif

ifeq (${LOAD_IMAGE_V2},1)
BL2_SOURCES		+=	bl2/bl2_image_load_v2.c
else
BL2_SOURCES		+=	bl2/bl2_image_load.c
endif

BL2_LINKERFILE		:=	bl2/bl2.ld.S
