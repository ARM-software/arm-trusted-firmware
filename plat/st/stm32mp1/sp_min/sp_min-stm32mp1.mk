#
# Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

SP_MIN_WITH_SECURE_FIQ	:=	1

BL32_CFLAGS		+=	-DSTM32MP_SHARED_RESOURCES

BL32_SOURCES		+=	drivers/st/etzpc/etzpc.c			\
				plat/common/aarch32/platform_mp_stack.S		\
				plat/st/stm32mp1/sp_min/sp_min_setup.c		\
				plat/st/stm32mp1/stm32mp1_pm.c			\
				plat/st/stm32mp1/stm32mp1_shared_resources.c	\
				plat/st/stm32mp1/stm32mp1_topology.c

# FDT wrappers
include common/fdt_wrappers.mk
BL32_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

# Generic GIC v2
include drivers/arm/gic/v2/gicv2.mk

BL32_SOURCES		+=	${GICV2_SOURCES}			\
				plat/common/plat_gicv2.c		\
				plat/st/stm32mp1/stm32mp1_gic.c

# Generic PSCI
BL32_SOURCES		+=	plat/common/plat_psci_common.c

# SCMI server drivers
BL32_SOURCES		+=	drivers/scmi-msg/base.c		\
				drivers/scmi-msg/clock.c		\
				drivers/scmi-msg/entry.c		\
				drivers/scmi-msg/reset_domain.c	\
				drivers/scmi-msg/smt.c

# stm32mp1 specific services
BL32_SOURCES		+=	plat/st/stm32mp1/services/bsec_svc.c		\
				plat/st/stm32mp1/services/stm32mp1_svc_setup.c	\
				plat/st/stm32mp1/stm32mp1_scmi.c

# Arm Archtecture services
BL32_SOURCES		+=	services/arm_arch_svc/arm_arch_svc_setup.c
