#
# Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL1_SOURCES		+=	bl1/bl1_main.c				\
				bl1/${ARCH}/bl1_arch_setup.c		\
				bl1/${ARCH}/bl1_context_mgmt.c		\
				bl1/${ARCH}/bl1_entrypoint.S		\
				bl1/${ARCH}/bl1_exceptions.S		\
				lib/cpus/${ARCH}/cpu_helpers.S		\
				lib/cpus/errata_report.c		\
				lib/el3_runtime/${ARCH}/context_mgmt.c	\
				plat/common/plat_bl1_common.c		\
				plat/common/${ARCH}/platform_up_stack.S \
				${MBEDTLS_SOURCES}

ifeq (${DISABLE_MTPMU},1)
BL1_SOURCES		+=	lib/extensions/mtpmu/${ARCH}/mtpmu.S
endif

ifeq (${ARCH},aarch64)
BL1_SOURCES		+=	lib/cpus/aarch64/dsu_helpers.S		\
				lib/el3_runtime/aarch64/context.S
endif

ifeq (${TRUSTED_BOARD_BOOT},1)
BL1_SOURCES		+=	bl1/bl1_fwu.c
endif

BL1_LINKERFILE		:=	bl1/bl1.ld.S
