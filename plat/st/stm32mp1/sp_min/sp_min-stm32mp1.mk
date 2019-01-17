#
# Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

SP_MIN_WITH_SECURE_FIQ	:=	1

BL32_SOURCES		+=	plat/common/aarch32/platform_mp_stack.S		\
				plat/st/stm32mp1/sp_min/sp_min_setup.c		\
				plat/st/stm32mp1/stm32mp1_pm.c			\
				plat/st/stm32mp1/stm32mp1_topology.c
# Generic GIC v2
BL32_SOURCES		+=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				plat/common/plat_gicv2.c		\
				plat/st/stm32mp1/stm32mp1_gic.c

# Generic PSCI
BL32_SOURCES		+=	plat/common/plat_psci_common.c

# stm32mp1 specific services
BL32_SOURCES		+=	plat/st/stm32mp1/services/bsec_svc.c		\
				plat/st/stm32mp1/services/stm32mp1_svc_setup.c
