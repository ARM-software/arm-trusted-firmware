#
# Copyright (c) 2013-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Default configuration values
GICV3_IMPL			?=	GIC500
GICV3_IMPL_GIC600_MULTICHIP	?=	0
GICV3_OVERRIDE_DISTIF_PWR_OPS	?=	0

GICV3_SOURCES	+=	drivers/arm/gic/common/gic_common.c	\
			drivers/arm/gic/v3/gicv3_main.c		\
			drivers/arm/gic/v3/gicv3_helpers.c	\
			drivers/arm/gic/v3/gicdv3_helpers.c	\
			drivers/arm/gic/v3/gicrv3_helpers.c

ifeq (${GICV3_OVERRIDE_DISTIF_PWR_OPS}, 0)
GICV3_SOURCES	+=	drivers/arm/gic/v3/arm_gicv3_common.c
endif

# Either GIC-600 or GIC-500 can be selected at one time
ifeq (${GICV3_IMPL}, GIC600)
# GIC-600 sources
GICV3_SOURCES	+=	drivers/arm/gic/v3/gic600.c
ifeq (${GICV3_IMPL_GIC600_MULTICHIP}, 1)
GICV3_SOURCES	+=	drivers/arm/gic/v3/gic600_multichip.c
endif
else ifeq (${GICV3_IMPL}, GIC500)
# GIC-500 sources
GICV3_SOURCES	+=	drivers/arm/gic/v3/gic500.c
else
$(error "Incorrect GICV3_IMPL value ${GICV3_IMPL}")
endif
