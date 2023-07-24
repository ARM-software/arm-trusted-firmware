#
# Copyright (c) 2023, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/libfdt/libfdt.mk
include common/fdt_wrappers.mk

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/		\
				-I${PLAT_QEMU_COMMON_PATH}/include	\
				-I${PLAT_QEMU_PATH}/include		\
				-Iinclude/common/tbbr

ifeq (${ARCH},aarch32)
QEMU_CPU_LIBS		:=	lib/cpus/${ARCH}/cortex_a15.S
else
QEMU_CPU_LIBS		:=	lib/cpus/aarch64/aem_generic.S		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				lib/cpus/aarch64/cortex_a76.S		\
				lib/cpus/aarch64/neoverse_n_common.S	\
				lib/cpus/aarch64/neoverse_n1.S		\
				lib/cpus/aarch64/neoverse_v1.S		\
				lib/cpus/aarch64/qemu_max.S

PLAT_INCLUDES		+=	-Iinclude/plat/arm/common/${ARCH}
endif
