#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL32_SOURCES		+=	plat/qemu/sp_min/sp_min_setup.c		\
				plat/qemu/aarch32/plat_helpers.S	\
				plat/qemu/qemu_pm.c			\
				plat/qemu/topology.c

BL32_SOURCES		+=	lib/cpus/aarch32/aem_generic.S		\
				lib/cpus/aarch32/cortex_a15.S

BL32_SOURCES		+=	plat/common/aarch32/platform_mp_stack.S \
				plat/common/plat_psci_common.c \
				plat/common/plat_gicv2.c


BL32_SOURCES		+=	drivers/arm/gic/v2/gicv2_helpers.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/common/gic_common.c
