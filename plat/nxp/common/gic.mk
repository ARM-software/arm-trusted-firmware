# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#------------------------------------------------------------------------------
#
# Select the GIC files
#
# -----------------------------------------------------------------------------

ifeq ($(GIC), GIC400)
GIC_SOURCES		+=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c		\
				${PLAT_COMMON_PATH}/nxp_gicv2.c
else
    $(error -> GIC type not set!)
endif


# -----------------------------------------------------------------------------

