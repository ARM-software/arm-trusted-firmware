#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# We don't use BL1 or BL2, so BL31 is the first image to execute
RESET_TO_BL31		:=	1
# Only one core starts up at first
COLD_BOOT_SINGLE_CPU	:=	1
# We can choose where a core starts executing
PROGRAMMABLE_RESET_ADDRESS:=	1

# System coherency is managed in hardware
HW_ASSISTED_COHERENCY	:=	1
USE_COHERENT_MEM	:=	0

ERROR_DEPRECATED	:=	1
ENABLE_PLAT_COMPAT	:=	0

# A53 erratum for SoC. (enable them all)
ERRATA_A53_826319	:=	1
ERRATA_A53_835769	:=	1
ERRATA_A53_836870	:=	1
ERRATA_A53_843419	:=	1
ERRATA_A53_855873	:=	1

PLAT_INCLUDES		+=	\
				-I${PLAT_PATH}/include			\
				-Iinclude/plat/arm/common/		\
				-Iinclude/plat/arm/common/aarch64/	\

PLAT_BL_COMMON_SOURCES	+=	\
				lib/cpus/aarch64/cortex_a53.S		\

BL31_SOURCES		+=	\
				${PLAT_PATH}/common/k3_bl31_setup.c	\
				${PLAT_PATH}/common/k3_helpers.S	\
