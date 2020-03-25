#
# Copyright (c) 2013-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Default configuration values
GICV3_SUPPORT_GIC600		?=	0
GICV3_IMPL_GIC600_MULTICHIP	?=	0
GICV3_OVERRIDE_DISTIF_PWR_OPS	?=	0
GIC_ENABLE_V4_EXTN		?=	0
GIC_EXT_INTID			?=	0

GICV3_SOURCES	+=	drivers/arm/gic/v3/gicv3_main.c		\
			drivers/arm/gic/v3/gicv3_helpers.c	\
			drivers/arm/gic/v3/gicdv3_helpers.c	\
			drivers/arm/gic/v3/gicrv3_helpers.c

ifeq (${GICV3_OVERRIDE_DISTIF_PWR_OPS}, 0)
GICV3_SOURCES	+=	drivers/arm/gic/v3/arm_gicv3_common.c
endif

GICV3_SOURCES	+=	drivers/arm/gic/v3/gic-x00.c
ifeq (${GICV3_IMPL_GIC600_MULTICHIP}, 1)
GICV3_SOURCES	+=	drivers/arm/gic/v3/gic600_multichip.c
endif

# Set GIC-600 support
$(eval $(call assert_boolean,GICV3_SUPPORT_GIC600))
$(eval $(call add_define,GICV3_SUPPORT_GIC600))

# Set GICv4 extension
$(eval $(call assert_boolean,GIC_ENABLE_V4_EXTN))
$(eval $(call add_define,GIC_ENABLE_V4_EXTN))

# Set support for extended PPI and SPI range
$(eval $(call assert_boolean,GIC_EXT_INTID))
$(eval $(call add_define,GIC_EXT_INTID))
